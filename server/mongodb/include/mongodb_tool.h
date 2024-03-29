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
 * mongodb_tool.h
 *
 *  Created on: 26 Jun 2015
 *      Author: billy
 */
#ifndef MONGODB_TOOL_H_
#define MONGODB_TOOL_H_

#include "mongoc.h"
#include "typedefs.h"
#include "jansson.h"
#include "mongodb_library.h"
#include "operation.h"


#ifndef DOXYGEN_SHOULD_SKIP_THIS

	#ifdef GRASSROOTS_MONGODB_LIBRARY_EXPORTS // defined if we are building the LIB DLL (instead of using it)

		#ifdef ALLOCATE_MONGODB_TAGS
			#define MONGODB_PREFIX GRASSROOTS_MONGODB_API
			#define MONGODB_VAL(x)	= x
		#else
			#define MONGODB_PREFIX  extern GRASSROOTS_MONGODB_API
			#define MONGODB_VAL(x)
		#endif
	#else
		#define MONGODB_PREFIX extern GRASSROOTS_MONGODB_API
		#define MONGODB_VAL(x)
	#endif

#endif //	#ifndef DOXYGEN_SHOULD_SKIP_THIS





/** The identifier used to uniquely specify a MongoDB document.
 *
 * @memberof MongoTool
 */
MONGODB_PREFIX const char *MONGO_ID_S MONGODB_VAL("_id");

/** The identifier used to specify a collection name to use.
 *
 * @memberof MongoTool
 */
MONGODB_PREFIX const char *MONGO_COLLECTION_S MONGODB_VAL("collection");

/**
 * The identifier used to specify the operation to perform.
 *
 * @memberof MongoTool
 */
MONGODB_PREFIX const char *MONGO_OPERATION_S MONGODB_VAL("operation");

/**
 * The identifier used to specify an add data to a collection or document.
 *
 * @memberof MongoTool
 */
MONGODB_PREFIX const char *MONGO_OPERATION_INSERT_S MONGODB_VAL("insert");

/**
 * The identifier used to specify searching for documents.
 *
 * @memberof MongoTool
 */
MONGODB_PREFIX const char *MONGO_OPERATION_SEARCH_S MONGODB_VAL("search");

/**
 * The identifier used to specify an remove data from a collection or document.
 *
 * @memberof MongoTool
 */
MONGODB_PREFIX const char *MONGO_OPERATION_REMOVE_S MONGODB_VAL("remove");

/**
 * The identifier used to specify a JSON fragment denoting
 * operations.
 *
 * @memberof MongoTool
 */
MONGODB_PREFIX const char *MONGO_OPERATION_DATA_S MONGODB_VAL("data");

/**
 * The identifier used to specify which fields to get the values of
 * for the results of a query.
 *
 * @memberof MongoTool
 */
MONGODB_PREFIX const char *MONGO_OPERATION_FIELDS_S MONGODB_VAL("fields");

/**
 * The identifier used to generate a dump of an entire collection.
 *
 * @memberof MongoTool
 */
MONGODB_PREFIX const char *MONGO_OPERATION_GET_ALL_S MONGODB_VAL("dump");

/**
 * The identifier used to specify a query operator.
 *
 * @memberof MongoTool
 */
MONGODB_PREFIX const char *MONGO_CLAUSE_OPERATOR_S MONGODB_VAL("operator");

/**
 * The identifier used to specify a query value.
 *
 * @memberof MongoTool
 */
MONGODB_PREFIX const char *MONGO_CLAUSE_VALUE_S MONGODB_VAL("value");


//MONGODB_PREFIX const uint32 MONGO_OID_STRING_BUFFER_SIZE MONGODB_VAL(25);

#define MONGO_OID_STRING_BUFFER_SIZE (25)

MONGODB_PREFIX const char *MONGO_OID_KEY_S MONGODB_VAL("$oid");



/**
 * The key for specifying the timestamp of a saved piece of data
 *
 * @ingroup MongoTool
 */
MONGODB_PREFIX const char *MONGO_TIMESTAMP_S MONGODB_VAL ("modified");


