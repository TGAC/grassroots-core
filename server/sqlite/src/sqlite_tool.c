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
 * sqlite_tool.c
 *
 *  Created on: 26 Nov 2017
 *      Author: billy
 */
#include <math.h>
#include <string.h>

#define ALLOCATE_SQLITE_TAGS (1)
#include "sqlite_tool.h"
#include "memory_allocations.h"
#include "streams.h"
#include "json_tools.h"
#include "grassroots_server.h"
#include "json_util.h"
#include "search_options.h"
#include "sql_clause.h"
#include "math_utils.h"
#include "string_utils.h"
#include "key_value_pair.h"
#include "sqlite_column.h"


static int ConvertSQLiteRowToJSON (void *data_p, int num_columns, char **values_ss, char **column_names_ss);

static bool AddValuesToByteBufferForUpsert (const char *primary_key_s, const char * table_s, const json_t *values_p, ByteBuffer *buffer_p);

static bool AddValuesToByteBufferForUpdate (const char *primary_key_s, const char * table_s, const json_t *set_p, const json_t *where_p, ByteBuffer *buffer_p);

static bool AddJSONObjectToByteBuffer (ByteBuffer *buffer_p, json_t *value_p);


static bool DoInsert (json_t *value_p, const char * const table_s, const char * const primary_key_s, ByteBuffer *buffer_p);


#ifdef _DEBUG
	#define SQLITE_TOOL_DEBUG	(STM_LEVEL_FINER)
#else
	#define SQLITE_TOOL_DEBUG	(STM_LEVEL_NONE)
#endif



SQLiteTool *AllocateSQLiteTool (const char *db_s, int flags)
{
	SQLiteTool *tool_p = (SQLiteTool *) AllocMemory (sizeof (SQLiteTool));

	if (tool_p)
		{
			memset (tool_p, 0, sizeof (SQLiteTool));

			if (SetSQLiteDatabase (tool_p, db_s, flags))
				{
					return tool_p;
				}		/* if (SetSQLiteDatabase (tool_p, db_s, flags)) */

			FreeSQLiteTool (tool_p);
		}		/* if (tool_p) */

	return NULL;
}



bool SetSQLiteToolTable (SQLiteTool *tool_p, const char *table_s)
{
	bool success_flag = false;

	if (table_s)
		{
			char *copied_table_s = EasyCopyToNewString (table_s);

			if (copied_table_s)
				{
					if (tool_p -> sqlt_table_s)
						{
							FreeCopiedString (tool_p -> sqlt_table_s);
						}

					tool_p -> sqlt_table_s = copied_table_s;
					success_flag = true;
				}
		}
	else
		{
			if (tool_p -> sqlt_table_s)
				{
					FreeCopiedString (tool_p -> sqlt_table_s);
					tool_p -> sqlt_table_s = NULL;
				}
		}

	return success_flag;
}


bool SetSQLiteDatabase (SQLiteTool *tool_p, const char *db_s, int flags)
{
	bool success_flag = false;
	int res = sqlite3_open_v2 (db_s, & (tool_p -> sqlt_database_p), flags, NULL);

	if (res == SQLITE_OK)
		{
			success_flag = true;
		}		/* if (res == SQLITE_OK) */

	return success_flag;
}


bool CloseSQLiteTool (SQLiteTool *tool_p)
{
	bool success_flag = true;

	if (tool_p -> sqlt_database_p)
		{
			int res = sqlite3_close_v2 (tool_p -> sqlt_database_p);

			if (res != SQLITE_OK)
				{
					success_flag = false;
				}
		}

	return success_flag;
}


void FreeSQLiteTool (SQLiteTool *tool_p)
{
	if (!CloseSQLiteTool (tool_p))
		{

		}

	FreeMemory (tool_p);
}



bool RemoveSQLiteRows (SQLiteTool *tool_p, const json_t *selector_json_p, const bool remove_first_match_only_flag)
{
	bool success_flag = false;


	return success_flag;
}


json_t *FindMatchingSQLiteDocumentsByJSON (SQLiteTool *tool_p, json_t *where_clauses_p, const char **fields_ss, char **error_ss)
{
	json_t *res_p = NULL;

	return res_p;
}


