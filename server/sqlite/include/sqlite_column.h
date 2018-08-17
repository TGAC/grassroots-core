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
 * sqlite_column.h
 *
 *  Created on: 16 Aug 2018
 *      Author: billy
 */

#ifndef CORE_SERVER_SQLITE_INCLUDE_SQLITE_COLUMN_H_
#define CORE_SERVER_SQLITE_INCLUDE_SQLITE_COLUMN_H_

#include "typedefs.h"
#include "linked_list.h"
#include "sqlite_library.h"


/**
 * A datatype to model an SQLite column.
 */
typedef struct
{
	/**
	 * The name of the column.
	 */
	char *slc_name_s;

	/**
	 * The datatype for the column. This can be one of
	 *
	 * SQLITE_INTEGER
	 * SQLITE_FLOAT
	 * SQLITE_BLOB
 	 * SQLITE_NULL
	 * SQLITE_TEXT
	 */
	uint32 slc_type;

	/**
	 *  Does this column have a UNIQUE constraint?
	 */
	bool slc_unique_flag;

	/**
	 * Can the column contain NULL values?
	 */
	bool slc_null_flag;

	/**
	 * The column's CHECK constraint.
	 */
	char *slc_check_s;

	/**
	 * Is this column the primary key in its table?
	 */
	bool slc_primary_key_flag;

} SQLiteColumn;


/**
 * A datatype for storing a SQLiteColumn on a LinkedList.
 */
typedef struct
{
	/** The base ListItem */
	ListItem sqlcn_node;

	/** The SQLiteColumn */
	SQLiteColumn *sqlcn_column_p;

} SQLiteColumnNode;


#ifdef __cplusplus
extern "C"
{
#endif


GRASSROOTS_SQLITE_API	SQLiteColumn *AllocateSQLiteColumn (const char *name_s, uint32 datatype, bool primary_key_flag, bool unique_flag, bool can_be_null_flag, const char *check_s);

GRASSROOTS_SQLITE_API	void FreeSQLiteColumn (SQLiteColumn *column_p);

GRASSROOTS_SQLITE_API	SQLiteColumnNode *AllocateSQLiteColumnNode (const char *name_s, uint32 datatype, bool primary_key_flag, bool unique_flag, bool can_be_null_flag, const char *check_s);

GRASSROOTS_SQLITE_API	void FreeSQLiteColumnNode (ListItem *node_p);

GRASSROOTS_SQLITE_API char *GetSQLiteColumnAsString (const SQLiteColumn *column_p);

GRASSROOTS_SQLITE_API const char *GetSQLiteColumnDatatypeAsString (const SQLiteColumn *column_p);


#ifdef __cplusplus
}
#endif



#endif /* CORE_SERVER_SQLITE_INCLUDE_SQLITE_COLUMN_H_ */
