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
 * sql_clause_list.h
 *
 *  Created on: 23 Aug 2018
 *      Author: billy
 */

#ifndef CORE_SERVER_SQLITE_SRC_SQL_CLAUSE_LIST_H_
#define CORE_SERVER_SQLITE_SRC_SQL_CLAUSE_LIST_H_

#include "sqlite_library.h"
#include "linked_list.h"


typedef struct SQLClauseList
{
	/** The sublist of SQLClauseNodes for this SQLClauseList */
	LinkedList sqlcl_list;

} SQLClauseList;


#ifdef __cplusplus
extern "C"
{
#endif


/**
 * Allocate a SQLClauseList.
 *
 * @return the newly-allocated SQLClauseList or <code>NULL</code> upon error.
 * @memberof SQLClauseList
 */
GRASSROOTS_SQLITE_API SQLClauseList *AllocateSQLClauseList (void);


/**
 * Free a SQLClauseList.
 *
 * @param clause_list_p The SQLClauseList to free.
 * @memberof SQLClauseList
 */
GRASSROOTS_SQLITE_API void FreeSQLClauseList (SQLClauseList *clause_list_p);


/**
 * Add a SQLClauseNode to a SQLClauseList.
 *
 * @param op_s The operator the new SQLClause
 * @return the newly-allocated SQLClauseList or <code>NULL</code> upon error.
 * @memberof SQLClauseList
 */
GRASSROOTS_SQLITE_API bool AddSQLClauseToSQLClauseListByParts (SQLClauseList *list_p, const char *key_s, const char *comp_s, const char *value_s, const char *op_s);



/**
 * Free a SQLClauseListNode.
 * This will also call FreeSQLClauseList() for the contained SQLClauseList.
 *
 * @param node_p The SQLClauseListNode to free.
 * @memberof SQLClauseListNode
 */
GRASSROOTS_SQLITE_API void FreeSQLClauseListNode (ListItem *node_p);


/**
 * Get the string for all SQLClauses on a SQLClauseList.
 *
 * @param list_p The SQLClauseList of SQLClauses.
 * @return the newly-allocated string containing the SQLClauses as a statement or <code>NULL</code> upon error.
 * @memberof SQLClauseList
 */
GRASSROOTS_SQLITE_API char *GetAllSQLClausesAsString (SQLClauseList *list_p);


#ifdef __cplusplus
}
#endif


#endif /* CORE_SERVER_SQLITE_SRC_SQL_CLAUSE_LIST_H_ */