json_t *FindMatchingSQLiteDocuments (SQLiteTool *tool_p, LinkedList *where_clauses_p, const char **fields_ss, char **error_ss)
{
	bool success_flag = false;
	ByteBuffer *buffer_p = AllocateByteBuffer (1024);

	if (buffer_p)
		{
			if (AppendStringToByteBuffer (buffer_p, "SELECT "))
				{
					success_flag = true;

					if (fields_ss && *fields_ss)
						{
							int32 i = 0;

							while (success_flag && (*fields_ss != NULL))
								{
									if (i == 0)
										{
											success_flag = AppendStringToByteBuffer (buffer_p, *fields_ss);
										}
									else
										{
											success_flag = AppendStringsToByteBuffer (buffer_p, ", ", *fields_ss, NULL);
										}

									if (success_flag)
										{
											++ fields_ss;
											++ i;
										}
								}
						}
					else
						{
							success_flag = AppendStringToByteBuffer (buffer_p, " *");
						}

					if (success_flag)
						{
							if (AppendStringsToByteBuffer (buffer_p, " FROM ", tool_p -> sqlt_table_s, NULL))
								{
									success_flag = AddSQLClausesToByteBuffer (where_clauses_p, buffer_p);
								}
							else
								{
									success_flag = false;
								}

						}		/* if (success_flag) */

				}		/* if (AppendStringToByteBuffer (buffer_p, "SELECT ")) */

			if (success_flag)
				{
					json_t *results_p = json_array ();

					if (results_p)
						{
							char *error_s = NULL;
							const char *sql_s = GetByteBufferData (buffer_p);

							int res = sqlite3_exec (tool_p -> sqlt_database_p, sql_s, ConvertSQLiteRowToJSON, results_p, &error_s);

							if (res == SQLITE_OK)
								{
									return results_p;
								}

							if (error_s)
								{
									PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "FindMatchingSQLiteDocuments failed for query: \"%s\", error: \"%s\"", sql_s, error_s);
									*error_ss = error_s;
								}

							json_decref (results_p);
						}		/* if (results_p) */


				}		/* if (success_flag) */

			FreeByteBuffer (buffer_p);
		}		/* if (buffer_p) */

	return NULL;
}


void FreeSQLiteToolErrorString (SQLiteTool *tool_p, char *error_s)
{
	if (error_s)
		{
			sqlite3_free (error_s);
		}
}



json_t *GetCurrentValuesFromSQLiteToolAsJSON (SQLiteTool *tool_p, const char **fields_ss, const size_t num_fields)
{
	json_t *results_p = json_object ();

	if (results_p)
		{

			if (json_object_size (results_p) == 0)
				{
					json_decref (results_p);
					results_p = NULL;
				}

		}		/* if (results_p) */


	return results_p;
}


