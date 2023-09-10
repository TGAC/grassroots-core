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
/*
 * mongodb_tool.c
 *
 *  Created on: 26 Jun 2015
 *      Author: billy
 */
#include <math.h>

#define ALLOCATE_MONGODB_TAGS (1)
#include "mongodb_tool.h"
#include "memory_allocations.h"
#include "streams.h"
#include "json_tools.h"
#include "grassroots_server.h"
#include "mongodb_util.h"
#include "json_util.h"
#include "search_options.h"
#include "mongo_client_manager.h"
#include "string_utils.h"
#include "time_util.h"


static bool AddSimpleTypeToQuery (bson_t *query_p, const char *key_s, const json_t *value_p);


static bool AddChild (bson_t *parent_p, const char * const key_s, const char * const sub_key_s, const json_t * const value_p, const json_t *options_p);

static bool AddSearchOptions (bson_t *query_p, const json_t *options_p);


static int CompareStrings (const void *v0_p, const void *v1_p);


static bson_t *MakeQuery (const char **keys_ss, const size_t num_keys, const json_t *data_p);

static bool AddCollectionIndex (MongoTool *tool_p, const char *database_s, const char * const collection_s, bson_t *keys_p, const bool unique_flag, const bool sparse_flag);


#ifdef _DEBUG
#define MONGODB_TOOL_DEBUG	(STM_LEVEL_INFO)
#else
#define MONGODB_TOOL_DEBUG	(STM_LEVEL_NONE)
#endif



MongoTool *AllocateMongoTool (mongoc_client_t *client_p, struct MongoClientManager *mongo_manager_p)
{
	MongoTool *tool_p = (MongoTool *) AllocMemory (sizeof (MongoTool));

	if (tool_p)
		{
			bool owns_flag = false;

			if (!client_p)
				{
					client_p = GetMongoClientFromMongoClientManager (mongo_manager_p);
					owns_flag = true;
				}

			if (client_p)
				{
					tool_p -> mt_client_p = client_p;
					tool_p -> mt_owns_client_flag = owns_flag;
					tool_p -> mt_database_p = NULL;
					tool_p -> mt_collection_p = NULL;
					tool_p -> mt_cursor_p = NULL;
					tool_p -> mt_manager_p = mongo_manager_p;

					return tool_p;
				}		/* if (client_p) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get mongoc_client");
				}

			FreeMemory (tool_p);
		}		/* if (tool_p) */

	return NULL;
}


void FreeMongoTool (MongoTool *tool_p)
{
	if (tool_p -> mt_collection_p)
		{
			mongoc_collection_destroy (tool_p -> mt_collection_p);
		}

	if (tool_p -> mt_cursor_p)
		{
			mongoc_cursor_destroy (tool_p -> mt_cursor_p);
		}

	if (tool_p -> mt_database_p)
		{
			mongoc_database_destroy (tool_p -> mt_database_p);
		}

	if ((tool_p -> mt_owns_client_flag) && (tool_p -> mt_client_p) && (tool_p -> mt_manager_p))
		{
			ReleaseMongoClientFromMongoClientManager (tool_p -> mt_manager_p, tool_p -> mt_client_p);
		}

	FreeMemory (tool_p);
}


bool SetMongoToolDatabaseAndCollection (MongoTool *tool_p, const char *db_s, const char *collection_s)
{
	mongoc_database_t *database_p = mongoc_client_get_database (tool_p -> mt_client_p, db_s);

	if (database_p)
		{
			mongoc_collection_t *collection_p =  mongoc_client_get_collection (tool_p -> mt_client_p, db_s, collection_s);

			if (collection_p)
				{
					if (tool_p -> mt_database_p)
						{
							mongoc_database_destroy (tool_p -> mt_database_p);
						}

					if (tool_p -> mt_collection_p)
						{
							mongoc_collection_destroy (tool_p -> mt_collection_p);
						}

					tool_p -> mt_database_p = database_p;
					tool_p -> mt_collection_p = collection_p;

					return true;
				}

			mongoc_database_destroy (database_p);
		}

	return false;
}


bool SetMongoToolDatabase (MongoTool *tool_p, const char *db_s)
{
	bool success_flag = false;
	mongoc_database_t *database_p = mongoc_client_get_database (tool_p -> mt_client_p, db_s);

	if (database_p)
		{
			if (tool_p -> mt_database_p)
				{
					mongoc_database_destroy (tool_p -> mt_database_p);
				}

			if (tool_p -> mt_collection_p)
				{
					mongoc_collection_destroy (tool_p -> mt_collection_p);
				}

			tool_p -> mt_database_p = database_p;
			tool_p -> mt_collection_p = NULL;

			success_flag = true;
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get database \"%s\"", db_s);
		}

	return success_flag;
}


bool CreateMongoToolCollection (MongoTool *tool_p, const char *collection_s, bson_t *opts_p)
{
	bool success_flag = false;

	if (tool_p -> mt_database_p)
		{
			bson_error_t err;
			mongoc_collection_t *collection_p =  mongoc_database_create_collection (tool_p -> mt_database_p, collection_s, opts_p, &err);

			if (collection_p)
				{
					success_flag = true;
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "mongoc_client_create_collection () failed for \"%s\", err: \"%s\", code %d", collection_s, err.message, err.code);
				}
		}

	return success_flag;
}


int DoesCollectionExist (MongoTool *tool_p, const char *collection_s)
{
	bson_error_t error_val;
	int res = 0;
	bool exists_flag = mongoc_database_has_collection (tool_p -> mt_database_p, collection_s, &error_val);

	if (exists_flag)
		{
			res = 1;
		}
	else
		{
			/* Was there an error? */
			if (error_val.code)
				{
					res = -1;
				}
		}

	return res;
}



bool SetMongoToolCollection (MongoTool *tool_p, const char *collection_s)
{
	bool success_flag = false;

	if (tool_p -> mt_database_p)
		{
			const char *database_s = mongoc_database_get_name (tool_p -> mt_database_p);

			if (database_s)
				{
					mongoc_collection_t *collection_p =  mongoc_client_get_collection (tool_p -> mt_client_p, database_s, collection_s);

					if (collection_p)
						{
							if (tool_p -> mt_collection_p)
								{
									mongoc_collection_destroy (tool_p -> mt_collection_p);
								}

							tool_p -> mt_collection_p = collection_p;
							success_flag = true;
						}
				}
		}

	return success_flag;
}



bool SaveMongoDataFromBSON (MongoTool *mongo_p, const bson_t *data_to_save_p, const char *collection_s, bson_t *selector_p)
{
	return SaveAndBackupMongoDataFromBSON (mongo_p, data_to_save_p, collection_s, NULL, NULL, selector_p);
}


bool SaveAndBackupMongoDataFromBSON (MongoTool *mongo_p, const bson_t *data_to_save_p, const char *collection_s, const char *backup_collection_s, const char *id_key_s, bson_t *selector_p)
{
	bool success_flag = false;
	bool prepare_flag = true;

	if (collection_s)
		{
			if (!SetMongoToolCollection (mongo_p, collection_s))
				{
					prepare_flag = false;
				}
		}

	if (prepare_flag)
		{
			bson_t *reply_p = NULL;
			bson_error_t error_val;

			if (!selector_p)
				{
					if (InsertMongoDataAsBSON (mongo_p, data_to_save_p, &reply_p, &error_val))
						{
							success_flag = true;
						}
				}		/* if (insert_flag) */
			else
				{
					/*
					 * Are we backing up the current version of the document so that we can access its
					 * full history?
					 */
					if (backup_collection_s && id_key_s)
						{
							/* Get the existing current document */
							json_t *existing_docs_p = GetAllMongoResultsAsJSON (mongo_p, selector_p, NULL);

							if (existing_docs_p)
								{
									prepare_flag = false;

									if ((json_is_array (existing_docs_p)) && (json_array_size (existing_docs_p) == 1))
										{
											/* save it to the backup collection */
											if (SetMongoToolCollection (mongo_p, backup_collection_s))
												{
													json_t *existing_doc_p = json_array_get (existing_docs_p, 0);															
													json_t *id_p = json_object_get (existing_doc_p, MONGO_ID_S);
										
													if (id_p)
														{
															if (json_object_set (existing_doc_p, id_key_s, id_p) == 0)
																{
																	if (json_object_del (existing_doc_p, MONGO_ID_S) == 0)
																		{
																			if (InsertMongoData (mongo_p, existing_doc_p, &reply_p, &error_val))
																				{
																					if (SetMongoToolCollection (mongo_p, collection_s))
																						{
																							prepare_flag = true;
																						}
																					else
																						{
																							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "SetMongoToolCollection () failed for \"%s\"", collection_s);
																						}
																				}
																			else
																				{
																					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, existing_doc_p, 
																						"InsertMongoData () failed for \"%s\", err: \"%s\", code %d", backup_collection_s, error_val.message, error_val.code);																						
																				}
																	
																		}	
																	else
																		{
																			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, existing_doc_p, 
																						"json_object_del () failed for \"%s\"", MONGO_ID_S);																						
																		}
		
																}
															else
																{
																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, existing_doc_p, 
																				"json_object_set () failed for \"%s\"", id_key_s);																					
																}
														}
													else
														{
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, existing_doc_p, "json_object_get () failed for \"%s\"", MONGO_ID_S);																			
														}
												}
											else
												{
													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "SetMongoToolCollection () failed for \"%s\"", backup_collection_s);
												}

										}		/* if ((json_is_array (existing_docs_p)) && (json_array_size (existing_docs_p) == 1)) */
									else
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, existing_docs_p, "existing docs aren't an array of size 1");																														
										}

									json_decref (existing_docs_p);
								}

						}		/* if (backup_collection_s) */

					if (prepare_flag)
						{
							/* it's an update */
							if (SetMongoDataAsBSON (mongo_p, selector_p, data_to_save_p, &reply_p))
								{
									success_flag = true;
								}
							else
								{
									PrintBSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, data_to_save_p, "SetMongoDataAsBSON () failed");																																							
								}
						}


				}


			if (reply_p)
				{
					bson_destroy (reply_p);
				}

		}		/* if (prepare_flag) */

	return success_flag;

}

