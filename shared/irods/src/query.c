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
#include <stdlib.h>
#include <string.h>

#include "data_resource.h"
#include "query.h"


#include "rodsGenQueryNames.h"
#include "rcMisc.h"
#include "rcConnect.h"
#include "rodsGenQuery.h"
#include "genQuery.h"

#include "string_utils.h"
#include "memory_allocations.h"
#include "byte_buffer.h"
#include "streams.h"
#include "json_util.h"
#include "irods_connection.h"
#include "query_util.h"


#ifdef _DEBUG
	#define QUERY_DEBUG	(STM_LEVEL_FINE)
#else
	#define QUERY_DEBUG	(STM_LEVEL_NONE)
#endif



/**********************************/

static const columnName_t *GetColumnById (const int id);

static bool AddQueryResultsAsResourcesJSON (const char *full_path_s, const char *collection_s, const char *data_s, void *data_p);

static bool AddQueryResultsAsStrings (const char *full_path_s, const char *collection_s, const char *data_s, void *data_p);

static void IterateOverQueryResultPaths (const QueryResults * const qrs_p, bool (*callback_fn) (const char *full_path_s, const char *collection_s, const char *data_s, void *callback_data_p), void *callback_data_p);


/*********************************/


genQueryOut_t *ExecuteGenQuery (rcComm_t *connection_p, genQueryInp_t * const in_query_p)
{
	genQueryOut_t *out_query_p = NULL;
	int status = rcGenQuery (connection_p, in_query_p, &out_query_p);


	/* Did we run it successfully? */
	if (status == 0)
		{
			#if QUERY_DEBUG >= STM_LEVEL_FINER
			#endif
		}
	else if (status == CAT_NO_ROWS_FOUND)
		{
			printf ("No rows found\n");
		}
	else if (status < 0 )
		{
			printf ("error status: %d\n", status);
		}
	else
		{
			//printBasicGenQueryOut (out_query_p, "result: \"%s\" \"%s\"\n");
		}

	return out_query_p;
}


void InitGenQuery (genQueryInp_t *query_p)
{
	memset (query_p, 0, sizeof (genQueryInp_t));
	query_p -> maxRows = MAX_SQL_ROWS;
	query_p -> continueInx = 0;
}


void ClearGenQuery (genQueryInp_t *query_p)
{
	if (query_p -> selectInp.len)
		{
			free (query_p -> selectInp.inx);
			query_p -> selectInp.inx = NULL;

			free (query_p -> selectInp.value);
			query_p -> selectInp.value = NULL;

			query_p -> selectInp.len = 0;
		}


	if (query_p -> sqlCondInp.len)
		{
			free (query_p -> sqlCondInp.inx);
			query_p -> sqlCondInp.inx = NULL;

			free (query_p -> sqlCondInp.value);
			query_p -> sqlCondInp.value = NULL;

			query_p -> sqlCondInp.len = 0;
		}

}


QueryResults *GetAllMetadataDataAttributeNames (IRodsConnection *connection_p)
{
	return GetAllAttributeNames (connection_p, COL_META_DATA_ATTR_NAME);
}


QueryResults *GetAllMetadataDataAttributeValues (IRodsConnection *connection_p, const char * const name_s)
{
	return GetAllMetadataAttributeValues (connection_p, COL_META_DATA_ATTR_NAME, name_s, COL_META_DATA_ATTR_VALUE);
}


QueryResults *GetAllMetadataCollectionAttributeNames (IRodsConnection *connection_p)
{
	return GetAllAttributeNames (connection_p, COL_META_COLL_ATTR_NAME);
}


QueryResults *GetAllMetadataUserAttributeNames (IRodsConnection *connection_p)
{
	return GetAllAttributeNames (connection_p, COL_META_USER_ATTR_NAME);
}


genQueryOut_t *ExecuteQueryString (rcComm_t *connection_p, char *query_s)
{
	genQueryInp_t in_query;
	genQueryOut_t *out_query_p = NULL;
	int status;

	/* Build the query */
	InitGenQuery (&in_query);

	/* Fill in the iRODS query structure */
	status = fillGenQueryInpFromStrCond (query_s, &in_query);

	if (status >= 0)
		{
			out_query_p = ExecuteGenQuery (connection_p, &in_query);
		}


	clearGenQueryInp (&in_query);

	return out_query_p;
}