json_t *GetAllSQLiteResultsAsJSON (SQLiteTool *tool_p)
{
	json_t *results_array_p = NULL;

	if (tool_p)
		{
			results_array_p = json_array ();

			if (results_array_p)
				{

				}		/* if (results_array_p) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate results array");
				}

		}		/* if (tool_p) */

	return results_array_p;
}


char *InsertOrUpdateSQLiteData (SQLiteTool *tool_p, json_t *values_p, const char * const table_s, const char * const primary_key_s)
{
	/*
	 * Since we are using SQLite 3.24.0 or greater we have the ability
	 * to do an upsert (https://www.sqlite.org/lang_UPSERT.html) which
	 * takes care of doing the "insert or update" operation.
	 */
	char *error_s = NULL;
	ByteBuffer *buffer_p = AllocateByteBuffer (1024);

	if (buffer_p)
		{
			if (json_is_array (values_p))
				{
					const size_t size = json_array_size (values_p);
					size_t i = 0;

					for (i = 0; i < size; ++ i)
						{
							json_t *value_p = json_array_get (values_p, i);

							if (!DoInsert (value_p, table_s, primary_key_s, buffer_p))
								{
									error_s = EasyCopyToNewString ("Failed to update values");
									i = size;		/* force exit from loop */
								}
						}

				}		/* if (json_is_array (values_p)) */
			else if (json_is_object (values_p))
				{
					if (!DoInsert (values_p, table_s, primary_key_s, buffer_p))
						{
							error_s = EasyCopyToNewString ("Failed to update values");
						}

				}		/* else if (json_is_object (set_p)) */
			else
				{
					error_s = EasyCopyToNewString ("set_p is not a JSON object or array");
				}

			if (!error_s)
				{
					const char *sql_s = GetByteBufferData (buffer_p);

					error_s = EasyRunSQLiteToolStatement (tool_p, sql_s);

					if (error_s)
						{
							int extended_code = sqlite3_extended_errcode (tool_p -> sqlt_database_p);

							/*
							 * If we are doing an update, then
							 */
							if (extended_code == SQLITE_CONSTRAINT_NOTNULL)
								{

								}
						}
				}

			FreeByteBuffer (buffer_p);
		}		/* if (buffer_p) */

	return error_s;
}


char *CreateSQLiteTable (SQLiteTool *tool_p, const char *table_s, LinkedList *columns_p, const bool delete_if_exists_flag)
{
	char *error_s = NULL;
	ByteBuffer *buffer_p = AllocateByteBuffer (1024);

	if (buffer_p)
		{
			bool success_flag = true;

			if (delete_if_exists_flag)
				{
					success_flag = AppendStringsToByteBuffer (buffer_p, "DROP TABLE IF EXISTS ", table_s, ";", NULL);
				}

			if (success_flag)
				{
					if (AppendStringsToByteBuffer (buffer_p, "CREATE TABLE ", table_s, "(", NULL))
						{
							const SQLiteColumnNode * const last_node_p = (const SQLiteColumnNode * const) (columns_p -> ll_tail_p);
							SQLiteColumnNode *node_p = (SQLiteColumnNode *) (columns_p -> ll_head_p);

							while (node_p)
								{
									SQLiteColumn *column_p = node_p -> sqlcn_column_p;
									char *column_s = GetSQLiteColumnAsString (column_p);

									if (column_s)
										{
											const char *sep_s;

											if (node_p != last_node_p)
												{
													sep_s = ",";
												}
											else
												{
													sep_s = ");";
												}

											success_flag = AppendStringsToByteBuffer (buffer_p, column_s, sep_s, NULL);
										}
									else
										{
											success_flag = false;
										}

									if (success_flag)
										{
											node_p = (SQLiteColumnNode *) (node_p -> sqlcn_node.ln_next_p);
										}
									else
										{
											node_p = NULL;
										}

								}		/* while (node_p) */

							if (success_flag)
								{
									const char *sql_s = GetByteBufferData (buffer_p);

									error_s = EasyRunSQLiteToolStatement (tool_p, sql_s);
								}

						}		/* if (AppendStringsToByteBuffer (buffer_p, "CREATE TABLE ", table_s, "(")) */

				}		/* if (success_flag) */


			FreeByteBuffer (buffer_p);
		}		/* if (buffer_p) */

	return error_s;
}


char *RunSQLiteToolStatement (SQLiteTool *tool_p, const char *sql_s, int (*callback_fn) (void *data_p, int num_columns, char **columns_aa_text_ss, char **column_names_ss), void *data_p)
{
	char *error_s = NULL;
	char *sql_error_s = NULL;
	int res = sqlite3_exec (tool_p -> sqlt_database_p, sql_s, callback_fn, data_p, &sql_error_s);

	if (res != SQLITE_OK)
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "SQLite exec returned %d for \"%s\": error \"%s\"", res, sql_s, sql_error_s);
			error_s = EasyCopyToNewString (sql_error_s);
			sqlite3_free (sql_error_s);
		}

	return error_s;
}


bool PrepareStatement (SQLiteTool *tool_p, sqlite3_stmt **statement_pp, const char *sql_s)
{
	bool success_flag = false;
	size_t sql_len = strlen (sql_s);
	int res = sqlite3_prepare_v2 (tool_p -> sqlt_database_p, sql_s , sql_len + 1, statement_pp, NULL);

	if (res == 0)
		{
			success_flag = true;
		}

	return success_flag;
}



char *EasyRunSQLiteToolStatement (SQLiteTool *tool_p, const char *sql_s)
{
	return RunSQLiteToolStatement (tool_p, sql_s, NULL, NULL);
}