bool SaveMongoDataWithTimestamp (MongoTool *mongo_p, const json_t *data_to_save_p, const char *collection_s, bson_t *selector_p, const char *timestamp_key_s)
{
	return SaveAndBackupMongoDataWithTimestamp (mongo_p, data_to_save_p, collection_s, NULL, NULL, selector_p, timestamp_key_s);
}


bool SaveAndBackupMongoDataWithTimestamp (MongoTool *mongo_p, const json_t *data_to_save_p, const char *collection_s, const char *backup_collection_s, const char *id_key_s, bson_t *selector_p, const char *timestamp_key_s)
{
	bool success_flag = false;
	bson_t *doc_p = ConvertJSONToBSON (data_to_save_p);

	if (doc_p)
		{
			struct tm tm;

			if (GetPresentTime (&tm))
				{
					char *time_s = GetTimeAsString (&tm, true, NULL);

					if (time_s)
						{
							if (! (BSON_APPEND_UTF8 (doc_p, timestamp_key_s, time_s)))
								{
									PrintBSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, doc_p, "Failed to add \"%s\": \"%s\" as timestamp", timestamp_key_s, time_s);
								}

							FreeMemory (time_s);
						}
					else
						{
							PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to get current time as string");
						}
				}
			else
				{
					PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to get current time");
				}

			success_flag = SaveAndBackupMongoDataFromBSON (mongo_p, doc_p, collection_s, backup_collection_s, id_key_s, selector_p);

			bson_destroy (doc_p);
		}

	return success_flag;
}


bool SaveMongoData (MongoTool *mongo_p, const json_t *data_to_save_p, const char *collection_s, bson_t *selector_p)
{
	return SaveAndBackupMongoData (mongo_p, data_to_save_p, collection_s, NULL, NULL, selector_p);
}


bool SaveAndBackupMongoData (MongoTool *mongo_p, const json_t *data_to_save_p, const char *collection_s, const char *backup_collection_s, const char *id_key_s, bson_t *selector_p)
{
	bool success_flag = false;
	bson_t *doc_p = ConvertJSONToBSON (data_to_save_p);

	if (doc_p)
		{
			success_flag = SaveAndBackupMongoDataFromBSON (mongo_p, doc_p, collection_s, backup_collection_s, id_key_s, selector_p);

			bson_destroy (doc_p);
		}

	return success_flag;
}


bson_t *ConvertJSONToBSON (const json_t *json_p)
{
	bson_t *bson_p = NULL;
	char *value_s = json_dumps (json_p, JSON_INDENT (2));

	if (value_s)
		{
			bson_error_t error;

			bson_p = bson_new_from_json ((const uint8 *) value_s, -1, &error);

			if (!bson_p)
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to convert %s to BSON, error %s\n", value_s, error.message);
				}

			free (value_s);
		}		/* if (value_s) */

	return bson_p;
}


json_t *ConvertBSONToJSON (const bson_t *bson_p)
{
	json_t *json_p = NULL;
	char *value_s = bson_as_json (bson_p, NULL);

	if (value_s)
		{
			json_error_t error;

			json_p = json_loads (value_s, 0, &error);

			if (json_p)
				{
#if MONGODB_TOOL_DEBUG >= STM_LEVEL_FINE
					PrintLog (STM_LEVEL_FINE, __FILE__, __LINE__, "raw bson data:\n", value_s);
					PrintJSONToLog (STM_LEVEL_FINE, __FILE__, __LINE__, json_p, "bson to json data:");
#endif
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to convert %s to JSON, error %s\n", value_s, error.text);
				}

			bson_free (value_s);
		}		/* if (value_s) */

	return json_p;
}


bson_oid_t *InsertJSONIntoMongoCollection (MongoTool *tool_p, json_t *json_p)
{
	bson_oid_t *id_p = NULL;

	if (tool_p -> mt_collection_p)
		{
			bson_t *bson_p = ConvertJSONToBSON (json_p);

			if (bson_p)
				{
					id_p = (bson_oid_t *) AllocMemory (sizeof (bson_oid_t));

					if (id_p)
						{
							bool success_flag = false;
							bson_oid_init (id_p, NULL);

							if (BSON_APPEND_OID (bson_p, MONGO_ID_S, id_p))
								{
									bson_error_t error;
									success_flag = mongoc_collection_insert (tool_p -> mt_collection_p, MONGOC_INSERT_NONE, bson_p, NULL, &error);

									if (!success_flag)
										{
											char *value_s = json_dumps (json_p, JSON_INDENT (2));

											if (value_s)
												{
													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to insert %s, error %s\n", value_s, error.message);
													free (value_s);
												}
											else
												{
													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to insert json doc, error %s\n", error.message);
												}

										}		/* if (!success_flag) */

								}		/* if (BSON_APPEND_OID (bson_p, MONGO_ID_S, id_p)) */

							if (!success_flag)
								{
									FreeMemory (id_p);
									id_p = NULL;
								}

						}		/* if (id_p) */

					bson_destroy (bson_p);
				}		/* if (bson_p) */

		}		/* if (tool_p -> mt_collection_p) */

	return id_p;
}


/*
 * Based upon the example at http://mongoc.org/libmongoc/current/create-indexes.html
 */

bool AddCollectionCompoundIndex (MongoTool *tool_p, const char *database_s, const char * const collection_s, const char ** const keys_ss, const bool unique_flag, const bool sparse_flag)
{
	bool success_flag = false;
	bool setup_flag = true;
	bson_t keys;
	const char **key_ss = keys_ss;

	bson_init (&keys);

	while (setup_flag && (*key_ss))
		{
			if (BSON_APPEND_INT32 (&keys, *key_ss, 1))
				{
					++ key_ss;
				}
			else
				{
					setup_flag = false;
				}
		}

	if (setup_flag)
		{
			success_flag = AddCollectionIndex (tool_p, database_s, collection_s, &keys, unique_flag, sparse_flag);
		}

	return success_flag;
}



bool AddCollectionSingleIndex (MongoTool *tool_p, const char *database_s, const char * const collection_s, const char *key_s, const bool unique_flag, const bool sparse_flag)
{
	bool success_flag = false;
	bson_t keys;

	bson_init (&keys);

	if (BSON_APPEND_INT32 (&keys, key_s, 1))
		{
			success_flag = AddCollectionIndex (tool_p, database_s, collection_s, &keys, unique_flag, sparse_flag);
		}

	return success_flag;
}


bool DropCollectionIndex (MongoTool *tool_p, const char *index_s)
{
	bool success_flag = false;
	bson_error_t err;

	if (mongoc_collection_drop_index (tool_p -> mt_collection_p, MONGO_ID_S, &err))
		{
			success_flag = true;
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "mongoc_collection_drop_index () failed, error domain: " UINT32_FMT " code: " UINT32_FMT " message: \"%s\"", err.domain, err.code, err.message);
		}

	return success_flag;
}