char *BuildQueryString (const char **args_ss)
{
	char *buffer_p = NULL;
	const char **arg_pp = args_ss;
	size_t len = 0;

	/* Determine the length of string that we need */
	while (*arg_pp)
		{
			/* could cache these lengths to use later on... */
			len += strlen (*arg_pp);
			++ arg_pp;
		}

	buffer_p = (char *) AllocMemory ((len + 1) * sizeof (char));
	if (buffer_p)
		{
			char *current_p = buffer_p;
			arg_pp = args_ss;

			while (*arg_pp)
				{
					size_t l = strlen (*arg_pp);
					strncpy (current_p, *arg_pp, l);

					current_p += l;
					++ arg_pp;
				}

			/* terminate the string */
			*current_p = '\0';
		}

	return buffer_p;
}


void FreeBuiltQueryString (char *query_s)
{
	free (query_s);
}



bool SetQuerySelectClauses (genQueryInp_t *in_query_p, int num_columns, const int * const columns_p, const int * const UNUSED_PARAM  (values_p))
{
	bool success_flag = false;

	in_query_p -> selectInp.inx = (int *) AllocMemory (num_columns * sizeof (int));

	if (in_query_p -> selectInp.inx)
		{
			in_query_p -> selectInp.value = (int *) AllocMemory (num_columns * sizeof (int));

			if (in_query_p -> selectInp.value)
				{
					int *ptr = in_query_p -> selectInp.value;

					in_query_p -> selectInp.len = num_columns;

					/* fill in the columns */
					memcpy (in_query_p -> selectInp.inx, columns_p, num_columns * sizeof (int));

					/* we just want default selections, so set all of the values to 1 */
					while (num_columns > 0)
						{
							*ptr = 1;

							++ ptr;
							-- num_columns;
						}

					success_flag = true;
				}		/* if (in_query_p -> selectInp.value) */

			if (!success_flag)
				{
					free (in_query_p -> selectInp.inx);
					in_query_p -> selectInp.inx = NULL;
				}
		}

	return success_flag;
}


bool SetQueryWhereClauses (genQueryInp_t *in_query_p, int num_columns, const int *columns_p, const char **clauses_ss, const char **where_ops_ss)
{
	bool success_flag = false;

	if ((num_columns > 0) && columns_p && clauses_ss)
		{

			in_query_p -> sqlCondInp.inx = (int *) AllocMemory (num_columns * sizeof (int));

			if (in_query_p -> sqlCondInp.inx)
				{
					in_query_p -> sqlCondInp.value = (char **) AllocMemory (num_columns * sizeof (char *));

					if (in_query_p -> sqlCondInp.value)
						{
							char **dest_pp = in_query_p -> sqlCondInp.value;
							const char **src_pp = clauses_ss;
							const char **op_pp = where_ops_ss;

							in_query_p -> sqlCondInp.len = num_columns;

							/* fill in the columns */
							memcpy (in_query_p -> sqlCondInp.inx, columns_p, num_columns * sizeof (int));

							/* add the operators */
							while (num_columns > 0)
								{
									const char *op_s = (op_pp && *op_pp) ? *op_pp : "= \'";
									char *clause_s = ConcatenateVarargsStrings (op_s, *src_pp, "\'", NULL);

									if (clause_s)
										{
											*dest_pp = clause_s;

											++ dest_pp;

											++ src_pp;
											if (op_pp)
												{
													++ op_pp;
												}
											-- num_columns;
										}
									else
										{
											while (dest_pp >= in_query_p -> sqlCondInp.value)
												{
													FreeCopiedString (*dest_pp);
													-- dest_pp;
												}
										}
								}

							if (num_columns == 0)
								{
									success_flag = true;
								}
							else
								{
									free (in_query_p -> sqlCondInp.value);
									in_query_p -> sqlCondInp.value = NULL;
								}
						}

					if (!success_flag)
						{
							free (in_query_p -> sqlCondInp.inx);
							in_query_p -> sqlCondInp.inx = NULL;
						}
				}
		}
	else
		{
			success_flag = true;
		}

	return success_flag;
}