static bool AddValuesToByteBufferForUpdate (const char *primary_key_s, const char * table_s, const json_t *set_p, const json_t *where_p, ByteBuffer *buffer_p)
{
	bool success_flag = true;
	const size_t num_items = json_object_size (set_p);

	if (num_items > 0)
		{
			ByteBuffer *update_buffer_p = AllocateByteBuffer (1024);


			if (update_buffer_p)
				{
					if (AppendStringsToByteBuffer (update_buffer_p, "UPDATE ", table_s, " SET ", NULL))
						{
							size_t i = num_items - 1;
							size_t num_added = 0;

							void *iterator_p = json_object_iter (set_p);

							while (success_flag && iterator_p)
								{
									const char *key_s = json_object_iter_key (iterator_p);
									json_t *value_p = json_object_iter_value (iterator_p);

									if (AppendStringsToByteBuffer (buffer_p, key_s, " = ", NULL))
										{
											if (AddJSONObjectToByteBuffer (buffer_p, value_p))
												{
													if (i != 0)
														{
															if (!AppendStringToByteBuffer (buffer_p, ", "))
																{
																	success_flag = false;
																}
														}		/* if (i != 0) */
													else
														{
															++ num_added;
														}

												}		/* if (AddJSONObjectToByteBuffer (buffer_p, value_p)) */
											else
												{
													success_flag = false;

												}
										}		/* if (AppendStringsToByteBuffer (buffer_p, key_s, " = ", NULL)) */
									else
										{
											success_flag = false;
										}

									if (success_flag)
										{
											iterator_p = json_object_iter_next (set_p, iterator_p);
											++ i;
										}

								}		/* while (iterator_p) */


							/*
							 * Have we added all of the SET values ok?
							 */
							if (success_flag)
								{
									/*
									 * Now add the WHERE values
									 */
									i = json_object_size (where_p) - 1;

									if (i > 0)
										{
											void *iterator_p = json_object_iter (where_p);

											-- i;

											while (iterator_p)
												{
													const char *key_s = json_object_iter_key (iterator_p);
													json_t *value_p = json_object_iter_value (iterator_p);


													if (*key_s == '$')
														{

														}

													success_flag = false;

													if (AppendStringsToByteBuffer (buffer_p, key_s, " = ", NULL))
														{
															if (AddJSONObjectToByteBuffer (buffer_p, value_p))
																{
																	if (i != 0)
																		{
																			if (AppendStringToByteBuffer (buffer_p, ", "))
																				{
																					success_flag = true;
																				}
																		}		/* if (i != 0) */
																	else
																		{
																			success_flag = true;
																		}

																}		/* if (AddJSONObjectToByteBuffer (buffer_p, value_p)) */

														}		/* if (AppendStringsToByteBuffer (buffer_p, key_s, " = ", NULL)) */

													if (success_flag && iterator_p)
														{
															iterator_p = json_object_iter_next (set_p, iterator_p);
															++ i;
														}

												}		/* while (iterator_p) */

										}		/* if (i >= 0) */

								}		/* if (success_flag) */

						}		/* if (AppendStringsToByteBuffer (update_buffer_p, "UPDATE ", table_s, NULL)) */

					FreeByteBuffer (update_buffer_p);
				}		/* if (update_buffer_p) */

		}

	return success_flag;
}


static bool AddJSONObjectToByteBuffer (ByteBuffer *buffer_p, json_t *value_p)
{
	char *value_s = NULL;
	bool success_flag = false;
	bool alloc_flag = false;

	if (json_is_string (value_p))
		{
			value_s = (char *) json_string_value (value_p);
		}
	else if (json_is_integer (value_p))
		{
			int i = json_integer_value (value_p);
			value_s = ConvertIntegerToString (i);

			if (value_s)
				{
					alloc_flag = true;
				}
		}
	else if (json_is_real (value_p))
		{
			double d = json_real_value (value_p);
			value_s = ConvertDoubleToString (d);

			if (value_s)
				{
					alloc_flag = true;
				}
		}

	if (value_s)
		{
			if (AppendStringsToByteBuffer (buffer_p, "'", value_s, "'", NULL))
				{
					success_flag = true;
				}

			if (alloc_flag)
				{
					FreeCopiedString (value_s);
				}

		}		/* if (value_s) */


	return success_flag;
}