static bool AddCollectionIndex (MongoTool *tool_p, const char *database_s, const char * const collection_s, bson_t *keys_p, const bool unique_flag, const bool sparse_flag)
{
	bool success_flag = false;
	bool setup_flag = false;

	if (database_s)
		{
			setup_flag = SetMongoToolDatabaseAndCollection (tool_p, database_s, collection_s);
		}
	else
		{
			setup_flag = SetMongoToolCollection (tool_p, collection_s);
		}

	if (setup_flag)
		{
			char *index_s = mongoc_collection_keys_to_index_string (keys_p);

			if (index_s)
				{
					bson_t *doc_p = bson_new ();

					if (doc_p)
						{
							bool added_to_command_flag = false;

							if (BSON_APPEND_DOCUMENT (doc_p, "key", keys_p))
								{
									if (BSON_APPEND_UTF8 (doc_p, "name", index_s))
										{
											if ((!unique_flag) || (BSON_APPEND_BOOL (doc_p, "unique", true)))
												{
													if ((!sparse_flag) || (BSON_APPEND_BOOL (doc_p, "sparse", true)))
														{
															bson_t *create_indexes_command_p = BCON_NEW ("createIndexes",
																																					 BCON_UTF8 (collection_s),
																																					 "indexes",
																																					 "[",
																																					 BCON_DOCUMENT (doc_p),
																																					 "]");

															if (create_indexes_command_p)
																{
																	bson_t reply;
																	char *reply_s = NULL;
																	bson_error_t error;

																	added_to_command_flag = true;

																	if (mongoc_database_write_command_with_opts (tool_p -> mt_database_p, create_indexes_command_p, NULL, &reply, &error))
																		{
																			success_flag = true;
																		}
																	else
																		{
																			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "mongoc_database_write_command_with_opts () failed for \"%s\", \"%s\": \"%s\"",
																									 database_s, collection_s, error.message);
																		}

																	reply_s = bson_as_json (&reply, NULL);

																	if (reply_s)
																		{
																			PrintLog (STM_LEVEL_INFO, __FILE__, __LINE__, "Reply for creating index for \"%s\", \"%s\": \"%s\"",
																								database_s, collection_s, reply_s);

																			bson_free (reply_s);
																		}		/* if (reply_s) */

																	bson_destroy (&reply);


																	bson_destroy (create_indexes_command_p);
																}		/* if (create_indexes_command_p) */
														}
												}
										}

								}


							if (!added_to_command_flag)
								{
									bson_destroy (doc_p);
								}

						}

				}		/* if (setup_flag) */


		}		/* if (setup_flag) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "SetMongoToolDatabaseAndCollection () failed for \"%s\", \"%s\"",
									 database_s ? database_s : "NULL", collection_s);
		}

	return success_flag;
}


bool UpdateMongoDocumentsByJSON (MongoTool *tool_p, const json_t *query_p, const json_t *update_p, const bool multiple_flag)
{
	bool success_flag = false;
	bson_t *query_bson_p = ConvertJSONToBSON (query_p);

	if (query_bson_p)
		{
			success_flag = UpdateMongoDocumentsByBSON (tool_p, query_bson_p, update_p, multiple_flag);

			bson_destroy (query_bson_p);
		}

	return success_flag;

}


bool UpdateMongoDocuments (MongoTool *tool_p, const bson_oid_t *id_p, const json_t *update_p, const bool multiple_flag)
{
	bool success_flag = false;
	bson_t *query_p = BCON_NEW (MONGO_ID_S, BCON_OID (id_p));

	if (query_p)
		{
			success_flag = UpdateMongoDocumentsByBSON (tool_p, query_p, update_p, multiple_flag);

			bson_destroy (query_p);
		}		/* if (query_p) */

	return success_flag;
}


bool UpdateMongoDocumentsByBSON (MongoTool *tool_p, const bson_t *query_p, const json_t *update_p, const bool multiple_flag)
{
	bool success_flag = false;

	if (tool_p -> mt_collection_p)
		{
			bson_t *data_p = ConvertJSONToBSON (update_p);

			if (data_p)
				{
					bson_t *update_statement_p = bson_new ();

					if (update_statement_p)
						{
							if (bson_append_document (update_statement_p, "$set", -1, data_p))
								{
									bson_error_t error;

#if MONGODB_TOOL_DEBUG >= STM_LEVEL_FINE
									PrintBSONToLog (STM_LEVEL_FINE, __FILE__, __LINE__, query_p, "UpdateMongoDocument query_p");
									PrintBSONToLog (STM_LEVEL_FINE, __FILE__, __LINE__, update_statement_p, "UpdateMongoDocument update_statement_p");
#endif

									if (multiple_flag)
										{
											success_flag = mongoc_collection_update_many (tool_p -> mt_collection_p, query_p, update_statement_p, NULL, NULL, &error);
										}
									else
										{
											success_flag = mongoc_collection_update_one (tool_p -> mt_collection_p, query_p, update_statement_p, NULL, NULL, &error);
										}

								}		/* if (bson_append_document (update_statement_p, "$set", -1, bson_p)) */

							bson_destroy (update_statement_p);
						}		/* if (query_p) */

					bson_destroy (data_p);
				}		/* if (data_p) */

		}		/* if (tool_p -> mt_collection_p) */

	return success_flag;
}



bool RemoveMongoDocuments (MongoTool *tool_p, const json_t *selector_json_p, const bool remove_first_match_only_flag)
{
	bool success_flag = false;
	bson_t *selector_p = ConvertJSONToBSON (selector_json_p);

	if (selector_p)
		{
			success_flag = RemoveMongoDocumentsByBSON (tool_p, selector_p, remove_first_match_only_flag);
			bson_destroy (selector_p);
		}		/* if (selector_p) */


	return success_flag;
}


bool RemoveMongoDocumentsByBSON (MongoTool *tool_p, const bson_t *selector_p, const bool remove_first_match_only_flag)
{
	bool success_flag = false;

	if (tool_p -> mt_collection_p)
		{
			bson_error_t error;
			mongoc_remove_flags_t flags = remove_first_match_only_flag ? MONGOC_REMOVE_SINGLE_REMOVE : MONGOC_REMOVE_NONE;

			if (mongoc_collection_remove (tool_p -> mt_collection_p, flags, selector_p, NULL, &error))
				{
					success_flag = true;
				}		/* if (mongoc_collection_update (tool_p -> mt_collection_p, MONGOC_UPDATE_NONE, query_p, update_statement_p, NULL, &error)) */

		}		/* if (tool_p -> mt_collection_p) */

	return success_flag;
}




static bool AddSimpleTypeToQuery (bson_t *query_p, const char *key_s, const json_t *value_p)
{
	bool success_flag = false;
	const int json_datatype = json_typeof (value_p);

	switch (json_datatype)
	{
		/*
				JSON_OBJECT
				JSON_ARRAY
				JSON_STRING
				JSON_INTEGER
				JSON_REAL
				JSON_TRUE
				JSON_FALSE
				JSON_NULL
		 */
		case JSON_STRING:
			success_flag = BSON_APPEND_UTF8 (query_p, key_s, json_string_value (value_p));
			break;

		case JSON_INTEGER:
#if JSON_INTEGER_IS_LONG_LONG
			success_flag = BSON_APPEND_INT64 (query_p, key_s, json_integer_value (value_p));
#else
			success_flag = BSON_APPEND_INT32 (query_p, key_s, json_integer_value (value_p));
#endif
			break;

		case JSON_REAL:
			success_flag = BSON_APPEND_DOUBLE (query_p, key_s, json_real_value (value_p));
			break;

		case JSON_TRUE:
			success_flag = BSON_APPEND_BOOL (query_p, key_s, true);
			break;

		case JSON_FALSE:
			success_flag = BSON_APPEND_BOOL (query_p, key_s, false);
			break;

		case JSON_OBJECT:
		case JSON_ARRAY:
		case JSON_NULL:
			PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "json object type %d is not yet supported", json_datatype);
			break;

	}

	return success_flag;
}


static bool AddChild (bson_t *parent_p, const char * const key_s, const char * const sub_key_s, const json_t * const value_p, const json_t *options_p)
{
	bool success_flag = false;
	bson_t child;

	if (BSON_APPEND_DOCUMENT_BEGIN (parent_p, key_s, &child))
		{
			PrintBSONToLog (STM_LEVEL_FINEST, __FILE__, __LINE__, parent_p, "parent 1: ");
			PrintBSONToLog (STM_LEVEL_FINEST, __FILE__, __LINE__, &child, "child 1: ");

			if (AddSimpleTypeToQuery (&child, sub_key_s, value_p))
				{
					PrintBSONToLog (STM_LEVEL_FINEST, __FILE__, __LINE__, parent_p, "parent 2: ");
					PrintBSONToLog (STM_LEVEL_FINEST, __FILE__, __LINE__, &child, "child 2: ");

					success_flag = true;

					if (options_p)
						{
							success_flag = AddSearchOptions (&child, options_p);
						}

					if (success_flag)
						{
							success_flag = bson_append_document_end (parent_p, &child);

							PrintBSONToLog (STM_LEVEL_FINEST, __FILE__, __LINE__, parent_p, "parent 3: ");
							PrintBSONToLog (STM_LEVEL_FINEST, __FILE__, __LINE__, &child, "child 3: ");
						}
				}
		}		/* if (BSON_APPEND_DOCUMENT_BEGIN (parent_p, key_s, &child)) */


	return success_flag;
}


