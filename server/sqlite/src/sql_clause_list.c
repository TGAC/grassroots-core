/*
** Copyright 2014-2018 The Earlham Institute
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/
/*
 * sql_clause_list.c
 *
 *  Created on: 23 Aug 2018
 *      Author: billy
 */

#include "sql_clause_list.h"
#include "memory_allocations.h"
#include "byte_buffer.h"
#include "string_utils.h"
#include "sql_clause.h"
#include "streams.h"



SQLClauseList *AllocateSQLClauseList (void)
{
	SQLClauseList *clause_list_p = (SQLClauseList *) AllocMemory (sizeof (SQLClauseList));

	if (clause_list_p)
		{
			InitLinkedList (& (clause_list_p -> sqlcl_list));
		}		/* if (clause_list_p) */

	return clause_list_p;
}


void FreeSQLClauseList (SQLClauseList *clause_list_p)
{
	FreeLinkedList (& (clause_list_p -> sqlcl_list));

	FreeMemory (clause_list_p);
}



bool AddSQLClauseToSQLClauseListByParts (SQLClauseList *list_p, const char *key_s, const char *comp_s, const char *value_s, const char *op_s)
{
	bool success_flag = false;
	SQLClauseNode *node_p = AllocateSQLClauseNode (key_s, op_s, value_s, comp_s);

	if (node_p)
		{
			LinkedListAddTail (& (list_p -> sqlcl_list), & (node_p -> sqlcn_node));
			success_flag = true;
		}

	return success_flag;
}



char *GetSQLClausesAsString (SQLClauseList *list_p)
{
	char *sql_s = NULL;
	ByteBuffer *buffer_p = AllocateByteBuffer (1024);

	if (buffer_p)
		{
			if (AppendStringToByteBuffer (buffer_p, "WHERE "))
				{
					bool success_flag = true;
					SQLClauseNode * const first_node_p = (SQLClauseNode * const) (list_p -> sqlcl_list.ll_head_p);
					SQLClauseNode *node_p = first_node_p;

					while (node_p && success_flag)
						{
							if (node_p != first_node_p)
								{
									if (node_p -> sqlcn_op_s)
										{
											success_flag = AppendStringsToByteBuffer (buffer_p, " ", node_p -> sqlcn_op_s, " ", NULL);
										}
									else
										{
											SQLClause *clause_p = node_p -> sqlcn_clause_p;

											success_flag = false;
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "SQLClause %s %s %s has no clause operator and is not the first item in the list", clause_p -> sqlc_key_s, clause_p -> sqlc_op_s, clause_p -> sqlc_value_s);
										}
								}

							if (success_flag)
								{
									if (AddSQLClauseToByteBuffer (node_p -> sqlcn_clause_p, buffer_p))
										{
											node_p = (SQLClauseNode *) node_p -> sqlcn_node.ln_next_p;
										}
									else
										{
											success_flag = false;
										}
								}

						}		/* while (node_p && success_flag) */

					if (success_flag)
						{
							sql_s = DetachByteBufferData (buffer_p);
							buffer_p = NULL;
						}

				}		/* if (AppendStringToByteBuffer (buffer_p, "WHERE ")) */

			if (buffer_p)
				{
					FreeByteBuffer (buffer_p);
				}

		}		/* if (buffer_p) */

	return sql_s;
}
