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
 * sqllite_tool_test.c
 *
 *  Created on: 16 Aug 2018
 *      Author: billy
 */

#include <stdio.h>
#include <string.h>

#include "sqlite_tool.h"
#include "sqlite_column.h"
#include "string_utils.h"


static const char * const S_ID_COLUMN_S = "id";
static const char * const S_SURNAME_COLUMN_S = "surname";
static const char * const S_FORENAME_COLUMN_S = "forename";
static const char * const S_HEIGHT_COLUMN_S = "height";
static const char * const S_DESCRIPTION_COLUMN_S = "description";




static LinkedList *GetTableColumns (void);

static bool AddColumn (LinkedList *columns_p, const char *name_s, uint32 datatype, bool primary_key_flag, bool unique_flag, bool can_be_null_flag, const char *check_s);


static bool AddData (json_t *values_array_p, const int32 *id_p, const char *surname_s, const char *forename_s, const double64 *height_p, const char *description_s, json_t *where_values_p);

static json_t *GetDataToInsert (void);

static json_t *GetDataToUpdate (void);




int main (int argc, char *argv [])
{
	if (argc > 2)
		{
			const char *db_s = argv [1];
			const char *table_s = argv [2];
			SQLiteTool *tool_p = AllocateSQLiteTool (db_s, SQLITE_OPEN_READONLY);

			if (tool_p)
				{
					if (SetSQLiteToolTable (tool_p, table_s))
						{
							sqlite3_stmt *statement_p = NULL;
							const char *sql_s = "SELECT cluster FROM cluster_gene WHERE gene = ?";

							if (PrepareStatement (tool_p, &statement_p, sql_s))
								{
									int res;
									int i = 3;
									const int param_index = 1;

									printf ("prepared \"%s\"\n", sql_s);

									printf ("argc %d\n", argc);

									while (i < argc)
										{
											const char *param_s = argv [i];
											const int param_length = strlen (param_s);

											printf ("Binding \"%s\" (%d)\n", param_s, param_length);

											res = sqlite3_bind_text (statement_p, param_index, param_s, param_length, SQLITE_STATIC);

											if (res == SQLITE_OK)
												{
													while ((res = sqlite3_step (statement_p)) == SQLITE_ROW)
														{
															const int res_column = 0;
															const char *res_s = (const char *) sqlite3_column_text (statement_p, res_column);

															printf ("Searching for \"%s\" gave \"%s\"\n", param_s, res_s);

														}
												}
											else
												{
													printf ("Failed to bind \"%s\": \"%s\"\n", param_s, sqlite3_errstr (res));
												}

											res = sqlite3_reset (statement_p);

											if (res == SQLITE_OK)
												{
													++ i;
												}
											else
												{
													printf ("Failed to reset \"%s\": \"%s\"\n", sql_s, sqlite3_errstr (res));
												}
										}

									res = sqlite3_finalize (statement_p);

									if (res != SQLITE_OK)
										{

										}

								}		/* if (PrepareStatement (tool_p, &statement_p, sql_s)) */
							else
								{
									printf ("Failed to prepare statement \"%s\"\n", sql_s);
								}


						}		/* if (SetSqliteToolTable (tool_p, table_s)) */
					else
						{
							printf ("Failed to set SQLiteTool table to %s", table_s);
						}

					FreeSQLiteTool (tool_p);
				}		/* if (tool_p) */
			else
				{
					puts ("Failed to allocate SQLiteTool");
				}

			puts ("exiting");

		}		/* if (argc > 1) */
	else
		{
			puts ("usage: sqlite_test_tool <test_database_filename>");
		}

	return 0;
}


int main1 (int argc, char *argv [])
{
	if (argc > 1)
		{
			const char *table_s = "test_table";
			SQLiteTool *tool_p = AllocateSQLiteTool (argv [1], SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);

			if (tool_p)
				{
					if (SetSQLiteToolTable (tool_p, table_s))
						{
							LinkedList *columns_p = NULL;

							puts ("Allocated SQLiteTool");

							columns_p = GetTableColumns ();

							if (columns_p)
								{
									char *error_s = NULL;

									puts ("Got Table Columns");

									error_s = CreateSQLiteTable (tool_p, table_s, columns_p, true);

									FreeLinkedList (columns_p);

									if (error_s)
										{
											printf ("error: \"%s\"\n", error_s);
											FreeCopiedString (error_s);
										}
									else
										{
											json_t *values_p = GetDataToInsert ();

											if (values_p)
												{
													error_s = InsertOrUpdateSQLiteData (tool_p, values_p, table_s, S_ID_COLUMN_S);

													json_decref (values_p);

													if (error_s)
														{
															printf ("InsertOrUpdateSQLiteData error: \"%s\"\n", error_s);
															FreeCopiedString (error_s);
														}
													else
														{
															values_p = GetDataToUpdate ();

															if (values_p)
																{
																	error_s = InsertOrUpdateSQLiteData (tool_p, values_p, table_s, S_ID_COLUMN_S);

																	json_decref (values_p);

																	if (error_s)
																		{
																			printf ("InsertOrUpdateSQLiteData error: \"%s\"\n", error_s);
																			FreeCopiedString (error_s);
																		}
																}
															else
																{
																	puts ("failed to get data to update");
																}
														}
												}
											else
												{
													puts ("failed to get data to insert");
												}
										}

								}		/* if (columns_p) */
							else
								{
									puts ("Failed to get Table Columns");
								}

						}		/* if (SetSqliteToolTable (tool_p, table_s)) */
					else
						{
							printf ("Failed to set SQLiteTool table to %s", table_s);
						}

					FreeSQLiteTool (tool_p);
				}		/* if (tool_p) */
			else
				{
					puts ("Failed to allocate SQLiteTool");
				}

			puts ("exiting");

		}		/* if (argc > 1) */
	else
		{
			puts ("usage: sqlite_test_tool <test_database_filename>");
		}

	return 0;
}