static bool AddSearchOptions (bson_t *query_p, const json_t *options_p)
{
	bool success_flag = true;
	ByteBuffer *buffer_p = AllocateByteBuffer (32);

	if (buffer_p)
		{
			if (json_is_object (options_p))
				{
					json_t *option_value_p = json_object_get (options_p, SO_CASE_INSENSITIVE_S);

					if (option_value_p)
						{
							if (((json_is_boolean (option_value_p)) && (json_is_true (option_value_p))) ||
									((json_is_string (option_value_p)) && (strcmp (json_string_value (option_value_p), "true") == 0)))
								{
									success_flag = AppendStringToByteBuffer (buffer_p, "i");
								}
						}
				}		/* if (json_is_object (options_p)) */

			if (success_flag && (GetByteBufferSize (buffer_p) > 0))
				{
					const char *value_s = GetByteBufferData (buffer_p);
					success_flag = BSON_APPEND_UTF8 (query_p, "$options", value_s);
				}

			FreeByteBuffer (buffer_p);
		}		/* if (buffer_p) */

	return success_flag;
}


bool AddToQuery (bson_t *query_p, const char *key_s, const json_t *json_clause_p)
{
	bool success_flag = false;

	switch (json_typeof (json_clause_p))
	{
		/*
				JSON_OBJECT
				JSON_ARRAY
				JSON_STRING
				JSON_INTEGER
				JSON_REAL
				JSON_TRUE
				JSON_FALSE
				JSON_NULL
		 */
		case JSON_STRING:
			success_flag = BSON_APPEND_UTF8 (query_p, key_s, json_string_value (json_clause_p));
			break;

		case JSON_INTEGER:
#if JSON_INTEGER_IS_LONG_LONG
			success_flag = BSON_APPEND_INT64 (query_p, key_s, json_integer_value (json_clause_p));
#else
			success_flag = BSON_APPEND_INT32 (query_p, key_s, json_integer_value (json_clause_p));
#endif
			break;

		case JSON_REAL:
			success_flag = BSON_APPEND_DOUBLE (query_p, key_s, json_real_value (json_clause_p));
			break;

		case JSON_TRUE:
			success_flag = BSON_APPEND_BOOL (query_p, key_s, true);
			break;

		case JSON_FALSE:
			success_flag = BSON_APPEND_BOOL (query_p, key_s, false);
			break;

		case JSON_ARRAY:
			{
				bson_t *in_p = bson_new ();

				if (in_p)
					{
						if (bson_append_array_begin (query_p, key_s, -1, in_p))
							{
								const size_t size = json_array_size (json_clause_p);
								size_t i;
								uint32 buffer_length = ((uint32) (ceil (log10 ((double) size)))) + 2;

								char *buffer_s = (char *) AllocMemory (buffer_length * sizeof (char));

								if (buffer_s)
									{
										for (i = 0; i < size; ++ i)
											{
												const char *index_p;

												if (bson_uint32_to_string (i, &index_p, buffer_s, buffer_length) > 0)
													{
														json_t *element_p = json_array_get (json_clause_p, i);
														AddSimpleTypeToQuery (in_p, index_p, json_clause_p);

													}
											}

										FreeMemory (buffer_s);
									}		/* if (buffer_s) */

								if (!bson_append_array_end (query_p, in_p))
									{

									}

							}		/* if (bson_append_array_begin (query_p, key_s, -1, in_p)) */

					}		/* if (in_p) */

			}
			break;

		case JSON_OBJECT:
			{
				/*
				 * key:  one of "=", "<", "<=", ">", ">=", "in", "range", "not", "like"
				 * value: can be single value or array. For a "range" key, it will be an array
				 * of 2 elements that are the inclusive lower and upper bounds.
				 */
				json_t *operator_p = json_object_get (json_clause_p, MONGO_CLAUSE_OPERATOR_S);
				json_t *value_p = json_object_get (json_clause_p, MONGO_CLAUSE_VALUE_S);

				if (operator_p && value_p)
					{
						if (json_is_string (operator_p))
							{
								const char *op_s = json_string_value (operator_p);

								if (strcmp (op_s, SO_RANGE_S) == 0)
									{
										if (json_is_array (value_p))
											{
												if (json_array_size (value_p) == 2)
													{
														json_t *range_value_p = json_array_get (value_p, 0);

														if (AddChild (query_p, key_s, "$gte", range_value_p, json_clause_p))
															{
																range_value_p = json_array_get (value_p, 1);

																if (AddChild (query_p, key_s, "$lte", range_value_p, json_clause_p))
																	{
																		success_flag = true;
																	}

															}
													}
											}
									}
								else
									{
										const char *token_s = NULL;

										if (strcmp (op_s, SO_EQUALS_S) == 0)
											{
												token_s = "$eq";
											}
										else if (strcmp (op_s, SO_LESS_THAN_S) == 0)
											{
												token_s = "$lt";
											}
										else if (strcmp (op_s, SO_LESS_THAN_OR_EQUALS_S) == 0)
											{
												token_s = "$lte";
											}
										else if (strcmp (op_s, SO_GREATER_THAN_S) == 0)
											{
												token_s = "$gt";
											}
										else if (strcmp (op_s, SO_GREATER_THAN_OR_EQUALS_S) == 0)
											{
												token_s = "$gte";
											}
										else if (strcmp (op_s, SO_NOT_EQUALS_S) == 0)
											{
												token_s = "$ne";
											}
										else if (strcmp (op_s, SO_LIKE_S) == 0)
											{
												token_s = "$regex";
											}
										else
											{
												PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Unknown operation %s", op_s);
											}

										if (token_s)
											{
												success_flag = AddChild (query_p, key_s, token_s, value_p, json_clause_p);
											}
									}
							}
					}

			}
			break;

		case JSON_NULL:
			success_flag = true;
			break;

		default:
			break;
	}		/* switch (json_typeof (json_p)) */


#if MONGODB_TOOL_DEBUG >= STM_LEVEL_FINER
	PrintBSONToLog (STM_LEVEL_FINE, __FILE__, __LINE__, query_p, "bson search query");
#endif

	return success_flag;
}


bson_t *GenerateQuery (const json_t *json_p)
{
	bson_t *query_p = NULL;

	/*
	 * loop through each of the key:value queries
	 *
	 * if value is not an object or array, treat the statement as key = value
	 * if value is an array, treat it as in.
	 * if value is an object, value can have the following fields:
	 *
	 * key:  one of "=", "<", "<=", ">", ">=", "in", "range", "not"
	 * value: can be single value or array. For a "range" key, it will be an array
	 * of 2 elements that are the inclusive lower and upper bounds.
	 */

	if (json_p)
		{
			query_p = bson_new ();

#if MONGODB_TOOL_DEBUG >= STM_LEVEL_FINE
			PrintJSONToLog (STM_LEVEL_FINE, __FILE__, __LINE__, json_p, "json search query");
#endif


			if (query_p)
				{
					const char *key_s;
					json_t *value_p;

					json_object_foreach (json_p, key_s, value_p)
					{
						AddToQuery (query_p, key_s, value_p);
					}		/* json_object_foreach (json_p, key_p, value_p) */

#if MONGODB_TOOL_DEBUG >= STM_LEVEL_FINE
					PrintBSONToLog (STM_LEVEL_FINE, __FILE__, __LINE__, query_p, "final bson search query");
#endif

				}		/* if (query_p) */

		}		/* if (json_p) */



	return query_p;
}


bool FindMatchingMongoDocumentsByJSON (MongoTool *tool_p, const json_t *query_json_p, const char **fields_ss, bson_t *extra_opts_p)
{
	bool success_flag = false;
	bson_t *query_p = GenerateQuery (query_json_p);  // ConvertJSONToBSON (query_json_p);

	if (query_p)
		{
#if MONGODB_TOOL_DEBUG >= STM_LEVEL_FINER
			PrintBSONToLog (STM_LEVEL_FINER, __FILE__, __LINE__, query_p, "query: ");
#endif

			success_flag = FindMatchingMongoDocumentsByBSON (tool_p, query_p, fields_ss, extra_opts_p);
			bson_destroy (query_p);
		}

	return success_flag;
}


void LogAllBSON (const bson_t *bson_p, const int level, const char * const file_s, const int line, const char * const prefix_s)
{
	bson_iter_t iter;
	bson_iter_t sub_iter;

	if (bson_iter_init (&iter, bson_p))
		{
			if ((BSON_ITER_HOLDS_DOCUMENT (&iter) || BSON_ITER_HOLDS_ARRAY (&iter)) && bson_iter_recurse (&iter, &sub_iter))
				{
					while (bson_iter_next (&sub_iter))
						{
							PrintLog (level, file_s, line, "%s: %s", prefix_s, bson_iter_key (&sub_iter));
						}
				}
		}
}


