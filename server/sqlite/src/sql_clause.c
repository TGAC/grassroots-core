/*
 * sql_clause.c
 *
 *  Created on: 11 Dec 2017
 *      Author: billy
 */

#include "sql_clause.h"
#include "string_utils.h"
#include "memory_allocations.h"


SQLClause *AllocateSQLClause (const char *key_s, const char *op_s, const char *value_s)
{
	bool success_flag = true;
	char *copied_key_s = NULL;

	if (CloneValidString (key_s, &copied_key_s))
		{
			char *copied_op_s = NULL;

			if (CloneValidString (op_s, &copied_op_s))
				{
					char *copied_value_s = NULL;

					if (CloneValidString (value_s, &copied_value_s))
						{
							SQLClause *clause_p = (SQLClause *) AllocMemory (sizeof (SQLClause));

							if (clause_p)
								{
									clause_p -> sqlc_key_s = copied_key_s;
									clause_p -> sqlc_op_s = copied_op_s;
									clause_p -> sqlc_value_s = copied_value_s;

									return clause_p;
								}		/* if (clause_p) */

							if (copied_value_s)
								{
									FreeCopiedString (copied_value_s);
								}

						}		/* if (CloneValidString (value_s, &copied_value_s)) */

					if (copied_op_s)
						{
							FreeCopiedString (copied_op_s);
						}

				}		/* if (CloneValidString (op_s, &copied_op_s)) */


			if (copied_key_s)
				{
					FreeCopiedString (copied_key_s);
				}

		}		/* if (CloneValidString (key_s, &copied_key_s)) */

	return NULL;
}


void FreeSQLClause (SQLClause *clause_p)
{
	if (clause_p -> sqlc_key_s)
		{
			FreeCopiedString (clause_p -> sqlc_key_s);
		}

	if (clause_p -> sqlc_op_s)
		{
			FreeCopiedString (clause_p -> sqlc_op_s);
		}

	if (clause_p -> sqlc_value_s)
		{
			FreeCopiedString (clause_p -> sqlc_value_s);
		}

	FreeMemory (clause_p);
}


SQLClauseNode *AllocateSQLClauseNode (const char *key_s, const char *op_s, const char *value_s)
{
	SQLClauseNode *node_p = (SQLClauseNode *) AllocMemory (sizeof (SQLClauseNode));

	if (node_p)
		{
			SQLClause *clause_p = AllocateSQLClause (key_s, op_s, value_s);

			if (clause_p)
				{
					node_p -> sqlcn_clause_p = clause_p;

					node_p -> sqlcn_node.ln_prev_p = NULL;
					node_p -> sqlcn_node.ln_next_p = NULL;

					return node_p;
				}

			FreeMemory (node_p);
		}

	return NULL;
}


void FreeSQLClauseNode (ListItem *node_p)
{
	SQLClauseNode *sql_node_p = (SQLClauseNode *) node_p;

	if (sql_node_p -> sqlcn_clause_p)
		{
			FreeSQLClause (sql_node_p -> sqlcn_clause_p);
		}

	FreeMemory (sql_node_p);
}


bool AddSQLClauseToByteBuffer (const SQLClause *clause_p, ByteBuffer *buffer_p)
{
	return AppendStringsToByteBuffer (buffer_p, clause_p -> sqlc_key_s, clause_p -> sqlc_op_s, "\"", clause_p -> sqlc_value_s, "\"", NULL);
}



bool AddSQLClausesToByteBuffer (LinkedList *clauses_p, ByteBuffer *buffer_p)
{
	bool success_flag = false;

	if (clauses_p -> ll_size > 0)
		{
			if (AppendStringToByteBuffer (buffer_p, " WHERE "))
				{
					SQLClauseNode *node_p = (SQLClauseNode *) (clauses_p -> ll_head_p);

					if (AddSQLClauseToByteBuffer (node_p -> sqlcn_clause_p, buffer_p))
						{
							node_p = (SQLClauseNode *) (node_p -> sqlcn_node.ln_next_p);
							success_flag = true;

							while (node_p && success_flag)
								{
									if (AppendStringToByteBuffer (buffer_p, ", "))
										{
											if (AddSQLClauseToByteBuffer (node_p -> sqlcn_clause_p, buffer_p))
												{
													node_p = (SQLClauseNode *) (node_p -> sqlcn_node.ln_next_p);
												}
											else
												{
													success_flag = false;
												}

										}		/* if (AppendStringToByteBuffer (buffer_p, ", ")) */
									else
										{
											success_flag = false;
										}

								}		/* while (node_p && success_flag) */

							if (success_flag)
								{
									success_flag = AppendStringToByteBuffer (buffer_p, ";");
								}

						}		/* if (AddSQLClauseToByteBuffer (node_p -> sqlcn_clause_p, buffer_p)) */

				}		/* if (AppendStringToByteBuffer (buffer_p, " WHERE ")) */

		}		/* if (clauses_p -> ll_size > 0) */

	return success_flag;
}