int PrintQueryOutput (FILE *out_f, const genQueryOut_t *query_result_p)
{
	int i;

	for (i = 0;i < query_result_p -> rowCnt; ++ i)
		{
			int j;
			const sqlResult_t *sql_result_p = query_result_p -> sqlResult;

			fprintf (out_f, "i = %d ----\n", i);

			for (j = 0; j < query_result_p -> attriCnt; ++ j, ++ sql_result_p)
				{
					char *result_s = sql_result_p -> value;

					result_s += i * (sql_result_p -> len);

					if (fprintf (out_f, "j = %d (%d) -> %s (%d)\n", j, sql_result_p -> attriInx, result_s, sql_result_p -> len) < 0)
						{
							return -1;
						}
				}
		}

	return 0;
}


/*
 *
 * 	const columnName_t *qr_column_p;
	char **qr_values_pp;
	int qr_num_values;
	*/

QueryResult *AllocateQueryResult (int num_rows, const columnName_t *column_p)
{
	char **values_pp = (char **) AllocMemoryArray (num_rows, sizeof (char *));

	if (values_pp)
		{
			QueryResult *result_p = (QueryResult *) AllocMemory (sizeof (QueryResult));

			if (result_p)
				{
					result_p -> qr_num_values = num_rows;
					result_p -> qr_column_p = column_p;
					result_p -> qr_values_pp = values_pp;

					return result_p;
				}

			FreeMemory (values_pp);
		}

	return NULL;
}


bool InitQueryResult (QueryResult *result_p, int num_rows, const columnName_t *column_p)
{
	bool success_flag = false;
	char **values_pp = (char **) AllocMemoryArray (num_rows, sizeof (char *));

	if (values_pp)
		{
			result_p -> qr_num_values = num_rows;
			result_p -> qr_column_p = column_p;
			result_p -> qr_values_pp = values_pp;

			success_flag = true;
		}

	return success_flag;
}


bool SetQueryResultValue (QueryResult *result_p, int index, const char *value_s)
{
	bool success_flag = false;
	char *copied_value_s = strdup (value_s);

	if (copied_value_s)
		{
			char **value_pp = (result_p -> qr_values_pp) + index;

			if (*value_pp)
				{
					free (*value_pp);
				}

			*value_pp = copied_value_s;

			success_flag = true;
		}


	return success_flag;
}


const char *GetColumnNameForId (const int id)
{
	char *col_name_s = NULL;
	const columnName_t *column_p = GetColumnById (id);

	if (column_p)
		{
			col_name_s = column_p -> columnName;
		}

	return col_name_s;
}


const columnName_t *GetColumnById (const int id)
{
	const columnName_t *column_p = columnNames;
  int i;

  for (i = NumOfColumnNames; i > 0; -- i, ++ column_p)
		{
			if (column_p -> columnId == id)
				{
					return column_p;
				}
		}

#ifdef EXTENDED_ICAT
	column_p = extColumnNames;

  for (i = NumOfExtColumnNames; i > 0; -- i, ++ column_p)
		{
			if (column_p -> columnId == id)
				{
					return column_p;
				}
		}
#endif

  return NULL;
}



void PrintQueryResults (FILE *out_f, const QueryResults * const result_p)
{
	/*
	int qr_num_results;
	QueryResult *qr_values_p;
	*/

	int i;
	QueryResult *qr_p = result_p -> qr_values_p;

	for (i = 0; i < result_p -> qr_num_results; ++ i, ++ qr_p)
		{
			fprintf (out_f, "%d :=\n", i);
			PrintQueryResult (out_f, qr_p);
		}

}


void PrintQueryResult (FILE *out_f, const QueryResult * const result_p)
{
	const columnName_t *col_p = (const columnName_t *) (result_p -> qr_column_p);
	int i;
	char **values_pp = result_p -> qr_values_pp;

	fprintf (out_f, "col: %d - \"%s\"\n", col_p -> columnId, col_p -> columnName);


	for (i = 0; i < result_p -> qr_num_values; ++ i, ++ values_pp)
		{
			fprintf (out_f, "%d: \"%s\"\n", i, *values_pp);
		}

}