static LinkedList *GetTableColumns (void)
{
	LinkedList *columns_p = AllocateLinkedList (FreeSQLiteColumnNode);

	if (columns_p)
		{
			if (AddColumn (columns_p, S_ID_COLUMN_S, SQLITE_INTEGER, true, false, false, NULL))
				{
					if (AddColumn (columns_p, S_SURNAME_COLUMN_S, SQLITE_TEXT, false, false, true, NULL))
						{
							if (AddColumn (columns_p, S_FORENAME_COLUMN_S, SQLITE_TEXT, false, false, true, NULL))
								{
									if (AddColumn (columns_p, S_HEIGHT_COLUMN_S, SQLITE_FLOAT, false, false, false, NULL))
										{
											if (AddColumn (columns_p, S_DESCRIPTION_COLUMN_S, SQLITE_TEXT, false, false, true, NULL))
												{
													return columns_p;
												}
											else
												{
													puts ("Failed to get description column");
												}
										}
									else
										{
											puts ("Failed to get height column");
										}
								}
							else
								{
									puts ("Failed to get forename column");
								}
						}
					else
						{
							puts ("Failed to get surname column");
						}
				}
			else
				{
					puts ("Failed to get id column");
				}

			FreeLinkedList (columns_p);
		}
	else
		{
			puts ("Failed to allocate columns list");
		}

	return NULL;
}


static bool AddColumn (LinkedList *columns_p, const char *name_s, uint32 datatype, bool primary_key_flag, bool unique_flag, bool can_be_null_flag, const char *check_s)
{
	bool success_flag = false;
	SQLiteColumnNode *node_p = AllocateSQLiteColumnNode (name_s, datatype, primary_key_flag, unique_flag, can_be_null_flag, check_s);

	if (node_p)
		{
			LinkedListAddTail (columns_p, & (node_p -> sqlcn_node));
			success_flag = true;
		}

	return success_flag;
}


static json_t *GetDataToInsert (void)
{
	json_t *values_p = json_array ();

	if (values_p)
		{
			double64 height = 1.80;

			if (AddData (values_p, NULL, "Fish", "Billy", &height, NULL, NULL))
				{
					height = 2.860;

					if (AddData (values_p, NULL, "Donkey", "Wonky", &height, "Oh to be a horse", NULL))
						{
							height = 1.230;

							if (AddData (values_p, NULL, "Bag", "Bean", &height, "Woof!", NULL))
								{
									return values_p;
								}
						}
				}

			json_decref (values_p);
		}

	return NULL;
}




static bool AddData (json_t *values_array_p, const int32 *id_p, const char *surname_s, const char *forename_s, const double64 *height_p, const char *description_s, json_t *where_values_p)
{
	json_t *entry_p = json_object ();

	if (entry_p)
		{
			json_t *set_p = json_object ();

			if (set_p)
				{
					if (json_object_set_new (entry_p, SQLITE_SET_CLAUSE_S, set_p) == 0)
						{
							if ((id_p == NULL) || (json_object_set_new (set_p, S_ID_COLUMN_S, json_integer (*id_p)) == 0))
								{
									if ((surname_s == NULL) || (json_object_set_new (set_p, S_SURNAME_COLUMN_S, json_string (surname_s)) == 0))
										{
											if ((forename_s == NULL) || (json_object_set_new (set_p, S_FORENAME_COLUMN_S, json_string (forename_s)) == 0))
												{
													if ((height_p == NULL) || (json_object_set_new (set_p, S_HEIGHT_COLUMN_S, json_real (*height_p)) == 0))
														{
															if ((description_s == NULL) || (json_object_set_new (set_p, S_DESCRIPTION_COLUMN_S, json_string (description_s)) == 0))
																{
																	if (json_array_append_new (values_array_p, entry_p) == 0)
																		{
																			if ((where_values_p == NULL) || (json_object_set_new (entry_p, SQLITE_WHERE_CLAUSE_S, where_values_p) == 0))
																				{
																					return true;
																				}
																		}
																}
														}
												}
										}
								}

							json_decref (entry_p);
						}
					else
						{
							json_decref (set_p);
						}
				}
		}

	return false;
}



static json_t *GetDataToUpdate (void)
{
	json_t *values_p = json_array ();

	if (values_p)
		{
			int32 id = 3;
			double64 height = 0.890;

			if (AddData (values_p, &id, "Bag", "Bean", &height, "Woofedy Woof!", NULL))
				{
					json_t *where_clauses_p = json_object ();

					if (where_clauses_p)
						{
							if (json_object_set_new (where_clauses_p, S_SURNAME_COLUMN_S, json_string ("Donkey")) == 0)
								{
									if (AddData (values_p, NULL, "Badger", NULL, NULL, NULL, where_clauses_p))
										{
											return values_p;
										}
								}
							else
								{
									json_decref (where_clauses_p);
								}
						}

				}

			json_decref (values_p);
		}

	return NULL;

}

