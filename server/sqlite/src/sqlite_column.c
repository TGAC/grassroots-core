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
 * sqlite_column.c
 *
 *  Created on: 16 Aug 2018
 *      Author: billy
 */

#include "sqlite_column.h"
#include "memory_allocations.h"
#include "string_utils.h"
#include "sqlite3.h"
#include "byte_buffer.h"


static bool AddSQLiteColumnFlagToByteBuffer (ByteBuffer *buffer_p, bool value_flag, const char *value_s);

static bool AddSQLiteColumnCheckConstraintToByteBuffer (ByteBuffer *buffer_p, const char *check_s);



SQLiteColumn *AllocateSQLiteColumn (const char *name_s, uint32 datatype, bool primary_key_flag, bool unique_flag, bool can_be_null_flag, const char *check_s)
{
	char *copied_name_s = EasyCopyToNewString (name_s);

	if (copied_name_s)
		{
			bool success_flag = true;
			char *copied_check_s = NULL;

			if (check_s)
				{
					copied_check_s = EasyCopyToNewString (check_s);

					if (!copied_check_s)
						{
							success_flag = false;
						}
				}

			if (success_flag)
				{
					SQLiteColumn *column_p = (SQLiteColumn *) AllocMemory (sizeof (SQLiteColumn));

					if (column_p)
						{
							column_p -> slc_name_s = copied_name_s;
							column_p -> slc_type = datatype;
							column_p -> slc_unique_flag = unique_flag;
							column_p -> slc_null_flag = can_be_null_flag;
							column_p -> slc_check_s = copied_check_s;
							column_p -> slc_primary_key_flag = primary_key_flag;

							return column_p;
						}		/* if (column_p) */
				}

			FreeCopiedString (copied_name_s);
		}		/* if (copied_name_s) */

	return NULL;
}


void FreeSQLiteColumn (SQLiteColumn *column_p)
{
	if (column_p -> slc_name_s)
		{
			FreeCopiedString (column_p -> slc_name_s);
		}

	if (column_p -> slc_check_s)
		{
			FreeCopiedString (column_p -> slc_check_s);
		}


	FreeMemory (column_p);
}


SQLiteColumnNode *AllocateSQLiteColumnNode (const char *name_s, uint32 datatype, bool primary_key_flag, bool unique_flag, bool can_be_null_flag, const char *check_s)
{
	SQLiteColumn *column_p = AllocateSQLiteColumn (name_s, datatype, primary_key_flag, unique_flag, can_be_null_flag, check_s);

	if (column_p)
		{
			SQLiteColumnNode *node_p = (SQLiteColumnNode *) AllocMemory (sizeof (SQLiteColumnNode));

			if (node_p)
				{
					node_p -> sqlcn_column_p = column_p;

					node_p -> sqlcn_node.ln_prev_p = NULL;
					node_p -> sqlcn_node.ln_next_p = NULL;

					return node_p;
				}

			FreeSQLiteColumn (column_p);
		}

	return NULL;
}


void FreeSQLiteColumnNode (ListItem *node_p)
{
	SQLiteColumnNode *column_node_p = (SQLiteColumnNode *) node_p;

	FreeSQLiteColumn (column_node_p -> sqlcn_column_p);
	FreeMemory (column_node_p);
}


const char *GetSQLiteColumnDatatypeAsString (const SQLiteColumn *column_p)
{
	const char *datatype_s = NULL;

	switch (column_p -> slc_type)
		{
			case SQLITE_INTEGER:
				datatype_s = "INTEGER";
				break;

			case SQLITE_FLOAT:
				datatype_s = "REAL";
				break;

			case SQLITE_NULL:
				datatype_s = "NULL";
				break;

			case SQLITE_TEXT:
				datatype_s = "TEXT";
				break;

			case SQLITE_BLOB:
				datatype_s = "BLOB";
				break;

			default:
				break;
		}

	return datatype_s;
}


char *GetSQLiteColumnAsString (const SQLiteColumn *column_p)
{
	ByteBuffer *buffer_p = AllocateByteBuffer (1024);

	if (buffer_p)
		{
			const char *datatype_s = GetSQLiteColumnDatatypeAsString (column_p);

			if (datatype_s)
				{
					if (AppendStringsToByteBuffer (buffer_p, column_p -> slc_name_s, " ", datatype_s, NULL))
						{
							if (AddSQLiteColumnFlagToByteBuffer (buffer_p, column_p -> slc_primary_key_flag, "PRIMARY KEY"))
								{
									if (AddSQLiteColumnFlagToByteBuffer (buffer_p, ! (column_p -> slc_null_flag), "NOT NULL"))
										{
											if (AddSQLiteColumnFlagToByteBuffer (buffer_p, column_p -> slc_unique_flag, "UNIQUE"))
												{
													if (AddSQLiteColumnCheckConstraintToByteBuffer (buffer_p, column_p -> slc_check_s))
														{
															char *data_s = DetachByteBufferData (buffer_p);

															return data_s;
														}
												}
										}
								}
						}
				}

			FreeByteBuffer (buffer_p);
		}

	return NULL;
}


static bool AddSQLiteColumnCheckConstraintToByteBuffer (ByteBuffer *buffer_p, const char *check_s)
{
	bool success_flag = true;

	if (check_s)
		{
			if (! (AppendStringsToByteBuffer (buffer_p, " CHECK (", check_s, ")", NULL)))
				{
					success_flag = false;
				}
		}

	return success_flag;
}


static bool AddSQLiteColumnFlagToByteBuffer (ByteBuffer *buffer_p, bool value_flag, const char *value_s)
{
	bool success_flag = true;

	if (value_flag)
		{
			if (! (AppendStringsToByteBuffer (buffer_p, " ", value_s, NULL)))
				{
					success_flag = false;
				}
		}

	return success_flag;
}