json_t *GetQueryResultAsJSON (const QueryResults * const qrs_p)
{
	json_t *root_p = json_array ();

	if (root_p)
		{
			/*
			 * We know that QueryResults is tabular so we just need
			 * to query the first result to get the number of rows.
			 */
			int num_rows = qrs_p -> qr_values_p -> qr_num_values;
			int i = 0;
			bool success_flag = true;

			while ((i < num_rows) && success_flag)
				{
					QueryResult *qr_p = qrs_p -> qr_values_p;
					int j = 0;
					const int num_columns =  qrs_p -> qr_num_results;

					while ((j < num_columns) && success_flag)
						{
							json_t *json_row_p = NULL;

							if (j == 0)
								{
									json_row_p = json_object ();

									if (json_row_p)
										{
											if (json_array_append_new (root_p, json_row_p) != 0)
												{
													json_decref (json_row_p);
													json_row_p = NULL;
												}
										}
								}
							else
								{
									json_row_p = json_array_get (root_p, i);
								}

							#if QUERY_DEBUG >= STM_LEVEL_FINE
							{
								char *dump_s = json_dumps (root_p, JSON_INDENT (2) | JSON_PRESERVE_ORDER);
								PrintLog (STM_LEVEL_FINE, __FILE__, __LINE__, "%s\n", dump_s);
								free (dump_s);
							}
							#endif

							success_flag = false;

							if (json_row_p)
								{
									const columnName_t *col_p = (const columnName_t *) (qr_p -> qr_column_p);
									json_t *value_p = json_string (* ((qr_p -> qr_values_pp) + i));

									if (value_p)
										{
											if (json_object_set_new (json_row_p, col_p -> columnName, value_p) == 0)
												{
													success_flag = true;
												}
										}		/* if (col_name_p) */

									if (!success_flag)
										{
											if (json_object_clear (json_row_p) != 0)
												{
													//error
												}
										}

								}		/* if (json_row_p) */

							#if QUERY_DEBUG >= STM_LEVEL_FINE
							{
								char *dump_s = json_dumps (root_p, JSON_INDENT (2) | JSON_PRESERVE_ORDER);
								PrintLog (STM_LEVEL_FINE, __FILE__, __LINE__, "%s\n", dump_s);
								free (dump_s);
							}
							#endif

							if (success_flag)
								{
									++ j;
									++ qr_p;
								}
						}		/* while ((j < num_columns) && success_flag) */


					++ i;
				}

			if (!success_flag)
				{
					json_object_clear (root_p);
					json_decref (root_p);
					root_p = NULL;
				}

		}		/* if (root_p) */

	#if QUERY_DEBUG >= STM_LEVEL_FINE
	{
		char *dump_s = json_dumps (root_p, JSON_INDENT (2) | JSON_PRESERVE_ORDER);
		PrintLog (STM_LEVEL_FINE, __FILE__, __LINE__, "%s\n", dump_s);
		free (dump_s);
	}
	#endif

	return root_p;
}


LinkedList *GetQueryResultsPaths (const QueryResults * const qrs_p)
{
	LinkedList *paths_p = AllocateStringLinkedList ();

	if (paths_p)
		{
			IterateOverQueryResultPaths (qrs_p, AddQueryResultsAsStrings, paths_p);
		}		/* if (paths_p) */

	return paths_p;
}


json_t *GetQueryResultAsResourcesJSON (const QueryResults * const qrs_p)
{
	json_t *root_p = json_array ();

	if (root_p)
		{
			IterateOverQueryResultPaths (qrs_p, AddQueryResultsAsResourcesJSON, root_p);
		}

	#if QUERY_DEBUG >= STM_LEVEL_FINE
		{
			PrintJSONToLog (QUERY_DEBUG, __FILE__, __LINE__, root_p, "GetQueryResultAsResourcesJSON exit:\n");
		}
	#endif


	return root_p;
}


static bool AddQueryResultsAsStrings (const char *full_path_s, const char * UNUSED_PARAM (collection_s), const char * UNUSED_PARAM (data_s), void *data_p)
{
	bool success_flag = false;
	LinkedList *paths_p = (LinkedList *) data_p;
	StringListNode *node_p = AllocateStringListNode (full_path_s, MF_DEEP_COPY);

	if (node_p)
		{
			LinkedListAddTail (paths_p, (ListItem *) node_p);
			success_flag = true;
		}

	return success_flag;
}



