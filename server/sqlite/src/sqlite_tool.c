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
#include "grassroots_config.h"
#include "json_util.h"
#include "search_options.h"
#include "sql_clause.h"


static int ConvertSQLiteRowToJSON (void *data_p, int num_columns, char **values_ss, char **column_names_ss);


#ifdef _DEBUG
	#define SQLITE_TOOL_DEBUG	(STM_LEVEL_FINER)
#else
	#define SQLITE_TOOL_DEBUG	(STM_LEVEL_NONE)
#endif



SQLiteTool *AllocateSQLiteTool (const char *db_s, int flags, const char *table_s)
{
	SQLiteTool *tool_p = (SQLiteTool *) AllocMemory (sizeof (SQLiteTool));

	if (tool_p)
		{
			sqlite3 *db_p = NULL;
			int res;

			memset (tool_p, 0, sizeof (SQLiteTool));

			if (SetSQLiteDatabase (tool_p, db_s, flags, table_s))
				{
					return tool_p;
				}		/* if (SetSQLiteDatabase (tool_p, db_s, flags, table_s)) */

			FreeMemory (tool_p);
		}		/* if (tool_p) */

	return NULL;
}


bool SetSQLiteDatabase (SQLiteTool *tool_p, const char *db_s, int flags, const char *table_s)
{
	bool success_flag = false;
	sqlite3 *db_p = NULL;
	int res = sqlite3_open_v2 (db_s, &db_p, flags, NULL);

	if (res == SQLITE_OK)
		{
			tool_p -> sqlt_database_p = db_p;
			tool_p -> sqlt_table_s = table_s;

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



json_t *GetCurrentValuesAsJSON (SQLiteTool *tool_p, const char **fields_ss, const size_t num_fields)
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


const char *InsertOrUpdateSQLiteData (SQLiteTool *tool_p, json_t *values_p, const char * const database_s, const char * const primary_key_id_s, const char * const mapped_id_s, const char * const object_key_s)
{
	return NULL;
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