/**
 * A MongoTool is a datatype that allows access to the data stored within
 * a MongoDB instance.
 *
 * @ingroup mongodb_group
 */
typedef struct MongoTool
{
	/**
	 * @private
	 *
	 * This is the current mongo client.
	 */
	mongoc_client_t *mt_client_p;


	bool mt_owns_client_flag;

	/**
	 * @private
	 *
	 * This is the current mongo collection.
	 */
	mongoc_collection_t *mt_collection_p;


	/**
	 * @private
	 *
	 * This is the current mongo database.
	 */
	mongoc_database_t *mt_database_p;

	/**
	 * @private
	 *
	 * This is the current mongo cursor.
	 */
	mongoc_cursor_t *mt_cursor_p;


	struct MongoClientManager *mt_manager_p;
} MongoTool;


/*
 * The following preprocessor macros allow us to declare
 * and define the variables in the same place. By default,
 * they will expand to
 *
 * 		extern const char *SERVICE_NAME_S;
 *
 * however if ALLOCATE_MONGODB_TAGS is defined then it will
 * become
 *
 * 		const char *SERVICE_NAME_S = "path";
 *
 * ALLOCATE_MONGODB_TAGS must be defined only once prior to
 * including this header file. Currently this happens in
 * mongodb_tool.c.
 */