int PrintBSONToLog (const uint32 level, const char *filename_s, const int line_number, const bson_t *bson_p, const char *message_s, ...)
{
	int result = -1;
	va_list args;

	va_start (args, message_s);

	result = PrintLogVarArgs (level, filename_s, line_number, message_s, args);

	if (bson_p)
		{
			size_t len;
			char *dump_s = bson_as_json (bson_p, &len);

			if (dump_s)
				{
					PrintLog (level, filename_s, line_number, "%s", dump_s);
					bson_free (dump_s);
				}
		}

	return result;
}


int PrintBSONToErrors (const uint32 level, const char *filename_s, const int line_number, const bson_t *bson_p, const char *message_s, ...)
{
	int result = -1;
	va_list args;

	va_start (args, message_s);

	result = PrintErrorsVarArgs (level, filename_s, line_number, message_s, args);

	if (bson_p)
		{
			size_t len;
			char *dump_s = bson_as_json (bson_p, &len);

			if (dump_s)
				{
					PrintErrors (level, filename_s, line_number, "%s", dump_s);
					bson_free (dump_s);
				}
		}

	return result;
}




void LogBSONOid (const bson_oid_t *bson_p, const int level, const char * const filename_s, const int line, const char * const prefix_s)
{
	char buffer_s [25];

	bson_oid_to_string (bson_p, buffer_s);

	if (prefix_s)
		{
			PrintLog (level, filename_s, line, "%s %s", prefix_s, buffer_s);
		}
	else
		{
			PrintLog (level, filename_s, line, "%s", buffer_s);

		}
}


bool FindMatchingMongoDocumentsByBSON (MongoTool *tool_p, const bson_t *query_p, const char **fields_ss, bson_t *extra_opts_p)
{
	bool success_flag = false;

	if (tool_p -> mt_collection_p)
		{
			bson_t *fields_p = NULL;
			mongoc_cursor_t *cursor_p = NULL;

			success_flag = true;

			/*
			 * Add the fields to retrieve
			 */
			if (fields_ss)
				{
					if (extra_opts_p)
						{
							fields_p = bson_copy (extra_opts_p);
						}
					else
						{
							fields_p = bson_new ();
						}

					if (fields_p)
						{
							const char **field_ss = fields_ss;

							while (success_flag && *field_ss)
								{
									if (BSON_APPEND_INT32 (fields_p, *field_ss, 1))
										{
											++ field_ss;
										}
									else
										{
											success_flag = false;
										}

								}		/* while (success_flag && field_ss) */

						}		/* if (fields_p) */

				}		/* if (fields_ss) */


#if MONGODB_TOOL_DEBUG >= STM_LEVEL_FINE
			PrintBSONToLog (STM_LEVEL_FINE, __FILE__, __LINE__, query_p, "mongo query");
#endif


			cursor_p = mongoc_collection_find_with_opts (tool_p -> mt_collection_p, query_p, extra_opts_p, NULL);

			if (cursor_p)
				{
					if (tool_p -> mt_cursor_p)
						{
							mongoc_cursor_destroy (tool_p -> mt_cursor_p);
						}

					tool_p -> mt_cursor_p = cursor_p;
					success_flag = HasMongoQueryResults (tool_p);
				}

			if (fields_p)
				{
					bson_destroy (fields_p);
				}		/* if (fields_p) */


		}		/* if (tool_p -> mt_collection_p) */

	return success_flag;
}


bool HasMongoQueryResults (MongoTool *tool_p)
{
	bool results_flag = false;

	/*
	 * mongoc_cursor_more is currently broken (https://jira.mongodb.org/browse/CDRIVER-516)
	 * so we have to workaround it
	 */
	if (tool_p -> mt_cursor_p)
		{
			bson_error_t error;
			mongoc_cursor_t *temp_p = mongoc_cursor_clone (tool_p -> mt_cursor_p);

			if (temp_p)
				{
					const bson_t *bson_p = NULL;
					results_flag = mongoc_cursor_next (temp_p, &bson_p);

					if (mongoc_cursor_error (temp_p, &error))
						{
							results_flag = false;
							PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, " mongo cursor error : %d.%d: %s\n", error.domain, error.code, error.message);
						}

					mongoc_cursor_destroy (temp_p);
				}
		}

	return results_flag;
}


int32 GetAllMongoResultsForKeyValuePair (MongoTool *tool_p, json_t **docs_pp, const char * const key_s, const char * const value_s, const char **fields_ss)
{
	int32 num_results = -1;
	bson_t *query_p = bson_new ();

	if (query_p)
		{
			if (BSON_APPEND_UTF8 (query_p, key_s, value_s))
				{
#if MONGODB_TOOL_DEBUG >= STM_LEVEL_FINE
					PrintBSONToLog (STM_LEVEL_FINE, __FILE__, __LINE__, query_p, "InsertOrUpdateMongoData query: ");
#endif

					num_results = 0;

					if (FindMatchingMongoDocumentsByBSON (tool_p, query_p, fields_ss, NULL))
						{
							*docs_pp = GetAllExistingMongoResultsAsJSON (tool_p);

							if (*docs_pp)
								{
									num_results = (int32) json_array_size (*docs_pp);
								}
						}
				}

			bson_destroy (query_p);
		}		/* if (query_p) */

	return num_results;
}



bool IterateOverMongoResults (MongoTool *tool_p, bool (*process_bson_fn) (const bson_t *document_p, void *data_p), void *data_p)
{
	bool success_flag = true;

	if (tool_p -> mt_cursor_p)
		{
			const bson_t *document_p = NULL;

			while (success_flag && (mongoc_cursor_next (tool_p -> mt_cursor_p, &document_p)))
				{
					success_flag = process_bson_fn (document_p, data_p);
				}

			if (!mongoc_cursor_more (tool_p -> mt_cursor_p))
				{
					mongoc_cursor_destroy (tool_p -> mt_cursor_p);
					tool_p -> mt_cursor_p = NULL;
				}

		}		/* if (tool_p -> mt_cursor_p) */
	else
		{
			success_flag = false;
		}

	return success_flag;
}


json_t *GetAllExistingMongoResultsAsJSON (MongoTool *tool_p)
{
	json_t *results_array_p = NULL;

	if (tool_p)
		{
			results_array_p = json_array ();

			if (results_array_p)
				{
					if (!IterateOverMongoResults (tool_p, AddBSONDocumentToJSONArray, results_array_p))
						{
						}

				}		/* if (results_array_p) */

		}		/* if (tool_p) */

	return results_array_p;

}


json_t *ConvertBSONValueToJSON (const bson_value_t *value_p)
{
	json_t *result_p = NULL;

	switch (value_p -> value_type)
	{
		case BSON_TYPE_DOUBLE:
			result_p = json_real (value_p -> value.v_double);
			break;

		case BSON_TYPE_UTF8:
			result_p = json_string (value_p -> value.v_utf8.str);
			break;

		case BSON_TYPE_BOOL:
			result_p = (value_p -> value.v_bool) ? json_true () : json_false ();
			break;

		case BSON_TYPE_INT32:
			result_p = json_integer (value_p -> value.v_int32);
			break;

		case BSON_TYPE_INT64:
			result_p = json_integer (value_p -> value.v_int64);
			break;

			/*
			BSON_TYPE_DOCUMENT   = 0x03,
			BSON_TYPE_ARRAY      = 0x04,
			BSON_TYPE_BINARY     = 0x05,
			BSON_TYPE_UNDEFINED  = 0x06,
			BSON_TYPE_OID        = 0x07,
			BSON_TYPE_BOOL       = 0x08,
			BSON_TYPE_DATE_TIME  = 0x09,
			BSON_TYPE_NULL       = 0x0A,
			BSON_TYPE_REGEX      = 0x0B,
			BSON_TYPE_DBPOINTER  = 0x0C,
			BSON_TYPE_CODE       = 0x0D,
			BSON_TYPE_SYMBOL     = 0x0E,
			BSON_TYPE_CODEWSCOPE = 0x0F,
			BSON_TYPE_TIMESTAMP  = 0x11,
			BSON_TYPE_MAXKEY     = 0x7F,
			BSON_TYPE_MINKEY     = 0xFF,
			 */
		default:
			break;
	}

	return result_p;
}


