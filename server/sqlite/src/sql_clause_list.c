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


SQLClauseList *AllocateSQLClauseList (const char *op_s)
{
	char *copied_op_s = EasyCopyToNewString (op_s);

	if (copied_op_s)
		{
			SQLClauseList *clause_list_p = (SQLClauseList *) AllocMemory (sizeof (SQLClauseList));

			if (clause_list_p)
				{
					InitLinkedList (& (clause_list_p -> sqlcl_list));

					clause_list_p -> sqlcl_operator_s = copied_op_s;

					return clause_list_p;
				}		/* if (clause_list_p) */

			FreeCopiedString (copied_op_s);
		}		/* if (copied_op_s) */

	return NULL;
}


void FreeSQLClauseList (SQLClauseList *clause_list_p)
{
	FreeLinkedList (& (clause_list_p -> sqlcl_list));
	FreeCopiedString (clause_list_p -> sqlcl_operator_s);

	FreeMemory (clause_list_p);
}


SQLClauseListNode *AllocateSQLClauseListNode (SQLClauseList *clause_list_p)
{
	SQLClauseListNode *node_p = (SQLClauseListNode *) AllocMemory (sizeof (SQLClauseListNode));

	if (node_p)
		{
			InitListItem (& (node_p -> sqlcln_node));

			node_p -> sqlcln_list_p = clause_list_p;
		}

	return node_p;
}

SQLClauseListNode *AllocateSQLClauseListNodeByParts (const char *op_s)
{
	SQLClauseList *clause_list_p = AllocateSQLClauseList (op_s);

	if (clause_list_p)
		{
			SQLClauseListNode *node_p = AllocateSQLClauseListNode (clause_list_p);

			if (node_p)
				{
					return node_p;
				}

			FreeSQLClauseList (clause_list_p);
		}

	return NULL;
}


void FreeSQLClauseListNode (ListItem *node_p)
{
	SQLClauseListNode *clause_list_node_p = (SQLClauseListNode *) node_p;

	FreeSQLClauseList (clause_list_node_p -> sqlcln_list_p);
	FreeMemory (node_p);
}



bool AddSQLClauseToSQLClauseListByParts (SQLClauseList *list_p, const char *key_s, const char *op_s, const char *value_s)
{
	bool success_flag = false;
	SQLClauseNode *node_p = AllocateSQLClauseNode (key_s, op_s, value_s);

	if (node_p)
		{
			LinkedListAddTail (& (list_p -> sqlcl_list), & (node_p -> sqlcn_node));
			success_flag = true;
		}

	return success_flag;
}


char *GetAllSQLClausesAsString (SQLClauseList *list_p)
{
	char *sql_s = NULL;
	ByteBuffer *buffer_p = AllocateByteBuffer (1024);

	if (buffer_p)
		{
			SQLClauseNode *node_p = (SQLClauseNode *) (list_p -> sqlcl_list.ll_head_p);
			char *sub_sql_s = GetSQLClausesAsString (node_p);

		}		/* if (buffer_p) */

	return sql_s;
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
									success_flag = AppendStringsToByteBuffer (buffer_p, " ", list_p -> sqlcl_operator_s, " ", NULL);
								}

							if (success_flag)
								{
									success_flag = AddSQLClauseToByteBuffer (node_p -> sqlcn_clause_p, buffer_p);
								}

							if (success_flag)
								{
									node_p = (SQLClauseNode *) node_p -> sqlcn_node.ln_next_p;
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