#ifdef __cplusplus
extern "C"
{
#endif


/**
 * Set the database and collection that a MongoTool will use.
 *
 * @param tool_p The MongoTool to update.
 * @param db_s The database to use.
 * @param collection_s The collection to use.
 * @return <code>true</code> if the MongoTool was updated successfully,
 * <code>false</code> otherwise.
 * @memberof MongoTool
 */
GRASSROOTS_MONGODB_API bool SetMongoToolDatabaseAndCollection (MongoTool *tool_p, const char *db_s, const char *collection_s);


/**
 * Set the database that a MongoTool will use.
 *
 * @param tool_p The MongoTool to update.
 * @param db_s The database to use.
 * @return <code>true</code> if the MongoTool was updated successfully,
 * <code>false</code> otherwise.
 * @memberof MongoTool
 */
GRASSROOTS_MONGODB_API bool SetMongoToolDatabase (MongoTool *tool_p, const char *db_s);


/**
 * Set the collection that a MongoTool will use.
 *
 * @param tool_p The MongoTool to update.
 * @param collection_s The collection to use.
 * @return <code>true</code> if the MongoTool was updated successfully,
 * <code>false</code> otherwise.
 * @memberof MongoTool
 */
GRASSROOTS_MONGODB_API bool SetMongoToolCollection (MongoTool *tool_p, const char *collection_s);


/**
 * This allocates a MongoTool that connects to the MongoDB instance specified in the
 * grassroots.config file
 *
 * @return A MongoTool or <code>NULL</code> upon error.
 * @memberof MongoTool
 * @see InitMongoDB
 */
GRASSROOTS_MONGODB_API MongoTool *AllocateMongoTool (mongoc_client_t *client_p, struct MongoClientManager *mongo_manager_p);


/**
 * Delete a MongoTool and release the connection that it held
 *
 * @param tool_p The MongoTool to free.
 * @memberof MongoTool
 */
GRASSROOTS_MONGODB_API void FreeMongoTool (MongoTool *tool_p);


/**
 * Insert data from a given JSON fragment using a given MongoTool.
 *
 * @param tool_p The MongoTool to use.
 * @param json_p The JSON fragment to insert.
 * @return A pointer to a newly-created BSON id or <code>NULL</code>
 * upon error. This value will need to be freed using FreeMemory() to
 * avoid a memory leak.
 * @memberof MongoTool
 */
GRASSROOTS_MONGODB_API bson_oid_t *InsertJSONIntoMongoCollection (MongoTool *tool_p, json_t *json_p);


/**
 * Update some MongoDB documents.
 *
 * @param tool_p The MongoTool that will update the MongoDB documents.
 * @param query_p The query used to choose the MongoDB documents that will be updated.
 * @param update_p The update statement specifying the update operation to perform.
 * @return <code>true</code> if the MongoDB documents were updated successfully,
 * <code>false</code> otherwise.
 * @memberof MongoTool
 */
GRASSROOTS_MONGODB_API bool UpdateMongoDocumentsByJSON (MongoTool *tool_p, const json_t *query_p, const json_t *update_p, const bool multiple_flag);


/**
 * Create a BSON object from the given id and use it to update some MongoDB documents.
 *
 * @param tool_p The MongoTool that will update the MongoDB documents.
 * @param id_p The id to update the MongoDB documents with.
 * @param json_p The update statement specifying the update operation to perform.
 * @return <code>true</code> if the MongoDB documents were updated successfully,
 * <code>false</code> otherwise.
 * @memberof MongoTool
 * @see UpdateMongoDocumentByBSON
 */
GRASSROOTS_MONGODB_API bool UpdateMongoDocuments (MongoTool *tool_p, const bson_oid_t *id_p, const json_t *json_p, const bool multiple_flag);


/**
 * Remove some documents from a MongoDB collection.
 *
 * @param tool_p The MongoTool that will amend the collection.
 * @param selector_json_p The statement used to choose which documents to remove.
 * @param remove_first_match_only_flag If this is <code>true</code> then only the first matching document
 * will be removed. If this is <code>false</code> then all matching documents will be removed.
 * @return <code>true</code> if any matching MongoDB documents were removed successfully,
 * <code>false</code> otherwise.
 * @memberof MongoTool
 */
GRASSROOTS_MONGODB_API bool RemoveMongoDocuments (MongoTool *tool_p, const json_t *selector_json_p, const bool remove_first_match_only_flag);


/**
 * Find matching documents for a given query.
 *
 * @param tool_p The MongoTool that will search the collection.
 * @param query_json_p The statement used to choose which documents to store.
 * @param fields_ss If specified, then just the keys listed in this array along with their
 * associated values will be stored. This array must have <code>NULL</code> as its final
 * element. If this is <code>NULL</code>, then all of the keys and values in the matching
 * documents will be stored.
 * @return <code>true</code> if any matching MongoDB documents were found successfully,
 * <code>false</code> otherwise.
 * @memberof MongoTool
 */
GRASSROOTS_MONGODB_API bool FindMatchingMongoDocumentsByJSON (MongoTool *tool_p, const json_t *query_json_p, const char **fields_ss, bson_t *extra_opts_p);


/**
 * Find matching documents for a given query.
 *
 * @param tool_p The MongoTool that will search the collection.
 * @param query_p The statement used to choose which documents to store.
 * @param fields_ss If specified, then just the keys listed in this array along with their
 * associated values will be stored. This array must have <code>NULL</code> as its final
 * element. If this is <code>NULL</code>, then all of the keys and values in the matching
 * documents will be stored.
 * @return <code>true</code> if any matching MongoDB documents were found successfully,
 * <code>false</code> otherwise.
 * @memberof MongoTool
 */
GRASSROOTS_MONGODB_API bool FindMatchingMongoDocumentsByBSON (MongoTool *tool_p, const bson_t *query_p, const char **fields_ss, bson_t *extra_opts_p);


/**
 * Iterate over all of a MongoTool's results.
 *
 * @param tool_p The MongoTool to iterate over.
 * @param process_bson_fn The function that will be called for each bson document in
 * the results.
 * @param data_p Any required custom data that will be passed to the callback function
 * for each bson document.
 * @return <code>true</code> if the MongoDB documents were iterated over successfully,
 * <code>false</code> otherwise.
 * @memberof MongoTool
 */
GRASSROOTS_MONGODB_API bool IterateOverMongoResults (MongoTool *tool_p, bool (*process_bson_fn) (const bson_t *document_p, void *data_p), void *data_p);


/**
 * Check whether a MongoTool has any results after running a query.
 *
 * @param tool_p The MongoTool to check.
 * @return <code>true</code> if the MongoTool has results,
 * <code>false</code> otherwise.
 * @memberof MongoTool

 */
GRASSROOTS_MONGODB_API bool HasMongoQueryResults (MongoTool *tool_p);


/**
 * Get all results from a mongodb collection.
 *
 * @param tool_p The MongoTool to get the results with.
 * @param query_p The query to run.
 * @return A json_t array with all of the results from the search or <code>NULL</code> upon error.
 * @memberof MongoTool
 */
GRASSROOTS_MONGODB_API json_t *GetAllMongoResultsAsJSON (MongoTool *tool_p, bson_t *query_p, bson_t *extra_opts_p);


/**
 * Fill an existing json array with all the results from searching a mongodb collection.
 *
 * @param tool_p The MongoTool to get the results with.
 * @param query_p The query to run.
 * @param results_array_p A json_t array with all of the results from the search.
 * @param <code>true</code> if the search and retrieval of the results was successful, <code>false</code> otherwise.
 * @memberof MongoTool
 */
GRASSROOTS_MONGODB_API bool PopulateJSONWithAllMongoResults (MongoTool *tool_p, bson_t *query_p, bson_t *extra_opts_p, json_t *results_array_p);


/**
 * Get all results for a given key-value query.
 *
 * @param tool_p The MongoTool to get the results with.
 * @param docs_pp A pointer to a JSON array pointer where the results will be placed.
 * @param key_s The key of the query.
 * @param value_s The value of the query.
 * @param fields_ss If specified, then just the keys listed in this array along with their
 * associated values will be stored. This array must have <code>NULL</code> as its final
 * element. If this is <code>NULL</code>, then all of the keys and values in the matching
 * documents will be stored.
 * @return The number of results found or -1 on error.
 * @memberof MongoTool
 */
GRASSROOTS_MONGODB_API int32 GetAllMongoResultsForKeyValuePair (MongoTool *tool_p, json_t **docs_pp, const char * const key_s, const char * const value_s, const char **fields_ss);


/**
 * When iterating over a MongoTool's results get the values and increment the current result that
 * the MongoTool is pointing at.
 *
 * @param tool_p The MongoTool to get the results from.
 * @param fields_ss If specified, then just the keys listed in this array along with their
 * associated values will be stored. If this is <code>NULL</code>, then all of the keys and values in the matching
 * documents will be stored.
 * @param num_fields The number of entries in fields_ss.
 * @return A newly-allocated json_object with the key-value pair results or <code>NULL</code> upon error.
 * @memberof MongoTool
 */
GRASSROOTS_MONGODB_API json_t *GetCurrentValuesAsJSON (MongoTool *tool_p, const char **fields_ss, const size_t num_fields);


/**
 * Convert all of a MongoTool's results into JSON array.
 *
 * @param tool_p The MongoTool to get the results with.
 * @return The newly-allocated JSON array containing the results or <code>NULL</code>
 * upon error.
 * @memberof MongoTool
 */
GRASSROOTS_MONGODB_API json_t *GetAllExistingMongoResultsAsJSON (MongoTool *tool_p);


/**
 * Convert a BSON document and add it to a JSON array.
 *
 * @param document_p The BSON document to extract the values from.
 * @param data_p The JSON array to add the values to
 * @return <code>true</code> if the values were added successfully,
 * <code>false</code> otherwise.
 */
GRASSROOTS_MONGODB_API bool AddBSONDocumentToJSONArray (const bson_t *document_p, void *data_p);


/**
 * Print the JSON representation of a BSON fragment to the log Stream.
 *
 * @param level The Stream level to specify when printing this.
 * @param filename_s The name of the file which will be specified with this message.
 * @param line_number The line number which will be specified with this message.
 * @param bson_p The BSON fragment to print.
 * @param prefix_s An optional string to print in the stream prior to the BSON representation.
 */
GRASSROOTS_MONGODB_API int PrintBSONToLog (const uint32 level, const char *filename_s, const int line_number, const bson_t *bson_p, const char *message_s, ...);


/**
 * Print the JSON representation of a BSON fragment to the errors Stream.
 *
 * @param level The Stream level to specify when printing this.
 * @param filename_s The name of the file which will be specified with this message.
 * @param line_number The line number which will be specified with this message.
 * @param bson_p The BSON fragment to print.
 * @param prefix_s An optional string to print in the stream prior to the BSON representation.
 */
GRASSROOTS_MONGODB_API int PrintBSONToErrors (const uint32 level, const char *filename_s, const int line_number, const bson_t *bson_p, const char *message_s, ...);


/**
 * Print a BSON id to the log Stream.
 *
 * @param bson_p The BSON fragment to print.
 * @param level The Stream level to specify when printing this.
 * @param filename_s The name of the file which will be specified with this message.
 * @param line_number The line number which will be specified with this message.
 * @param prefix_s An optional string to print in the stream prior to the BSON representation.
 */
GRASSROOTS_MONGODB_API void LogBSONOid (const bson_oid_t *bson_p, const int level, const char * const filename_s, const int line_number,  const char * const prefix_s);


/**
 * Print the JSON representation of a BSON object and all of its children
 * and members to the log Stream.
 *
 * @param bson_p The BSON fragment to print.
 * @param level The Stream level to specify when printing this.
 * @param file_s The name of the file which will be specified with this message.
 * @param line The line number which will be specified with this message.
 * @param prefix_s An optional string to print in the stream prior to the BSON representation.
 */
GRASSROOTS_MONGODB_API void LogAllBSON (const bson_t *bson_p, const int level, const char * const file_s, const int line, const char * const prefix_s);


/**
 * Check whether a collection contains any documents with a given key-value pair.
 *
 * @param tool_p The MongoTool to check with.
 * @param database_s The database to check.
 * @param collection_s The collection to check.
 * @param key_s The key to search for.
 * @param value_s The value to search for.
 * @return The number of matching documents in the given collection or -1 upon error.
 * @memberof MongoTool
 */
GRASSROOTS_MONGODB_API int32 IsKeyValuePairInCollection (MongoTool *tool_p, const char *database_s, const char *collection_s, const char *key_s, const char *value_s);


/**
 * Create a BSON query from a JSON fragment.
 *
 * @param json_p The JSON fragment to create the query from.
 * If the given fragment is not an object or array, treat the statement as key = value.
 * If it is an array, treat it as an "in" query.
 * If it is an object, the value can have the following fields:
 *
 * key:  one of "=", "<", "<=", ">", ">=", "in", "range", "not"
 * value: can be single value or array. For a "range" key, it will be an array
 * of 2 elements that are the inclusive lower and upper bounds.
 * @return The BSON query or <code>NULL</code> upon error.
 */
GRASSROOTS_MONGODB_API bson_t *GenerateQuery (const json_t *json_p);


/**
 * Convert a BSON object into its equivalent JSON type.
 *
 * @param value_p The value to convert.
 * @return The newly-created JSON object with the equivalent value
 * or <code>NULL</code> upon error.
 */
GRASSROOTS_MONGODB_API json_t *ConvertBSONValueToJSON (const bson_value_t *value_p);




/**
 * Create a new JSON fragment from a given BSON one.
 *
 * @param bson_p The BSON fragment to convert to JSON.
 * @return The JSON fragment or <code>NULL</code> upon error.
 */
GRASSROOTS_MONGODB_API json_t *ConvertBSONToJSON (const bson_t *bson_p);


/**
 * Insert of update data in a set of matching documents.
 *
 * @param tool_p The MongoTool to update with.
 * @param values_p The JSON values to add to the documents.
 * @param database_s The database to check.
 * @param collection_s The collection to check.
 * @param primary_key_id_s The key used to get the values from values_p that the matching
 * documents will be updated with.
 * @param mapped_id_s The key to use for the query to get the documents with. If this is
 * <code>NULL</code> then the primary_key_id_s will be used instead.
 * @param object_key_s The key used to insert values_p into the matching documents.
 * @return An error string or <code>NULL</code> if the updates were successful.
 * @memberof MongoTool
 */
GRASSROOTS_MONGODB_API const char *InsertOrUpdateMongoData (MongoTool *tool_p, json_t *values_p, const char * const database_s, const char * const collection_s, const char **primary_keys_ss, const size_t num_keys, const char * const mapped_id_s, const char * const object_key_s);


/**
 * Insert of update data in a set of matching documents.
 *
 * This calls InsertOrUpdateMongoData() with <code>NULL</code> entries
 * for the database, collection, mapped_id_s and object_key_s values.
 *
 * @param tool_p The MongoTool to update with.
 * @param values_p The JSON values to add to the documents.
 * @param primary_key_id_s The key used to get the values from values_p that the matching
 * documents will be updated with.
 * @return An error string or <code>NULL</code> if the updates were successful.
 * @memberof MongoTool
 */
GRASSROOTS_MONGODB_API const char *EasyInsertOrUpdateMongoData (MongoTool *tool_p, json_t *values_p, const char *const primary_key_id_s);



/**
 * Create an index for the current collection of a MongoTool.
 *
 * @param tool_p The MongoTool whose collection will have the new index.
 * @param fields_ss An array of field names to use to generate the unique or compound index.
 * The final element in this array must be <code>NULL</code>.
 * @return <code>true</code> if the values were added successfully,
 * <code>false</code> otherwise.
 * @memberof MongoTool
 */
GRASSROOTS_MONGODB_API bool CreateIndexForMongoCollection (MongoTool *tool_p, char **fields_ss);



GRASSROOTS_MONGODB_API bool InsertMongoData (MongoTool *tool_p, const json_t *values_p, bson_t **reply_pp, bson_error_t *error_p);


GRASSROOTS_MONGODB_API bool InsertMongoDataAsBSON (MongoTool *tool_p, const bson_t *doc_p, bson_t **reply_pp, bson_error_t *error_p);


GRASSROOTS_MONGODB_API bool RunMongoCommand (MongoTool *tool_p, bson_t *command_p, bson_t **reply_pp);


GRASSROOTS_MONGODB_API bool RemoveMongoDocumentsByBSON (MongoTool *tool_p, const bson_t *selector_p, const bool remove_first_match_only_flag);



/**
 * Create a new BSON fragment from a given JSON one.
 *
 * @param json_p The JSON fragment to convert to BSON.
 * @return The BSON fragment or <code>NULL</code> upon error.
 */
GRASSROOTS_MONGODB_API bson_t *ConvertJSONToBSON (const json_t *json_p);


GRASSROOTS_MONGODB_API char *GetBSONOidAsString (const bson_oid_t *id_p);


GRASSROOTS_MONGODB_API void FreeBSONOidString (char *id_s);


GRASSROOTS_MONGODB_API bson_oid_t *GetBSONOidFromString (const char *id_s);


GRASSROOTS_MONGODB_API bool SaveMongoData (MongoTool *mongo_p, const json_t *data_to_save_p, const char *collection_s, bson_t *selector_p);


GRASSROOTS_MONGODB_API bool SaveMongoDataWithTimestamp (MongoTool *mongo_p, json_t *data_to_save_p, const char *collection_s, bson_t *selector_p, const char *timestamp_key_s);

GRASSROOTS_MONGODB_API bool SaveAndBackupMongoDataWithTimestamp (MongoTool *mongo_p, json_t *data_to_save_p, const char *collection_s, const char *backup_collection_s, const char *id_key_s, bson_t *selector_p, const char *timestamp_key_s);


GRASSROOTS_MONGODB_API bool SaveMongoDataFromBSON (MongoTool *mongo_p, const bson_t *data_to_save_p, const char *collection_s, bson_t *selector_p);


GRASSROOTS_MONGODB_API bool SaveAndBackupMongoData (MongoTool *mongo_p, const json_t *data_to_save_p, const char *collection_s, const char *backup_collection_s, const char *id_key_s, bson_t *selector_p);


GRASSROOTS_MONGODB_API bool SaveAndBackupMongoDataFromBSON (MongoTool *mongo_p, const bson_t *data_to_save_p, const char *collection_s, const char *backup_collection_s, const char *id_key_s, bson_t *selector_p);


GRASSROOTS_MONGODB_API bool SetMongoDataAsBSON (MongoTool *tool_p, bson_t *selector_p, const bson_t *doc_p, bson_t **reply_pp);


GRASSROOTS_MONGODB_API bool UnsetMongoDataAsBSON (MongoTool *tool_p, bson_t *selector_p, const bson_t *doc_p, bson_t **reply_pp);


GRASSROOTS_MONGODB_API bool UpdateMongoDataAsBSON (MongoTool *tool_p, const char * const update_s, bson_t *selector_p, const bson_t *doc_p, bson_t **reply_pp);


GRASSROOTS_MONGODB_API bool SetMongoDataAsBSONForGivenId (MongoTool *tool_p, bson_oid_t *id_p, bson_t *update_p, bson_t **reply_pp);


GRASSROOTS_MONGODB_API bool SetMongoData (MongoTool *tool_p, bson_t *selector_p, const json_t *values_p, bson_t **reply_pp);


/**
 * Remove the specified fields from a document.
 *
 * @param tool_p The MongoTool to use to remove the fields with.
 * @param selector_p The query to find the document to alter.
 * @param fields_ss An array of strings specifying the fields to remove. This array must
 * have a <code>NULL</code> as its final entry.
 * @param reply_pp If you wish to any error messages back then a pointer to a bson_t pointer
 * can be placed here. This can be <code>NULL</code>
 * @return <code>true</code> if the fields were removed successfully, <code>false</code> otherwise.
 * @memberof MongoTool
 */
GRASSROOTS_MONGODB_API bool RemoveMongoFields (MongoTool *tool_p, bson_t *selector_p, const char **fields_ss, bson_t **reply_pp);


GRASSROOTS_MONGODB_API bson_oid_t *GetNewBSONOid (void);


GRASSROOTS_MONGODB_API bson_oid_t *GetNewUnitialisedBSONOid (void);


GRASSROOTS_MONGODB_API bson_oid_t *CopyBSONOid (const bson_oid_t *src_p);


GRASSROOTS_MONGODB_API void FreeBSONOid (bson_oid_t *id_p);


GRASSROOTS_MONGODB_API bson_t *AllocateBSON (void);


GRASSROOTS_MONGODB_API void FreeBSON (bson_t *bson_p);


GRASSROOTS_MONGODB_API bool GetMongoIdFromJSON (const json_t *data_p, bson_oid_t *id_p);


GRASSROOTS_MONGODB_API bool GetNamedIdFromJSON (const json_t *data_p, const char * const key_s, bson_oid_t *id_p);


GRASSROOTS_MONGODB_API const char *GetNamedIdAsStringFromJSON (const json_t *data_p, const char * const key_s);


GRASSROOTS_MONGODB_API bool GetIdFromJSONKeyValuePair (const json_t *id_val_p, bson_oid_t *id_p);


GRASSROOTS_MONGODB_API bool AddCompoundIdToJSON (json_t *data_p, bson_oid_t *id_p);


GRASSROOTS_MONGODB_API bool AddNamedCompoundIdToJSON (json_t *data_p, bson_oid_t *id_p, const char *key_s);


GRASSROOTS_MONGODB_API bool AddCompoundIdToJSONArray (json_t *array_p, const bson_oid_t *id_p);


GRASSROOTS_MONGODB_API bool AddIdToJSON (json_t *data_p, bson_oid_t *id_p, const char *key_s);


GRASSROOTS_MONGODB_API bool AddQueryTerm (bson_t *query_p, const char *key_s, const char *value_s, bool regex_flag);


GRASSROOTS_MONGODB_API int64 GetNumberOfMongoResults (MongoTool *tool_p, bson_t *query_p, bson_t *extra_opts_p);


GRASSROOTS_MONGODB_API OperationStatus ProcessMongoResults (MongoTool *tool_p, bson_t *query_p, bson_t *extra_opts_p, bool (*process_bson_fn) (const bson_t *document_p, void *data_p), void *data_p);



/**
 * Update some MongoDB documents.
 *
 * @param tool_p The MongoTool that will update the MongoDB documents.
 * @param query_p The query used to choose the MongoDB documents that will be updated.
 * @param update_p The update statement specifying the update operation to perform.
 * @return <code>true</code> if the MongoDB documents were updated successfully,
 * <code>false</code> otherwise.
 * @memberof MongoTool
 */
GRASSROOTS_MONGODB_API bool UpdateMongoDocumentsByBSON (MongoTool *tool_p, const bson_t *query_p, const json_t *update_p, const bool multiple_flag);



/**
 * Create an index for given set of keys.
 *
 * @param tool_p The MongoTool that will be used to create the index.
 * @param database_s The database to that the collection to add the index to is in.
 * @param collection_s The collection to add the index for.
 * @param key_ss An array of keys that will have the index. The final entry must be NULL.
 * @param unique_flag <code>true</code> if the values for the key are unique, <code>false</code> otherwise. The default value is <code>false</code>.
 * @param sparse_flag If <code>true</code>, the index only references documents with the specified field. These indexes use less space
 * but behave differently in some situations (particularly sorts). The default value is <code>false</code>.
 * @return <code>true</code> if the index was created successfully, <code>false</code> otherwise.
 * @memberof MongoTool
 */
GRASSROOTS_MONGODB_API bool AddCollectionCompoundIndex (MongoTool *tool_p, const char *database_s, const char * const collection_s, const char ** const keys_ss, const bool unique_flag, const bool sparse_flag);


/**
 * Create an index for given key.
 *
 * @param tool_p The MongoTool that will be used to create the index.
 * @param database_s The database to that the collection to add the index to is in.
 * @param collection_s The collection to add the index for.
 * @param key_s The key that will have the index.
 * @param index_type_s The type of data being indexed if you it is 2d data or something similar. If
 * this is <code>NULL</code> then this will be a default index.
 * @param unique_flag <code>true</code> if the values for the key are unique, <code>false</code> otherwise. The default value is <code>false</code>.
 * @param sparse_flag If <code>true</code>, the index only references documents with the specified field. These indexes use less space
 * but behave differently in some situations (particularly sorts). The default value is <code>false</code>.
 * @return <code>true</code> if the index was created successfully, <code>false</code> otherwise.
 * @memberof MongoTool
 */
GRASSROOTS_MONGODB_API bool AddCollectionSingleIndex (MongoTool *tool_p, const char *database_s, const char * const collection_s, const char *key_s, const char *index_type_s, const bool unique_flag, const bool sparse_flag);


/**
 * Create a Collection.
 *
 * @param tool_p The MongoTool that will be used to create the index.
 * @param collection_s The name of the collection to create.
 * @param opts_p The MongoDB options for creating the collection. Leave this as <code>NULL</code>
 * to use the defaults. See https://www.mongodb.com/docs/manual/reference/command/create/ for
 * more information.
 * @return <code>true</code> if the collection was created successfully, <code>false</code> otherwise.
 * @memberof MongoTool
 */
GRASSROOTS_MONGODB_API bool CreateMongoToolCollection (MongoTool *tool_p, const char *collection_s, bson_t *opts_p);


/**
 * Check if a named Collection exists.
 *
 * @param tool_p The MongoTool that will be used to create the index.
 * @param collection_s The name of the collection to check.
 * @return 1 if the collection already exists, 0 if it does not or -1 upon error.
 * @memberof MongoTool
 */
GRASSROOTS_MONGODB_API int DoesCollectionExist (MongoTool *tool_p, const char *collection_s);



/**
 * Drop an index with a given name from a Collection
 *
 * @param tool_p The MongoTool that will be used to create the index.
 * @param index_s The index to remove.
 * @return <code>true</code> if the index was dropped successfully, <code>false</code> otherwise.
 * @memberof MongoTool
 */
GRASSROOTS_MONGODB_API bool DropCollectionIndex (MongoTool *tool_p, const char *index_s);



#ifdef __cplusplus
}
#endif



#endif /* MONGODB_TOOL_H_ */
