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

#include "sqlite_tool.h"
#include "memory_allocations.h"
#include "streams.h"
#include "json_tools.h"
#include "grassroots_config.h"
#include "json_util.h"
#include "search_options.h"


static int CompareStrings (const void *v0_p, const void *v1_p);



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
			sqlite3 *db_p = NULL;
			int res;

			memset (tool_p, 0, sizeof (SQLiteTool));

			res = sqlite3_open_v2 (db_s, &db_p, flags, NULL);

			if (res == SQLITE_OK)
				{
					tool_p -> sqlt_database_p = db_p;

					return tool_p;
				}		/* if (res == SQLITE_OK) */

			FreeMemory (tool_p);
		}		/* if (tool_p) */

	return NULL;
}


bool SetSQLiteDatabase (SQLiteTool *tool_p, const char *db_s, int flags)
{
	bool success_flag = false;
	sqlite3 *db_p = NULL;
	int res = sqlite3_open_v2 (db_s, &db_p, flags, NULL);

	if (res == SQLITE_OK)
		{
			tool_p -> sqlt_database_p = db_p;
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



static int CompareStrings (const void *v0_p, const void *v1_p)
{
	const char *s0_p = (const void *) v0_p;
	const char *s1_p = (const void *) v1_p;

	return strcmp (s0_p, s1_p);
}


int32 IsKeyValuePairInDatabase (SQLiteTool *tool_p, const char *database_s, const char *key_s, const char *value_s)
{
	int32 res =-1;

	if (SetSQLiteToolCollection (tool_p, database_s))
		{
			json_error_t error;
			json_t *json_p = json_pack_ex (&error, 0, "{s:s}", key_s, value_s);

			if (json_p)
				{
					if (FindMatchingMongoDocumentsByJSON (tool_p, json_p, NULL))
						{
							res = HasMongoQueryResults (tool_p) ? 1 : 0;
						}

					WipeJSON (json_p);
				}
		}

	return res;
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