json_t *GetCurrentValuesAsJSON (MongoTool *tool_p, const char **fields_ss, const size_t num_fields)
{
	json_t *results_p = json_object ();

	if (results_p)
		{
			if (HasMongoQueryResults (tool_p))
				{
					const bson_t *doc_p;

					if (mongoc_cursor_next (tool_p -> mt_cursor_p, &doc_p))
						{
							bson_iter_t iter;

							if (bson_iter_init (&iter, doc_p))
								{
									if (fields_ss)
										{
											size_t num_fields_found = 0;

											qsort (fields_ss, num_fields, sizeof (const char *), CompareStrings);

											while ((bson_iter_next (&iter)) && (num_fields_found < num_fields))
												{
													const char *key_s = bson_iter_key (&iter);
													const char *found_key_s = (const char *) bsearch (key_s, *fields_ss, num_fields, sizeof (const char *), CompareStrings);

													if (found_key_s)
														{
															const bson_value_t *value_p = bson_iter_value (&iter);
															json_t *converted_value_p = ConvertBSONValueToJSON (value_p);

															if (converted_value_p)
																{
																	if (json_object_set_new (results_p, key_s, converted_value_p) != 0)
																		{

																		}
																}

															++ num_fields_found;
														}		/* if (bsearch (key_s, field_ss, num_fields - i, sizeof (const char *), CompareStrings)) */


												}		/* while ((bson_iter_next (&iter)) && (num_fields_found < num_fields)) */

										}		/* if (fields_ss) */
									else
										{
											while (bson_iter_next (&iter))
												{
													const char *key_s = bson_iter_key (&iter);

													const bson_value_t *value_p = bson_iter_value (&iter);
													json_t *converted_value_p = ConvertBSONValueToJSON (value_p);

													if (converted_value_p)
														{
															if (json_object_set_new (results_p, key_s, converted_value_p) != 0)
																{

																}
														}

												}		/* while (bson_iter_next (&iter)) */

										}

								}		/* if (bson_iter_init (&iter, doc_p)) */

						}		/* if (mongoc_cursor_next (tool_p -> mt_cursor_p, &doc_p)) */

				}		/* if (HasMongoQueryResults (tool_p)) */

			if (json_object_size (results_p) == 0)
				{
					json_decref (results_p);
					results_p = NULL;
				}

		}		/* if (results_p) */


	return results_p;
}



static int CompareStrings (const void *v0_p, const void *v1_p)
{
	const char *s0_p = (const void *) v0_p;
	const char *s1_p = (const void *) v1_p;

	return strcmp (s0_p, s1_p);
}


int32 IsKeyValuePairInCollection (MongoTool *tool_p, const char *database_s, const char *collection_s, const char *key_s, const char *value_s)
{
	int32 res =-1;

	if (SetMongoToolDatabaseAndCollection (tool_p, database_s, collection_s))
		{
			json_error_t error;
			json_t *json_p = json_pack_ex (&error, 0, "{s:s}", key_s, value_s);

			if (json_p)
				{
					if (FindMatchingMongoDocumentsByJSON (tool_p, json_p, NULL, NULL))
						{
							res = HasMongoQueryResults (tool_p) ? 1 : 0;
						}

					json_decref (json_p);
				}
		}

	return res;
}

int64 GetNumberOfMongoResults (MongoTool *tool_p, bson_t *query_p, bson_t *extra_opts_p)
{
	bson_error_t error;
	int64_t count = mongoc_collection_count_documents (tool_p -> mt_collection_p, query_p, extra_opts_p, NULL, NULL, &error);

	if (count == -1)
		{
			PrintBSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, query_p, "failed to count documents: %s", error.message);
		}

	return count;
}


bool PopulateJSONWithAllMongoResults (MongoTool *tool_p, bson_t *query_p, bson_t *extra_opts_p, json_t *results_array_p)
{
	bool success_flag = false;
	
	if (tool_p)
		{
			bool alloc_query_flag = false;

			if (!query_p)
				{
					query_p = bson_new ();

					if (query_p != NULL)
						{
							alloc_query_flag = true;
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate query");
						}
				}

			if (query_p)
				{
					if (FindMatchingMongoDocumentsByBSON (tool_p, query_p, NULL, extra_opts_p))
						{
							if (IterateOverMongoResults (tool_p, AddBSONDocumentToJSONArray, results_array_p))
								{
									success_flag = true;
								}
							else
								{
									PrintBSONToErrors (STM_LEVEL_FINER, __FILE__, __LINE__, query_p, "Failed to iterate over results");
								}

						}		/* if (FindMatchingMongoDocumentsByBSON (tool_p, query_p, NULL)) */
					else
						{
							#if MONGODB_TOOL_DEBUG >= STM_LEVEL_FINER
							PrintBSONToLog (STM_LEVEL_FINER, __FILE__, __LINE__, query_p, "No hits found");
							#endif
							
							success_flag = true;
						}

					if (alloc_query_flag)
						{
							bson_destroy (query_p);
						}

				}		/* if (query_p) */

		}		/* if (tool_p) */

	return success_flag;
}



json_t *GetAllMongoResultsAsJSON (MongoTool *tool_p, bson_t *query_p, bson_t *extra_opts_p)
{
	json_t *results_array_p = json_array ();

	if (results_array_p)
		{
			if (PopulateJSONWithAllMongoResults (tool_p, query_p, extra_opts_p, results_array_p))
				{
					return results_array_p;
				}

			json_decref (results_array_p);
		}		/* if (results_array_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate results array");
		}

	return NULL;
}


OperationStatus ProcessMongoResults (MongoTool *tool_p, bson_t *query_p, bson_t *extra_opts_p, bool (*process_bson_fn) (const bson_t *document_p, void *data_p), void *data_p)
{
	OperationStatus status = OS_FAILED_TO_START;
	bool alloc_query_flag = false;

	if (!query_p)
		{
			query_p = bson_new ();

			if (query_p != NULL)
				{
					alloc_query_flag = true;
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate query");
				}
		}

	if (query_p)
		{
			if (FindMatchingMongoDocumentsByBSON (tool_p, query_p, NULL, extra_opts_p))
				{
					if (tool_p -> mt_cursor_p)
						{
							bool success_flag = true;
							const bson_t *document_p = NULL;
							size_t num_docs = 0;
							size_t num_success = 0;

							while (success_flag && (mongoc_cursor_next (tool_p -> mt_cursor_p, &document_p)))
								{
									if (process_bson_fn (document_p, data_p))
										{
											++ num_success;
										}
									++ num_docs;
								}

							if (num_success == num_docs)
								{
									status = OS_SUCCEEDED;
								}
							else if (num_success > 0)
								{
									status = OS_PARTIALLY_SUCCEEDED;
								}
							else
								{
									status = OS_FAILED;
								}

							if (!mongoc_cursor_more (tool_p -> mt_cursor_p))
								{
									mongoc_cursor_destroy (tool_p -> mt_cursor_p);
									tool_p -> mt_cursor_p = NULL;
								}

						}		/* if (tool_p -> mt_cursor_p) */

				}		/* if (FindMatchingMongoDocumentsByBSON (tool_p, query_p, NULL)) */
			else
				{
					PrintBSONToLog (STM_LEVEL_FINER, __FILE__, __LINE__, query_p, "No hits found");
				}

			if (alloc_query_flag)
				{
					bson_destroy (query_p);
				}

		}		/* if (query_p) */

	return status;
}



bool AddBSONDocumentToJSONArray (const bson_t *document_p, void *data_p)
{
	bool success_flag = false;
	json_t *json_p = (json_t *) data_p;
	json_t *row_p = ConvertBSONToJSON (document_p);

	if (row_p)
		{
			if (json_array_append_new (json_p, row_p) == 0)
				{
					success_flag = true;
				}		/* if (json_array_append_new (json_p, row_p) == 0) */
			else
				{
					json_decref (row_p);
				}
		}		/* if (row_p) */

	return success_flag;

}


const char *EasyInsertOrUpdateMongoData (MongoTool *tool_p, json_t *values_p, const char *primary_key_id_s)
{
	const char ** const keys_ss = &primary_key_id_s;

	return InsertOrUpdateMongoData (tool_p, values_p, NULL, NULL, keys_ss, 1, NULL, NULL);
}


bool InsertMongoDataAsBSON (MongoTool *tool_p, const bson_t *doc_p, bson_t **reply_pp, bson_error_t *error_p)
{
	bool success_flag = false;
	bson_t *opts_p = NULL;
	bson_t reply;

	*reply_pp = NULL;

	success_flag = mongoc_collection_insert_one (tool_p -> mt_collection_p, doc_p, opts_p, &reply, error_p);

	if (success_flag)
		{
			bson_t *reply_p = bson_copy (&reply);

			if (reply_p)
				{
					*reply_pp = reply_p;
				}
			else
				{
					PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to copy reply");
				}

		}		/* if (success_flag) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to insert, error domain: " UINT32_FMT " code: " UINT32_FMT " message: \"%s\"", error_p -> domain, error_p -> code, error_p -> message);
		}

	return success_flag;
}



bool InsertMongoData (MongoTool *tool_p, const json_t *values_p, bson_t **reply_pp, bson_error_t *error_p)
{
	bool success_flag = false;
	bson_t *doc_p = ConvertJSONToBSON (values_p);

	if (doc_p)
		{
			success_flag = InsertMongoDataAsBSON (tool_p, doc_p, reply_pp, error_p);

			bson_destroy (doc_p);
		}		/* if (doc_p_ */

	return success_flag;
}