static bool AddQueryResultsAsResourcesJSON (const char *full_path_s, const char * UNUSED_PARAM (collection_s), const char *data_s, void *data_p)
{
	bool success_flag = false;
	json_t *root_p = (json_t *) data_p;
	json_t *resource_p = GetDataResourceAsJSONByParts (PROTOCOL_IRODS_S, full_path_s, data_s, NULL);


	if (resource_p)
		{
			#if QUERY_DEBUG >= STM_LEVEL_FINE
			{
				char *dump_s = json_dumps (resource_p, JSON_INDENT (2));
				PrintLog (STM_LEVEL_FINE, __FILE__, __LINE__, "resource:\n%s\n", dump_s);
				free (dump_s);
			}
			#endif


			if (json_array_append_new (root_p, resource_p) == 0)
				{
					success_flag = true;
				}
			else
				{
					json_decref (resource_p);
				}
		}		/* if (resource_p) */

	return success_flag;
}


static void IterateOverQueryResultPaths (const QueryResults * const qrs_p, bool (*callback_fn) (const char *full_path_s, const char *collection_s, const char *data_s, void *callback_data_p), void *callback_data_p)
{
	/*
	 * Find the collection and data columns
	 */
	QueryResult *collection_results_p = NULL;
	QueryResult *data_results_p = NULL;
	QueryResult *qr_p = qrs_p -> qr_values_p;
	int i = qrs_p -> qr_num_results;
	bool success_flag = false;

	while ((i > 0) && ((data_results_p == NULL) || (collection_results_p == NULL)))
		{
			const columnName_t *col_p = (const columnName_t *) (qr_p -> qr_column_p);

			if (col_p -> columnId == COL_COLL_NAME)
				{
					collection_results_p = qr_p;
				}
			else if (col_p -> columnId == COL_DATA_NAME)
				{
					data_results_p = qr_p;
				}

			++ qr_p;
			-- i;

		}		/* while ((i > 0) && ((data_results_p == NULL) || (collection_results_p == NULL))) */

	if (collection_results_p && data_results_p)
		{
			ByteBuffer *buffer_p = AllocateByteBuffer (1024);

			if (buffer_p)
				{
					/*
					 * We know that QueryResults is tabular so we just need
					 * to query the first result to get the number of rows.
					 */
					int num_rows = data_results_p -> qr_num_values;
					char **collection_values_pp = collection_results_p -> qr_values_pp;
					char **data_values_pp = data_results_p -> qr_values_pp;

					success_flag = true;

					for (i = 0; i < num_rows; ++ i, ++ collection_values_pp, ++ data_values_pp)
						{
							success_flag = false;
							const char *collection_s =  *collection_values_pp;
							const char *data_s =  *data_values_pp;

							if (AppendStringsToByteBuffer (buffer_p, collection_s, "/", data_s, NULL))
								{
									const char *full_path_s = GetByteBufferData (buffer_p);

									success_flag = callback_fn (full_path_s, collection_s, data_s, callback_data_p);

									ResetByteBuffer (buffer_p);
								}

							if (!success_flag)
								{
									i = num_rows;
								}
						}

				}		/* if (buffer_p) */

		}		/* if (collection_results_p && data_results_p) */

}



bool FillInQueryResult (QueryResult *query_result_p, const sqlResult_t *sql_result_p, const int num_rows)
{
	bool success_flag = false;
	const columnName_t *column_p = GetColumnById (sql_result_p -> attriInx);

	if (column_p)
		{
			if (InitQueryResult (query_result_p, num_rows, column_p))
				{
					char *result_s = sql_result_p -> value;
					const int len = sql_result_p -> len;
					int i;

					success_flag = true;

					for (i = 0; i < num_rows; ++ i, result_s += len)
						{
							if (!SetQueryResultValue (query_result_p, i, result_s))
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to set query result %i of %i to %s", i, num_rows, result_s);
									success_flag = false;
									i = num_rows;		/* force exit from loop */
								}
						}
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "InitQueryResult failed");
				}
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Could not get column name for %d", sql_result_p -> attriInx);
		}


	return success_flag;
}