static bool AddValuesToByteBufferForUpsert (const char *primary_key_s, const char * table_s, const json_t *set_values_p, ByteBuffer *buffer_p)
{
	bool success_flag = false;

	ByteBuffer *keys_buffer_p = AllocateByteBuffer (1024);

	if (keys_buffer_p)
		{
			ByteBuffer *values_buffer_p = AllocateByteBuffer (1024);

			if (values_buffer_p)
				{
					ByteBuffer *conflict_values_buffer_p = AllocateByteBuffer (1024);

					if (conflict_values_buffer_p)
						{
							size_t index = 0;
							const char *key_s;
							json_t *value_p;

							json_object_foreach (set_values_p, key_s, value_p)
								{
									/*
									 * For each key-value pair, set the success flag to false
									 * initially.
									 */
									success_flag = false;

									/*
									 * If we're not on the first key-value pair, then we
									 * need to add the commas.
									 */
									if (index != 0)
										{
											if (AppendStringToByteBuffer (keys_buffer_p, ","))
												{
													if (AppendStringToByteBuffer (values_buffer_p, ","))
														{
															if (AppendStringToByteBuffer (conflict_values_buffer_p, ","))
																{
																	success_flag = true;
																}

														} /* if (AppendStringToByteBuffer (values_buffer_p, ",")) */

												}		/* if (AppendStringToByteBuffer (keys_buffer_p, ",")) */
										}
									else
										{
											success_flag = true;
										}


									if (success_flag)
										{
											success_flag = false;

											if (AppendStringToByteBuffer (keys_buffer_p, key_s))
												{
													if (AppendStringsToByteBuffer (conflict_values_buffer_p, "    ", key_s, "=excluded.", key_s, NULL))
														{
															if (AddJSONObjectToByteBuffer (values_buffer_p, value_p))
																{
																	success_flag = true;
																}
														}		/* if (AppendStringsToByteBuffer (conflict_values_buffer_p, "    ", key_s, "=excluded.", key_s, NULL)) */

												}		/* if (AppendStringToByteBuffer (keys_buffer_p, key_s)) */

										}		/* if (success_flag) */


									if (success_flag)
										{
											++ index;
										}
									else
										{
											FreeByteBuffer (keys_buffer_p);
											FreeByteBuffer (values_buffer_p);
											FreeByteBuffer (conflict_values_buffer_p);

											return false;
										}

								}		/* json_object_foreach (values_p, key_s, value_p) */

							/*
							 * We have now filled the keys and values in, so now we can
							 * create the sql statements as per https://www.sqlite.org/lang_UPSERT.html
							 * such as
							 *
							 * CREATE TABLE phonebook(name TEXT PRIMARY KEY, phonenumber TEXT);
							 * INSERT INTO phonebook(name,phonenumber) VALUES('Alice','704-555-1212')
							 * ON CONFLICT(name) DO UPDATE SET phonenumber=excluded.phonenumber;
							 */
							success_flag = false;

							if (AppendStringsToByteBuffer (buffer_p, "INSERT INTO ", table_s, "(", NULL))
								{
									const char *data_s = GetByteBufferData (keys_buffer_p);

									if (AppendStringsToByteBuffer (buffer_p, data_s, ") ", NULL))
										{
											data_s = GetByteBufferData (values_buffer_p);

											if (AppendStringsToByteBuffer (buffer_p, "VALUES(", data_s, ")" , NULL))
												{
													data_s = GetByteBufferData (conflict_values_buffer_p);

													if (AppendStringsToByteBuffer (buffer_p, "  ON CONFLICT (", primary_key_s, ") DO UPDATE SET\n", data_s, ";", NULL))
														{
															/*
															 * We now have our full sql statement
															 */
															success_flag = true;

															#if SQLITE_TOOL_DEBUG >= STM_LEVEL_FINE
															PrintJSONToLog (STM_LEVEL_FINE, __FILE__, __LINE__, set_values_p, "For table \"%s\" and primary key \"%s\", UPSERT statement is \"%s\"", table_s, primary_key_s, GetByteBufferData (buffer_p));
															#endif
														}		/* if (AppendStringsToByteBuffer (buffer_p, "  ON CONFLICT (", primary_key_s, ") DO UPDATE SET\n" , NULL)) */

												}		/* if (AppendStringsToByteBuffer (buffer_p, "VALUES(", values_data_s, ") ON CONFLICT(" , NULL)) */

										}		/* if (AppendStringsToByteBuffer (buffer_p, keys_data_s, ") ", NULL)) */

								}		/* if (AppendStringsToByteBuffer (buffer_p, "INSERT INTO ", table_s, "(", NULL)) */

							FreeByteBuffer (conflict_values_buffer_p);
						}		/* if (conflict_values_buffer_p) */

					FreeByteBuffer (values_buffer_p);
				}		/* if (values_buffer_p) */

			FreeByteBuffer (keys_buffer_p);
		}		/* if (keys_buffer_p) */

	return success_flag;
}


