/*
** Copyright 2014-2016 The Earlham Institute
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

/**
 * @file
 * @brief
 */
/*
 * sqlite_tool.h
 *
 *  Created on: 26 Jun 2015
 *      Author: billy
 */

#ifndef SQLITE_TOOL_H_
#define SQLITE_TOOL_H_

#include "typedefs.h"
#include "jansson.h"
#include "sqlite_library.h"
#include "sqlite3.h"
#include "linked_list.h"


#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifdef ALLOCATE_SQLITE_TAGS
	#define SQLITE_PREFIX GRASSROOTS_SQLITE_API
	#define SQLITE_VAL(x)	= x
#else
	#define SQLITE_PREFIX extern
	#define SQLITE_VAL(x)
#endif

#endif 		/* #ifndef DOXYGEN_SHOULD_SKIP_THIS */



/** The identifier used to uniquely specify a MongoDB document.
 *
 * @memberof MongoTool
 */
SQLITE_PREFIX const char *SQLITE_OP_EQUALS_S SQLITE_VAL("=");




/**
 * A SQLiteTool is a datatype that allows access to the data stored within
 * a SQLite instance.
 *
 * @ingroup sqlite_group
 */
typedef struct SQLiteTool
{
	/**
	 * @private
	 *
	 * This is the current sqlite3 client.
	 */
	sqlite3 *sqlt_database_p;

	/**
	 * The name of the table that this SQLiteTool is currently accessing.
	 */
	const char *sqlt_table_s;
} SQLiteTool;