bool SetMongoDataAsBSONForGivenId (MongoTool *tool_p, bson_oid_t *id_p, bson_t *update_p, bson_t **reply_pp)
{
	bool success_flag = false;
	char buffer_s [MONGO_OID_STRING_BUFFER_SIZE];
	bson_t *selector_p = NULL;

	bson_oid_to_string (id_p, buffer_s);

	selector_p = BCON_NEW ("_id", "{", "$eq", BCON_OID (id_p), "}");

	if (selector_p)
		{
			success_flag = SetMongoDataAsBSON (tool_p, selector_p, update_p, reply_pp);

			bson_destroy (selector_p);
		}		/* if (selector_p) */

	return success_flag;
}


bool SetMongoData (MongoTool *tool_p, bson_t *selector_p, const json_t *values_p, bson_t **reply_pp)
{
	bool success_flag = false;
	bson_t *doc_p = ConvertJSONToBSON (values_p);

	if (doc_p)
		{
			success_flag = SetMongoDataAsBSON (tool_p, selector_p, doc_p, reply_pp);

			bson_destroy (doc_p);
		}		/* if (doc_p_ */

	return success_flag;
}


bool SetMongoDataAsBSON (MongoTool *tool_p, bson_t *selector_p, const bson_t *doc_p, bson_t **reply_pp)
{
	return UpdateMongoDataAsBSON (tool_p, "$set", selector_p, doc_p, reply_pp);
}



bool RemoveMongoFields (MongoTool *tool_p, bson_t *selector_p, const char **fields_ss, bson_t **reply_pp)
{
/*
	bson_t *update_p = BCON_NEW ("$unset",
															 "{",
															 ST_PHENOTYPES_S,
															 BCON_UTF8 (""),
															 "}");
*/
	bool success_flag = false;
	bson_t *doc_p = bson_new ();

	if (doc_p)
		{
			const char **field_ss = fields_ss;
			bool add_flag = true;

			while ((*field_ss) && add_flag)
				{
					if (BSON_APPEND_UTF8 (doc_p, *field_ss, ""))
						{
							++ field_ss;
						}
					else
						{
							PrintBSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, doc_p, "Failed to add field \"%s\"", *field_ss);
							add_flag = false;
						}
				}

			if (add_flag)
				{
					if (UpdateMongoDataAsBSON (tool_p, "$unset", selector_p, doc_p, reply_pp))
						{
							success_flag = true;
						}
					else
						{
							PrintBSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, selector_p, "UpdateMongoDataAsBSON () failed");
						}

				}

			bson_free (doc_p);
		}		/* if (doc_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create bson query");
		}

	return success_flag;
}


bool UpdateMongoDataAsBSON (MongoTool *tool_p, const char * const update_s, bson_t *selector_p, const bson_t *doc_p, bson_t **reply_pp)
{
	bool success_flag = false;
	bson_t *opts_p = NULL;
	bson_t reply;
	bson_error_t error;
	bson_t *update_p = NULL;

	*reply_pp = NULL;

	update_p = bson_new ();

	if (update_p)
		{
			if (BSON_APPEND_DOCUMENT (update_p, update_s, doc_p))
				{
#if MONGODB_TOOL_DEBUG >= STM_LEVEL_FINE
					{
						PrintBSONToLog (STM_LEVEL_FINE, __FILE__, __LINE__, update_p, "UpdateMongoDataAsBSON update_p: ");
						PrintBSONToLog (STM_LEVEL_FINE, __FILE__, __LINE__, selector_p, "UpdateMongoDataAsBSON selector_p: ");
					}
#endif


					success_flag = mongoc_collection_update_one (tool_p -> mt_collection_p, selector_p, update_p, opts_p, &reply, &error);

					if (success_flag)
						{
							bson_t *reply_p = bson_copy (&reply);

							if (reply_p)
								{
									*reply_pp = reply_p;
								}
							else
								{
									PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to copy reply");
								}

						}		/* if (success_flag) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to update db, error \"%s\"", error.message);
						}
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to append doc to update statement");
				}

			bson_destroy (update_p);
		}		/* if (update_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create update statement");
		}


	return success_flag;
}







bool RunMongoCommand (MongoTool *tool_p, bson_t *command_p, bson_t **reply_pp)
{
	bool success_flag = false;
	bson_t reply;
	bson_error_t error;

	success_flag = mongoc_collection_command_simple (tool_p -> mt_collection_p, command_p, NULL, &reply, &error);

	if (success_flag)
		{
			bson_t *reply_p = bson_copy (&reply);

			if (reply_p)
				{
					*reply_pp = reply_p;
				}
			else
				{
					PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to copy reply");
				}

		}		/* if (success_flag) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "command failed, error \"%s\"", error.message);
		}

	return success_flag;
}




const char *InsertOrUpdateMongoData (MongoTool *tool_p, json_t *values_p, const char * const database_s, const char * const collection_s, const char **primary_keys_ss, const size_t num_keys, const char * const mapped_id_s, const char * const object_key_s)
{
	const char *error_s = NULL;

	if (database_s && collection_s)
		{
			if (!SetMongoToolDatabaseAndCollection (tool_p, database_s, collection_s))
				{
					error_s = "Failed to set database and collection name";
				}
		}

	if (!error_s)
		{
			bson_t *query_p = MakeQuery (primary_keys_ss, num_keys, values_p);

			if (query_p)
				{
					const bool exists_flag = FindMatchingMongoDocumentsByBSON (tool_p, query_p, NULL, NULL);

#if MONGODB_TOOL_DEBUG >= STM_LEVEL_FINE
					PrintJSONToLog (STM_LEVEL_FINE, __FILE__, __LINE__, values_p, "FindMatchingMongoDocumentsByBSON returned %s", exists_flag ? "true" : "false");
#endif


					if (exists_flag)
						{
							if (object_key_s)
								{
									json_t *doc_p = json_object ();

									if (doc_p)
										{
											if (json_object_set (doc_p, object_key_s, values_p) == 0)
												{
													if (!UpdateMongoDocumentsByBSON (tool_p, query_p, doc_p, false))
														{
															error_s = "Failed to create update document";
														}
												}
											else
												{
													error_s = "Failed to update sub-document";
												}

											json_decref (doc_p);
										}
									else
										{
											error_s = "Failed to create sub-document for updating";
										}
								}
							else
								{
									if (!UpdateMongoDocumentsByBSON (tool_p, query_p, values_p, false))
										{
											error_s = "Failed to create update document";
										}
								}

						}		/* if (FindMatchingMongoDocumentsByBSON (tool_p, query_p, NULL)) */
					else
						{
							bson_oid_t *oid_p = NULL;

							if (object_key_s)
								{
									json_t *doc_p = json_object ();

									/* remove the primary_key_id_s field */
									const char **key_ss = primary_keys_ss;
									size_t i = 0;
									bool success_flag = true;

									for (i = 0; i < num_keys; ++ i, ++ key_ss)
										{
											json_t *value_p = json_object_get (values_p, *key_ss);

											if (value_p)
												{
													if (json_object_set (doc_p, *key_ss, value_p))
														{
															json_object_del (values_p, *key_ss);
														}
													else
														{
															success_flag = false;
															i = num_keys;
														}
												}

										}

									if (success_flag)
										{
											if (json_object_set (doc_p, object_key_s, values_p) == 0)
												{
													oid_p = InsertJSONIntoMongoCollection (tool_p, doc_p);

													if (doc_p)
														{
															json_decref (doc_p);
														}
												}
											else
												{
													error_s = "Failed to create sub-document to insert";
												}

										}
								}
							else
								{
									oid_p = InsertJSONIntoMongoCollection (tool_p, values_p);
								}


							if (oid_p)
								{

									FreeMemory (oid_p);
								}
							else
								{
									error_s = "Failed to insert data";
								}

						}

					bson_destroy (query_p);
				}		/* if (query_p) */

		}		/* if (SetMongoToolCollection (tool_p, database_s,collection_s)) */

	return error_s;
}