static bool DoInsert (json_t *value_p, const char * const table_s, const char * const primary_key_s, ByteBuffer *buffer_p)
{
	bool success_flag = false;
	json_t *set_p = json_object_get (value_p, SQLITE_SET_CLAUSE_S);

	if (set_p)
		{
			json_t *where_p = json_object_get (value_p, SQLITE_WHERE_CLAUSE_S);

			if (where_p)
				{
					/*
					 * Since we have a where clause, this is an update.
					 */
					if (AddValuesToByteBufferForUpdate (primary_key_s, table_s, set_p, where_p, buffer_p))
						{
							success_flag = true;
						}
				}
			else
				{
					/*
					 * This could still be an update on the primary key or
					 * an insert, so cater for both with an upsert.
					 */
					if (AddValuesToByteBufferForUpsert (primary_key_s, table_s, set_p, buffer_p))
						{
							success_flag = true;
						}
				}
		}

	return success_flag;
}


bool InsertSQLiteRow (SQLiteTool *tool_p, const json_t *data_p, char **error_ss)
{
	bool success_flag = false;
	ByteBuffer *buffer_p = AllocateByteBuffer (1024);

	if (buffer_p)
		{
			ByteBuffer *columns_buffer_p = AllocateByteBuffer (1024);

			if (columns_buffer_p)
				{
					ByteBuffer *values_buffer_p = AllocateByteBuffer (1024);

					if (values_buffer_p)
						{
							void *iter_p = json_object_iter (data_p);
							const size_t size = json_object_size (data_p);
							size_t i = 1;

							success_flag = true;

							while (iter_p && success_flag)
								{
									const char *key_s = json_object_iter_key (iter_p);
									json_t *value_p = json_object_iter_value (iter_p);

									if (AppendStringToByteBuffer (columns_buffer_p, key_s))
										{
											if (AddJSONObjectToByteBuffer (values_buffer_p, value_p))
												{
													if (i != size)
														{
															success_flag = ((AppendStringToByteBuffer (columns_buffer_p, ", ")) && (AppendStringToByteBuffer (values_buffer_p, ", ")));
														}
												}
											else
												{
													success_flag = false;
												}
										}
									else
										{
											success_flag = false;
										}

									if (success_flag)
										{
											iter_p = json_object_iter_next (data_p, iter_p);
											++ i;
										}
									else
										{

										}

								}		/* while (iter_p && success_flag) */

							if (success_flag)
								{
									const char *columns_s = GetByteBufferData (columns_buffer_p);
									const char *values_s = GetByteBufferData (values_buffer_p);

									if (AppendStringsToByteBuffer (buffer_p, "INSERT INTO ", tool_p -> sqlt_table_s, " (", columns_s, ") VALUES (", values_s, ");", NULL))
										{
											const char *sql_s = GetByteBufferData (buffer_p);

											*error_ss = EasyRunSQLiteToolStatement (tool_p, sql_s);

											if (*error_ss)
												{
													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to run sql statement \"%s\", error: \"%s\"", sql_s, *error_ss);
												}
										}
								}
							else
								{

								}
							FreeByteBuffer (values_buffer_p);
						}		/* if (values_buffer_p) */

					FreeByteBuffer (columns_buffer_p);
				}		/* if (columns_buffer_p) */

			FreeByteBuffer (buffer_p);
		}		/* if (buffer_p) */
	else
		{

		}

	return success_flag;
}


static int ConvertSQLiteRowToJSON (void *data_p, int num_columns, char **values_ss, char **column_names_ss)
{
	int res = SQLITE_OK;
	json_t *results_p = (json_t *) data_p;
	json_t *row_p = json_object ();

	if (row_p)
		{
			bool success_flag = true;
			int i = 0;

			while ((i < num_columns) && (success_flag))
				{
					if (*values_ss)
						{
							if (json_object_set_new (row_p, *column_names_ss, json_string (*values_ss)) != 0)
								{
									res = SQLITE_NOMEM;
									success_flag = false;
								}
						}

					++ i;
					++ values_ss;
					++ column_names_ss;
				}

			if (success_flag)
				{
					if (json_array_append_new (results_p, row_p) != 0)
						{
							res = SQLITE_NOMEM;
						}
				}

		}		/* if (row_p) */
	else
		{
			res = SQLITE_NOMEM;
		}

	return res;
}
