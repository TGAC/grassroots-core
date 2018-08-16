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

#include "sqlite_tool.h"
#include "sqlite_column.h"


static LinkedList *GetTableColumns (void);

static bool AddColumn (LinkedList *columns_p, char *name_s, uint32 datatype, bool primary_key_flag, bool unique_flag, bool can_be_null_flag, char *check_s);



int main (int argc, char *argv [])
{
	const char *table_s = "test_table";
	SQLiteTool *tool_p = AllocateSQLiteTool ("test_db", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, table_s);

	if (tool_p)
		{
			LinkedList *columns_p = NULL;

			puts ("Allocated SQLiteTool");

			columns_p = GetTableColumns ();

			if (columns_p)
				{
					char *error_s = NULL;

					puts ("Got Table Columns");

					error_s = CreateSQLiteTable (tool_p, table_s, columns_p);

					FreeLinkedList (columns_p);

					if (error_s)
						{
							printf ("error: \"%s\"\n", error_s);
						}
					else
						{
							json_t *values_p;

							InsertOrUpdateSQLiteData (tool_p, values_p, table_s, "id");
						}

				}		/* if (columns_p) */
			else
				{
					puts ("Failed to get Table Columns");
				}

			FreeSQLiteTool (tool_p);
		}		/* if (tool_p) */
	else
		{
			puts ("Failed to allocate SQLiteTool");
		}

	puts ("exiting");

	return 0;
}



static LinkedList *GetTableColumns (void)
{
	LinkedList *columns_p = AllocateLinkedList (FreeSQLiteColumnNode);

	if (columns_p)
		{
			if (AddColumn (columns_p, "id", SQLITE_INTEGER, true, false, false, NULL))
				{
					if (AddColumn (columns_p, "surname", SQLITE_TEXT, false, false, false, NULL))
						{
							if (AddColumn (columns_p, "forename", SQLITE_TEXT, false, false, false, NULL))
								{
									if (AddColumn (columns_p, "height", SQLITE_FLOAT, false, false, false, NULL))
										{
											if (AddColumn (columns_p, "description", SQLITE_TEXT, false, false, true, NULL))
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


static bool AddColumn (LinkedList *columns_p, char *name_s, uint32 datatype, bool primary_key_flag, bool unique_flag, bool can_be_null_flag, char *check_s)
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