bool CreateIndexForMongoCollection (MongoTool *tool_p, char **fields_ss)
{
	bool success_flag = true;
	char **field_ss = fields_ss;
	bson_t keys;

	bson_init (&keys);

	/* get the list of fields */
	while (success_flag && *field_ss)
		{
			success_flag = BSON_APPEND_INT32 (&keys, *field_ss, 1);
			++ field_ss;
		}

	if (success_flag)
		{
			char *index_s = mongoc_collection_keys_to_index_string (&keys);

			if (index_s)
				{
					const char *collection_name_s = mongoc_collection_get_name (tool_p -> mt_collection_p);

					bson_t *command_p = BCON_NEW ("createIndexes",
																				BCON_UTF8 (collection_name_s),
																				"indexes",
																				"[",
																				"{",
																				"key",
																				BCON_DOCUMENT (&keys),
																				"name",
																				BCON_UTF8 (index_s),
																				"}",
																				"]");

					if (command_p)
						{
							bson_t reply;
							bson_error_t error;

							success_flag = mongoc_database_write_command_with_opts (tool_p -> mt_database_p, command_p, NULL /* opts */, &reply, &error);

#if MONGODB_TOOL_DEBUG >= STM_LEVEL_FINE
							{
								char *reply_s = bson_as_json (&reply, NULL);

								if (reply_s)
									{
										PrintLog (STM_LEVEL_FINE, __FILE__, __LINE__, "%s", reply_s);
										bson_free (reply_s);
									}
							}
#endif

							if (!success_flag)
								{
									PrintErrors (STM_LEVEL_FINE, __FILE__, __LINE__, "Error creating MongoDB index for %s: %s", collection_name_s, error.message);
								}

							bson_destroy (command_p);
						}		/* if (command_p) */

					bson_free (index_s);
				}		/* if (index_s) */

		}		/* if (success_flag) */

	return success_flag;
}



static bson_t *MakeQuery (const char **keys_ss, const size_t num_keys, const json_t *data_p)
{
	bson_t *query_p = bson_new ();

	if (query_p)
		{
			bool success_flag = true;
			size_t i = 0;

			while (success_flag  && (i < num_keys))
				{
					json_t *value_p = json_object_get (data_p, *keys_ss);

					if (value_p)
						{
							switch (value_p -> type)
							{
								case JSON_STRING:
									if (!BSON_APPEND_UTF8 (query_p, *keys_ss, json_string_value (value_p)))
										{
											success_flag = false;
										}
									break;

								case JSON_INTEGER:
									if (!BSON_APPEND_INT64 (query_p, *keys_ss, json_integer_value (value_p)))
										{
											success_flag = false;
										}
									break;

								case JSON_REAL:
									if (!BSON_APPEND_DOUBLE (query_p, *keys_ss, json_real_value (value_p)))
										{
											success_flag = false;
										}
									break;

								case JSON_TRUE:
								case JSON_FALSE:
									if (!BSON_APPEND_BOOL (query_p, *keys_ss, json_boolean_value (value_p)))
										{
											success_flag = false;
										}
									break;

								default:
									success_flag = false;
									break;
							}

						}		/* if (value_p) */

						if (success_flag)
							{
								++ keys_ss;
								++ i;
							}

				}		/* while (success_flag  && (i < num_keys)) */

			if (success_flag)
				{
					return query_p;
				}

			bson_destroy (query_p);
		}		/* if (query_p) */

	return NULL;
}


bool GetIdFromJSONKeyValuePair (const json_t *id_val_p, bson_oid_t *id_p)
{
	bool success_flag = false;
	const char *oid_s = GetJSONString (id_val_p, MONGO_OID_KEY_S);

	if (oid_s)
		{
			bson_oid_init_from_string (id_p, oid_s);
			success_flag = true;
		}

	return success_flag;
}


bool GetMongoIdFromJSON (const json_t *data_p, bson_oid_t *id_p)
{
	return GetNamedIdFromJSON (data_p, MONGO_ID_S, id_p);
}


bool GetNamedIdFromJSON (const json_t *data_p, const char * const key_s, bson_oid_t *id_p)
{
	bool success_flag = false;
	const json_t *id_val_p = json_object_get (data_p, key_s);

	if (id_val_p)
		{
			success_flag = GetIdFromJSONKeyValuePair (id_val_p, id_p);
		}

	return success_flag;
}


const char *GetNamedIdAsStringFromJSON (const json_t *data_p, const char * const key_s)
{
	const char *oid_s = NULL;
	const json_t *id_val_p = json_object_get (data_p, key_s);

	if (id_val_p)
		{
			oid_s = GetJSONString (id_val_p, MONGO_OID_KEY_S);
		}

	return oid_s;
}


bool AddIdToJSON (json_t *data_p, bson_oid_t *id_p, const char *key_s)
{
	bool success_flag = false;
	char *id_s = GetBSONOidAsString (id_p);

	if (id_s)
		{
			json_t *val_p = json_string (id_s);

			if (val_p)
				{
					if (json_object_set_new (data_p, key_s, val_p) == 0)
						{
							success_flag = true;
						}
					else
						{
							json_decref (val_p);
						}
				}

			FreeBSONOidString (id_s);
		}

	return success_flag;
}


bool AddCompoundIdToJSON (json_t *data_p, bson_oid_t *id_p)
{
	return AddNamedCompoundIdToJSON (data_p, id_p, MONGO_ID_S);
}

bool AddNamedCompoundIdToJSON (json_t *data_p, bson_oid_t *id_p, const char *key_s)
{
	bool success_flag = false;
	char *id_s = GetBSONOidAsString (id_p);

	if (id_s)
		{
			json_t *id_json_p = json_object ();

			if (id_json_p)
				{
					json_t *val_p = json_string (id_s);

					if (val_p)
						{
							if (json_object_set_new (id_json_p, MONGO_OID_KEY_S, val_p) == 0)
								{
									if (json_object_set_new (data_p, key_s, id_json_p) == 0)
										{
											success_flag = true;
										}
								}
							else
								{
									json_decref (val_p);
								}

						}		/* if (val_p) */

					if (!success_flag)
						{
							json_decref (id_json_p);
						}

				}		/* if (id_json_p) */

			FreeBSONOidString (id_s);
		}		/* if (id_s) */

	return success_flag;
}


bool AddCompoundIdToJSONArray (json_t *array_p, const bson_oid_t *id_p)
{
	bool success_flag = false;
	char *id_s = GetBSONOidAsString (id_p);

	if (id_s)
		{
			json_t *id_json_p = json_object ();

			if (id_json_p)
				{
					json_t *val_p = json_string (id_s);

					if (val_p)
						{
							if (json_object_set_new (id_json_p, MONGO_OID_KEY_S, val_p) == 0)
								{
									if (json_array_append_new (array_p, id_json_p) == 0)
										{
											success_flag = true;
										}
								}
							else
								{
									json_decref (val_p);
								}

						}		/* if (val_p) */

					if (!success_flag)
						{
							json_decref (id_json_p);
						}

				}		/* if (id_json_p) */

			FreeBSONOidString (id_s);
		}		/* if (id_s) */

	return success_flag;
}



char *GetBSONOidAsString (const bson_oid_t *id_p)
{
	char *id_s = NULL;

	if (id_p)
		{
			id_s = (char *) AllocMemory (MONGO_OID_STRING_BUFFER_SIZE * sizeof (char));

			if (id_s)
				{
					bson_oid_to_string (id_p, id_s);
				}
		}

	return id_s;
}



void FreeBSONOidString (char *id_s)
{
	FreeMemory (id_s);
}


bson_oid_t *GetBSONOidFromString (const char *id_s)
{
	bson_oid_t *id_p = NULL;

	if (id_s)
		{
			if (bson_oid_is_valid (id_s, strlen (id_s)))
				{
					id_p = (bson_oid_t *) AllocMemory (sizeof (bson_oid_t));

					if (id_p)
						{
							bson_oid_init_from_string (id_p, id_s);
						}
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "\"%s\" is not a valid bson_oid", id_s);
				}
		}

	return id_p;
}


bson_oid_t *GetNewBSONOid (void)
{
	bson_oid_t *id_p = AllocMemory (sizeof (bson_oid_t));

	if (id_p)
		{
			bson_oid_init (id_p, bson_context_get_default ());
		}

	return id_p;
}


bson_oid_t *GetNewUnitialisedBSONOid (void)
{
	bson_oid_t *id_p = AllocMemory (sizeof (bson_oid_t));

	if (id_p)
		{
			memset (id_p, 0, sizeof (bson_oid_t));
		}

	return id_p;
}


bson_oid_t *CopyBSONOid (const bson_oid_t *src_p)
{
	bson_oid_t *id_p = AllocMemory (sizeof (bson_oid_t));

	if (id_p)
		{
			bson_oid_copy (src_p, id_p);
		}

	return id_p;
}

void FreeBSONOid (bson_oid_t *id_p)
{
	FreeMemory (id_p);
}


bson_t *AllocateBSON (void)
{
	bson_t *bson_p = (bson_t *) AllocMemory (sizeof  (bson_t));

	if (bson_p)
		{
			bson_init (bson_p);
		}

	return bson_p;
}



void FreeBSON (bson_t *bson_p)
{
	bson_destroy (bson_p);
}


bool AddQueryTerm (bson_t *query_p, const char *key_s, const char *value_s, bool regex_flag)
{
	bool success_flag = true;

	if (!IsStringEmpty (value_s))
		{
			if (regex_flag)
				{
					success_flag = BSON_APPEND_REGEX (query_p, key_s, value_s, "i");
				}
			else
				{
					success_flag = BSON_APPEND_UTF8 (query_p, key_s, value_s);
				}
		}

	return success_flag;
}
