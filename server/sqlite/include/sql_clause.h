/*
 * sql_clause.h
 *
 *  Created on: 11 Dec 2017
 *      Author: billy
 */

#ifndef CORE_SERVER_SQLITE_INCLUDE_SQL_CLAUSE_H_
#define CORE_SERVER_SQLITE_INCLUDE_SQL_CLAUSE_H_

#include "sqlite_library.h"

#include "linked_list.h"
#include "byte_buffer.h"


typedef struct SQLClause
{
	char *sqlc_key_s;
	char *sqlc_op_s;
	char *sqlc_value_s;
} SQLClause;


typedef struct SQLClauseNode
{
	ListItem sqlcn_node;

	SQLClause *sqlcn_clause_p;

} SQLClauseNode;



GRASSROOTS_SQLITE_API SQLClause *AllocateSQLClause (const char *key_s, const char *op_s, const char *value_s);


GRASSROOTS_SQLITE_API void FreeSQLClause (SQLClause *clause_p);


GRASSROOTS_SQLITE_API SQLClauseNode *AllocateSQLClauseNode (const char *key_s, const char *op_s, const char *value_s);


GRASSROOTS_SQLITE_API void FreeSQLClauseNode (ListItem *node_p);


GRASSROOTS_SQLITE_API bool AddSQLClauseToByteBuffer (const SQLClause *clause_p, ByteBuffer *buffer_p);


GRASSROOTS_SQLITE_API bool AddSQLClausesToByteBuffer (LinkedList *clauses_p, ByteBuffer *buffer_p);


#endif /* CORE_SERVER_SQLITE_INCLUDE_SQL_CLAUSE_H_ */