#ifdef __cplusplus
extern "C"
{
#endif


/**
 * Set the database an collection that a SQLite will use.
 *
 * @param tool_p The SQLite to update.
 * @param db_s The database to use.
 * @param table_s The table to use.
 * @param flags The flags to use when opening the underlying sqlite database
 * see sqlite3_open_v2()
 * @return <code>true</code> if the SQLite was updated successfully.
 * <code>false</code> otherwise.
 * @memberof SQLiteTool
 */
GRASSROOTS_SQLITE_API bool SetSQLiteDatabase (SQLiteTool *tool_p, const char *db_s, int flags, const char *table_s);


/**
 * This allocates a SQLiteTool that connects to the SQLite instance specified in the
 * grassroots.config file
 *
 * @param db_s The database to use.
 * @param table_s The table to use.
 * @param flags The flags to use when opening the underlying sqlite database
 * see sqlite3_open_v2()
 * @return A SQLiteTool or <code>NULL</code> upon error.
 * @memberof SQLiteTool
 * @see InitSQLite
 */
GRASSROOTS_SQLITE_API SQLiteTool *AllocateSQLiteTool (const char *db_s, int flags, const char *table_s);


/**
 * Close the database connection for a given a SQLiteTool.
 *
 * @param tool_p The SQLiteTool to close.
 * @return <code>true</code> if the tool was closed successfully,
 * <code>false</code> otherwise.
 * @memberof SQLiteTool
 */
GRASSROOTS_SQLITE_API bool CloseSQLiteTool (SQLiteTool *tool_p);



/**
 * Delete a SQLiteTool and release the connection that it held
 * This calls CloseSqlTool()  and then releases the memory.
 *
 * @param tool_p The SQLiteTool to free.
 * @memberof SQLiteTool
 */
GRASSROOTS_SQLITE_API void FreeSQLiteTool (SQLiteTool *tool_p);


/**
 * Insert data from a given JSON fragment using a given SQLite.
 *
 * @param tool_p The SQLite to use.
 * @param values_p The JSON fragment to insert.
 * @param primary_key_id_s The key used to get the values from values_p that the matching
 * documents will be updated with.
 * @return A pointer to a newly-created BSON id or <code>NULL</code>
 * upon error. This value will need to be freed using FreeMemory() to
 * avoid a memory leak.
 * @memberof SQLiteTool
 */
GRASSROOTS_SQLITE_API const char *InsertOrUpdatSQLiteData (SQLiteTool *tool_p, json_t *values_p,  const char *const primary_key_id_s);


/**
 * Remove some documents from a SQLite collection.
 *
 * @param tool_p The SQLite that will amend the collection.
 * @param selector_json_p The statement used to choose which documents to remove.
 * @param remove_first_match_only_flag If this is <code>true</code> then only the first matching document
 * will be removed. If this is <code>false</code> then all matching documents will be removed.
 * @return <code>true</code> if any matching SQLite documents were removed successfully,
 * <code>false</code> otherwise.
 * @memberof SQLiteTool
 */
GRASSROOTS_SQLITE_API bool RemoveSQLiteRows (SQLiteTool *tool_p, const json_t *selector_json_p, const bool remove_first_match_only_flag);


/**
 * Find matching documents for a given query.
 *
 * @param tool_p The SQLite that will search the collection.
 * @param where_clauses_p The LinkedList of SqlClauseNodes specifying the query to run.
 * @param fields_ss If specified, then just the keys listed in this array along with their
 * associated values will be stored. This array must have <code>NULL</code> as its final
 * element. If this is <code>NULL</code>, then all of the keys and values in the matching
 * documents will be stored.
 * @param error_ss If an error occurs, this will point to a newly-allocated string describing the error.
 * This needs to be freed using FreeSQLiteToolErrorString().
 * @return <code>true</code> if any matching SQLite documents were found successfully,
 * <code>false</code> otherwise.
 * @memberof SQLiteTool
 */
GRASSROOTS_SQLITE_API json_t *FindMatchingSQLiteDocuments (SQLiteTool *tool_p, LinkedList *where_clauses_p, const char **fields_ss, char **error_ss);


/**
 * Get all results from a SQLite database.
 *
 * @param tool_p The SQLite to get the results with.
 * @return A json_t array with all of the results from the search or <code>NULL</code> upon error.
 * @memberof SQLiteTool
 */
GRASSROOTS_SQLITE_API json_t *GetAllSQLiteResultsAsJSON (SQLiteTool *tool_p);


/**
 * Get all results for a given key-value query.
 *
 * @param tool_p The SQLite to get the results with.
 * @param docs_pp A pointer to a JSON array pointer where the results will be placed.
 * @param key_s The key of the query.
 * @param value_s The value of the query.
 * @param fields_ss If specified, then just the keys listed in this array along with their
 * associated values will be stored. This array must have <code>NULL</code> as its final
 * element. If this is <code>NULL</code>, then all of the keys and values in the matching
 * documents will be stored.
 * @return The number of results found or -1 on error.
 * @memberof SQLiteTool
 */
GRASSROOTS_SQLITE_API int32 GetAllSQLiteResultsForKeyValuePair (SQLiteTool *tool_p, json_t **docs_pp, const char * const key_s, const char * const value_s, const char **fields_ss);


/**
 * When iterating over a SQLite's results get the values and increment the current result that
 * the SQLite is pointing at.
 *
 * @param tool_p The SQLite to get the results from.
 * @param fields_ss If specified, then just the keys listed in this array along with their
 * associated values will be stored. If this is <code>NULL</code>, then all of the keys and values in the matching
 * documents will be stored.
 * @param num_fields The number of entries in fields_ss.
 * @return A newly-allocated json_object with the key-value pair results or <code>NULL</code> upon error.
 * @memberof SQLiteTool
 */
GRASSROOTS_SQLITE_API json_t *GetCurrentValuesAsJSON (SQLiteTool *tool_p, const char **fields_ss, const size_t num_fields);


/**
 * Convert all of a SQLiteTool results into JSON array.
 *
 * @param tool_p The SQLiteTool to get the results with.
 * @return The newly-allocated JSON array containing the results or <code>NULL</code>
 * upon error.
 * @memberof SQLiteTool
 */
GRASSROOTS_SQLITE_API json_t *GetAllExistingSQLiteResultsAsJSON (SQLiteTool *tool_p);


/**
 * Check whether a database_s contains any documents with a given key-value pair.
 *
 * @param tool_p The SQLite to check with.
 * @param database_s The database to check.
 * @param key_s The key to search for.
 * @param value_s The value to search for.
 * @return The number of matching documents in the given collection or -1 upon error.
 * @memberof SQLiteTool
 */
GRASSROOTS_SQLITE_API int32 IsKeyValuePairInDatabase (SQLiteTool *tool_p, const char *database_s, const char *key_s, const char *value_s);


/**
 * Insert of update data in a set of matching documents.
 *
 * @param tool_p The SQLite to update with.
 * @param values_p The JSON values to add to the documents.
 * @param database_s The database to check.
 * @param primary_key_id_s The key used to get the values from values_p that the matching
 * documents will be updated with.
 * @param mapped_id_s The key to use for the query to get the documents with. If this is
 * <code>NULL</code> then the primary_key_id_s will be used instead.
 * @param object_key_s The key used to insert values_p into the matching documents.
 * @return An error string or <code>NULL</code> if the updates were successful.
 * @memberof SQLiteTool
 */
GRASSROOTS_SQLITE_API const char *InsertOrUpdateSQLiteData (SQLiteTool *tool_p, json_t *values_p, const char * const database_s, const char * const primary_key_id_s, const char * const mapped_id_s, const char * const object_key_s);


/**
 * Insert of update data in a set of matching documents.
 *
 * This calls InsertOrUpdateSQLiteData() with <code>NULL</code> entries
 * for the database, collection, mapped_id_s and object_key_s values.
 *
 * @param tool_p The SQLite to update with.
 * @param values_p The JSON values to add to the documents.
 * @param primary_key_id_s The key used to get the values from values_p that the matching
 * documents will be updated with.
 * @return An error string or <code>NULL</code> if the updates were successful.
 * @memberof SQLiteTool
 */
GRASSROOTS_SQLITE_API const char *EasyInsertOrUpdateSQLiteData (SQLiteTool *tool_p, json_t *values_p, const char *const primary_key_id_s);


/**
 * Free an error string that was generated from a given SQLiteTool.
 *
 * @param tool_p The given SQLiteTool.
 * @param error_s The error string to free.
 * @memberof SQLiteTool
 */
GRASSROOTS_SQLITE_API void FreeSQLiteToolErrorString (SQLiteTool *tool_p, char *error_s);



#ifdef __cplusplus
}
#endif



#endif /* _TOOL_H_ */