QueryResults *GenerateQueryResults (const genQueryOut_t *query_result_p)
{
	QueryResults *results_p = (QueryResults *) AllocMemory (sizeof (QueryResults));

	if (results_p)
		{
			const int num_rows = query_result_p -> rowCnt;
			const int num_columns = query_result_p -> attriCnt;

			results_p -> qr_num_results = 0;
			results_p -> qr_values_p = NULL;

			if (num_rows > 0)
				{
					QueryResult *value_p = (QueryResult *) AllocMemory (num_columns * sizeof (QueryResult));

					if (value_p)
						{
							const sqlResult_t *sql_result_p = query_result_p -> sqlResult;
							bool success_flag = true;
							int i = num_columns;

							results_p -> qr_num_results = num_columns;
							results_p -> qr_values_p = value_p;

							while ((i > 0) && success_flag)
								{
									if (FillInQueryResult (value_p, sql_result_p, num_rows))
										{
											++ sql_result_p;
											++ value_p;

											-- i;
										}
									else
										{
											success_flag = false;
										}
								}


							if (success_flag)
								{
									return results_p;
								}
						}		/* if (value_p) */

				}		/* if (num_rows > 0) */


			FreeQueryResults (results_p);
		}		/* if (results_p) */


	return NULL;
}


void FreeQueryResults (QueryResults *results_p)
{
	int i = results_p -> qr_num_results;
	QueryResult *qr_p = results_p -> qr_values_p;

	for ( ; i > 0; -- i, ++ qr_p)
		{
			ClearQueryResult (qr_p);
		}

	if (results_p -> qr_values_p)
		{
			free (results_p -> qr_values_p);
		}

 	free (results_p);
}


void ClearQueryResult (QueryResult *result_p)
{
	int i = result_p -> qr_num_values;
	char ** value_pp = result_p -> qr_values_pp;

	for ( ; i > 0; -- i, ++ value_pp)
		{
			if (*value_pp)
				{
					free (*value_pp);
				}
		}

	FreeMemory (result_p -> qr_values_pp);
}


QueryResults *GetAllMetadataAttributeNames (IRodsConnection *connection_p)
{
	return GetAllAttributeNames (connection_p, COL_META_DATA_ATTR_NAME);
}


QueryResults *GetAllAttributeNames (IRodsConnection *connection_p, const int col_id)
{
	QueryResults *results_p = NULL;
	const char *col_s = GetColumnNameForId (col_id);

	if (col_s)
		{
			char *query_s = ConcatenateVarargsStrings ("SELECT ORDER(", col_s, ")", NULL);

			if (query_s)
				{
					genQueryOut_t *out_p = ExecuteQueryString (connection_p -> ic_connection_p, query_s);

					if (out_p)
						{
							results_p  = GenerateQueryResults (out_p);
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to execute \"%s\"", query_s);
						}

					freeGenQueryOut (&out_p);

					FreeCopiedString (query_s);
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Could not create select statement");
				}
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Could not get column name for %d", col_id);
		}

	return results_p;
}


QueryResults *GetAllMetadataAttributeValues (IRodsConnection *connection_p, const int key_id, const char * const key_s, const int value_id)
{
	QueryResults *results_p = NULL;
	const char *key_col_s = GetColumnNameForId (key_id);

	if (key_col_s)
		{
			const char *value_col_s = GetColumnNameForId (value_id);

			if (value_col_s)
				{
					ByteBuffer *buffer_p = AllocateByteBuffer (1024);

					if (buffer_p)
						{
							if (AppendStringsToByteBuffer (buffer_p, "SELECT ", value_col_s, " WHERE ", key_col_s, " = '", key_s, "';", NULL))
								{
									genQueryOut_t *out_p = ExecuteQueryString (connection_p -> ic_connection_p, (char *) GetByteBufferData (buffer_p));

									if (out_p)
										{
											results_p  = GenerateQueryResults (out_p);
										}
									else
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to execute \"%s\"", GetByteBufferData (buffer_p));
										}

									freeGenQueryOut (&out_p);
								}
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Could not create select statement");
								}

							FreeByteBuffer (buffer_p);
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Could not allocate ByteBuffer");
						}

				}		/* if (value_col_s) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Could not get value column name for %d", value_id);
				}

		}		/* if (key_col_s) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Could not get key column name for %d", key_id);
		}

	return results_p;
}

