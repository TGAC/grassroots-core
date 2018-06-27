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

/**
 * A datatype for storing a clause used to query a database.
 * Examples such as "foo=bar" would become
 *
 * sqlc_key_s = "foo"
 * sqlc_op_s = "="
 * sqlc_value_s = "bar"
 *
 * and "band like spina%" would become
 *
 * sqlc_key_s = "band"
 * sqlc_op_s = "like"
 * sqlc_value_s = "spina%"
 */
typedef struct SQLClause
{
	/** The key to use. */
	char *sqlc_key_s;

	/** The operator such as =, like, <, etc. */
	char *sqlc_op_s;

	/** The value to use in the query. */
	char *sqlc_value_s;
} SQLClause;


/**
 * A datatype for storing a SQLClause on a LinkedList.
 */
typedef struct SQLClauseNode
{
	/** The base ListItem */
	ListItem sqlcn_node;

	/** The SQLClause */
	SQLClause *sqlcn_clause_p;

} SQLClauseNode;


/**
 * Allocate a SQLClause.
 *
 * @param key_s The key for the underlying SQLClause.
 * @param op_s The key for the underlying SQLClause.
 * @param value_s The key for the underlying SQLClause.
 * @return the newly-allocated SQLClause or <code>NULL</code> upon error.
 * @memberof SQLClause
 */
GRASSROOTS_SQLITE_API SQLClause *AllocateSQLClause (const char *key_s, const char *op_s, const char *value_s);


/**
 * Free a SQLClause.
 *
 * @param clause_p The SQLClause to free.
 * @memberof SQLClause
 */
GRASSROOTS_SQLITE_API void FreeSQLClause (SQLClause *clause_p);


/**
 * Allocate a SQLClauseNode containing a fully initialised SQLClause.
 *
 * @param key_s The key for the underlying SQLClause.
 * @param op_s The key for the underlying SQLClause.
 * @param value_s The key for the underlying SQLClause.
 * @return the newly-allocated SQLClauseNode or <code>NULL</code> upon error.
 * @memberof SQLClauseNode
 */
GRASSROOTS_SQLITE_API SQLClauseNode *AllocateSQLClauseNode (const char *key_s, const char *op_s, const char *value_s);


/**
 * Free a SQLClauseNode.
 * This will also call FreeSQLClause() for the contained SQLClause.
 *
 * @param node_p The SQLClauseNode to free.
 * @memberof SQLClauseNode
 */
GRASSROOTS_SQLITE_API void FreeSQLClauseNode (ListItem *node_p);


/**
 * Add the string representing a SQLClause to a ByteBuffer to
 * be used as a query.
 *
 * @param clause_p The SQLClause.
 * @param buffer_p The ByteBuffer to print the SQLClause to.
 * @return <code>true</code> if the SQLClause was printed successfully to the ByteBuffer,
 * <code>false</code> otherwise
 * @memberof SQLClause
 */
GRASSROOTS_SQLITE_API bool AddSQLClauseToByteBuffer (const SQLClause *clause_p, ByteBuffer *buffer_p);


/**
 * Add the strings representing each of the SQLClauses stored on a LinkedList
 * of SQLClauseNodes to a ByteBuffer to be used as a query.
 *
 * @param clauses_p The LinkedList of SQLClauseNodes.
 * @param buffer_p The ByteBuffer to print the SQLClause to.
 * @return <code>true</code> if the all SQLClauses on the list was printed successfully to the ByteBuffer,
 * <code>false</code> otherwise
 * @memberof SQLClause
 */
GRASSROOTS_SQLITE_API bool AddSQLClausesToByteBuffer (LinkedList *clauses_p, ByteBuffer *buffer_p);


#endif /* CORE_SERVER_SQLITE_INCLUDE_SQL_CLAUSE_H_ */
