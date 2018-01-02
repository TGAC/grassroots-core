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
//#include "rodsGenQueryNames.h"
#include "rcMisc.h"
#include "genQuery.h"


#include "typedefs.h"
#include "byte_buffer.h"
#include "meta_search.h"
#include "streams.h"
#include "memory_allocations.h"
#include "string_utils.h"
#include "query_util.h"
#include "irods_connection.h"


#ifdef _DEBUG
	#define META_SEARCH_DEBUG	(STM_LEVEL_INFO)
#else
	#define META_SEARCH_DEBUG	(STM_LEVEL_NONE)
#endif

/**
 * This is defined in the irods libs but not declared in an
 * accessible header file as far as I can see, so simply
 * declare it extern here.
 */
#if META_SEARCH_DEBUG >= STM_LEVEL_FINE
extern int printGenQI (genQueryInp_t *input_query_p);
#endif



static bool AddSearchTermNodeFromJSON (LinkedList *terms_p, const json_t * const json_p);

static SearchTermNode *AllocateSearchTermNode (const char *clause_s, const char *key_s, const int key_id, const char *op_s, const char *value_s, const int value_id);

static void FreeSearchTermNode (ListItem *node_p);



IRodsSearch *AllocateIRodsSearch (void)
{
	LinkedList *list_p = AllocateLinkedList (FreeSearchTermNode);

	if (list_p)
		{
			IRodsSearch *search_p = (IRodsSearch *) AllocMemory (sizeof (IRodsSearch));

			if (search_p)
				{
					search_p -> is_search_terms_p = list_p;

					return search_p;
				}

			FreeLinkedList (list_p);
		}		/* if (list_p) */

	return NULL;
}


void FreeIRodsSearch (IRodsSearch *search_p)
{
	FreeLinkedList (search_p -> is_search_terms_p);
	FreeMemory (search_p);
}


void ClearIRodsSearch (IRodsSearch *search_p)
{
	ClearLinkedList (search_p -> is_search_terms_p);
}


QueryResults *DoIRodsSearch (IRodsSearch *search_p, IRodsConnection *connection_p)
{
	QueryResults *results_p = NULL;
	ByteBuffer *buffer_p = AllocateByteBuffer (1024);


	if (buffer_p)
		{
			SearchTermNode *node_p = (SearchTermNode *) (search_p -> is_search_terms_p -> ll_head_p);
			bool success_flag = true;

			while (node_p && success_flag)
				{
					SearchTerm *term_p = & (node_p -> stn_term);

					if (term_p -> st_clause_s)
						{
							success_flag = AppendStringsToByteBuffer (buffer_p, " ", term_p -> st_clause_s, NULL);
						}

					if (success_flag)
						{
							success_flag = AppendStringsToByteBuffer (buffer_p, " ", term_p -> st_key_s, " ", term_p -> st_op_s, " '", term_p -> st_value_s, "'", NULL);
						}

					if (success_flag)
						{
							node_p = (SearchTermNode *) (node_p -> stn_node.ln_next_p);
						}

				}		/* while (node_p && success_flag) */


			if (success_flag)
				{
					const char terminator_s [] = ";";

					if (AppendToByteBuffer (buffer_p, terminator_s, strlen (terminator_s)))
						{
							const char *sql_s = GetByteBufferData (buffer_p);
							genQueryOut_t *out_p = ExecuteQueryString (connection_p -> ic_connection_p, (char *) sql_s);

							if (out_p)
								{
									results_p  = GenerateQueryResults (out_p);
								}
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to execute irods meta search \"%s\"\n", sql_s);
								}

						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to terminate search buffer\n");
						}
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to fill in search buffer \"%s\"\n", GetByteBufferData (buffer_p));
				}

			FreeByteBuffer (buffer_p);
		}		/* if (buffer_p) */

	return results_p;
}


bool AddMetadataDataAttributeSearchTerm (IRodsSearch *search_p, const char *clause_s, const char *key_s, const char *op_s, const char *value_s)
{
	return AddIRodsSearchTerm (search_p, clause_s, key_s, COL_META_DATA_ATTR_NAME, op_s, value_s, COL_META_DATA_ATTR_VALUE);
}


bool AddIRodsSearchTerm (IRodsSearch *search_p, const char *clause_s, const char *key_s, const int key_id, const char *op_s, const char *value_s, const int value_id)
{
	bool success_flag = false;
	SearchTermNode *node_p = AllocateSearchTermNode (clause_s, key_s, key_id, op_s, value_s, value_id);

	if (node_p)
		{
			LinkedListAddTail (search_p -> is_search_terms_p, (ListItem *) node_p);
			success_flag = true;
		}

	return success_flag;
}


static SearchTermNode *AllocateSearchTermNode (const char *clause_s, const char *key_s, const int key_id, const char *op_s, const char *value_s, const int value_id)
{
	SearchTermNode *node_p = (SearchTermNode *) AllocMemory (sizeof (SearchTermNode));

	if (node_p)
		{
			ByteBuffer *buffer_p = NULL;
			bool success_flag = true;

			node_p -> stn_node.ln_prev_p = node_p -> stn_node.ln_next_p = NULL;

			node_p -> stn_term.st_clause_s = clause_s;
			node_p -> stn_term.st_key_s = key_s;
			node_p -> stn_term.st_key_column_id = key_id;
			node_p -> stn_term.st_op_s = op_s;
			node_p -> stn_term.st_value_s = value_s;
			node_p -> stn_term.st_value_column_id = value_id;

			if (key_s)
				{
					buffer_p = AllocateByteBuffer (1024);

					if (buffer_p)
						{
							if (AppendStringsToByteBuffer (buffer_p, "='", key_s, "'", NULL))
								{
									node_p -> stn_term.st_key_buffer_s = EasyCopyToNewString (GetByteBufferData (buffer_p));

									success_flag = (node_p -> stn_term.st_key_buffer_s != NULL);
								}
							else
								{
									success_flag = false;
								}

						}		/* if (buffer_p) */
					else
						{
							success_flag = false;
						}

				}		/* if (key_s) */


			if (success_flag)
				{
					if (value_s && op_s)
						{
							if (buffer_p)
								{
									ResetByteBuffer (buffer_p);
								}
							else
								{
									buffer_p = AllocateByteBuffer (1024);
								}

							if (buffer_p)
								{
									if (AppendStringsToByteBuffer (buffer_p, op_s, " '", value_s, "'", NULL))
										{
											node_p -> stn_term.st_value_buffer_s = EasyCopyToNewString (GetByteBufferData (buffer_p));

											success_flag = (node_p -> stn_term.st_value_buffer_s != NULL);
										}
									else
										{
											success_flag = false;
										}

								}		/* if (buffer_p) */
							else
								{
									success_flag = false;
								}

						}		/* if (key_s) */

				}		/* if (success_flag) */

			if (!success_flag)
				{
					FreeSearchTermNode ((ListItem *) node_p);
					node_p = NULL;
				}

			if (buffer_p)
				{
					FreeByteBuffer (buffer_p);
				}
		}		/* if (node_p) */

	return node_p;
}


int32 DetermineSearchTerms (LinkedList *terms_p, const json_t *json_p)
{
	int32 res = 0;

	if (json_is_array (json_p))
		{
			/* array is a JSON array */
			size_t index;
			json_t *json_value_p;

			json_array_foreach (json_p, index, json_value_p)
				{
					if (AddSearchTermNodeFromJSON (terms_p, json_value_p))
						{
							++ res;
						}
					else
						{
							char *dump_s = json_dumps (json_value_p, JSON_INDENT (2));

							if (dump_s)
								{
									PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to get search term node from %s\n", dump_s);
									free (dump_s);
								}
						}
				}
		}
	else
		{
			if (AddSearchTermNodeFromJSON (terms_p, json_p))
				{
					++ res;
				}
			else
				{
					char *dump_s = json_dumps (json_p, JSON_INDENT (2));

					if (dump_s)
						{
							PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to get search term node from %s\n", dump_s);
							free (dump_s);
						}
				}
		}

	return res;
}


static void FreeSearchTermNode (ListItem *node_p)
{
	SearchTermNode *search_node_p = (SearchTermNode *) node_p;

	if (search_node_p -> stn_term.st_key_buffer_s)
		{
			FreeCopiedString (search_node_p -> stn_term.st_key_buffer_s);
		}

	if (search_node_p -> stn_term.st_value_buffer_s)
		{
			FreeCopiedString (search_node_p -> stn_term.st_value_buffer_s);
		}

	FreeMemory (node_p);
}


static bool AddSearchTermNodeFromJSON (LinkedList *terms_p, const json_t * const json_p)
{
	bool success_flag = false;
	SearchTermNode *node_p = NULL;
	json_t *child_json_p = json_object_get (json_p, "operation");

	if (child_json_p  && json_is_string (child_json_p))
		{
			const char *op_s = json_string_value (child_json_p);

			child_json_p = json_object_get (json_p, "key");

			if (child_json_p && json_is_string (child_json_p))
				{
					const char *key_s = json_string_value (child_json_p);

					child_json_p = json_object_get (json_p, "key_id");

					if (child_json_p && json_is_integer (child_json_p))
						{
							int key_id = json_integer_value (child_json_p);

							child_json_p = json_object_get (json_p, "value");

							if (child_json_p && json_is_string (child_json_p))
								{
									const char *value_s = json_string_value (child_json_p);

									child_json_p = json_object_get (json_p, "value_id");

									if (child_json_p && json_is_integer (child_json_p))
										{
											int value_id = json_integer_value (child_json_p);
											const char *clause_s = NULL;

											child_json_p = json_object_get (json_p, "clause");

											if (child_json_p && json_is_string (child_json_p))
												{
													clause_s = json_string_value (child_json_p);
												}

											node_p = AllocateSearchTermNode (clause_s, key_s, key_id, op_s, value_s, value_id);

											if (node_p)
												{
													LinkedListAddTail (terms_p, & (node_p -> stn_node));
													success_flag = true;
												}
											else
												{
													PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to allocate search term node\n");
												}

										}		/* if (child_json_p && json_is_integer (child_json_p)) */
									else
										{
											PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to get search term value column id from json\n");
										}
								}
							else
								{
									PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to get search term value from json\n");
								}

						}		/* if (child_json_p && json_is_integer (child_json_p)) */
					else
						{
							PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to get search term key column id from json\n");
						}
				}
			else
				{
					PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to get search term key from json\n");
				}
		}
	else
		{
			PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to get search term operation from json\n");
		}

	return success_flag;
}

/*
  1898  imeta add -d barley crop barley
 1899  imeta add -d wheat_and_barley crop barley
 1900  imeta add -d wheat_and_barley crop wheat
 1901  imeta add -d wheat crop wheat
 1902  imeta au -d crop wheat
 1903  imeta au -d crop = wheat
 1904  imeta qu -d crop = wheat
 1905  imeta qu -d crop =  barley
 *
 */


QueryResults *DoMetaSearchForAllDataAndCollections (const IRodsSearch * const search_p, struct IRodsConnection *connection_p, const bool upper_case_flag, char *zone_s)
{
	int columns [] = { COL_COLL_NAME, COL_DATA_NAME };

	return DoMetaSearch (search_p, connection_p, columns, 2, upper_case_flag, zone_s);
}


QueryResults *DoMetaSearch (const IRodsSearch * const search_p, struct IRodsConnection *connection_p, int *select_column_ids_p, const int num_select_columns, const bool upper_case_flag, char *zone_s)
{
	QueryResults *results_p = NULL;
  genQueryInp_t input_query;
  genQueryOut_t *query_output_p = NULL;

	/*
	 * Set up the SELECT criteria
	 */
	int *select_options_p = (int *) AllocMemoryArray (num_select_columns, sizeof (int));

	if (select_options_p)
		{
			const uint32 num_clauses = search_p -> is_search_terms_p -> ll_size;

			memset (&input_query, 0, sizeof (genQueryInp_t));

			input_query.selectInp.inx = select_column_ids_p;
			input_query.selectInp.value = select_options_p;
			input_query.selectInp.len = num_select_columns;

			if (num_clauses > 0)
				{
					/*
					 * Set up the WHERE criteria
					 */
					int *where_options_p = (int *) AllocMemoryArray (num_clauses << 1, sizeof (int));

					if (where_options_p)
						{
							ByteBuffer *buffer_p = AllocateByteBuffer (1024);

							if (buffer_p)
								{
									char **conditions_ss = (char **) AllocMemoryArray (num_clauses << 1, sizeof (char *));

									if (conditions_ss)
										{
											SearchTermNode *node_p = (SearchTermNode *) (search_p -> is_search_terms_p -> ll_head_p);
											int *where_p = where_options_p;
											char **condition_ss = conditions_ss;
											int status = 0;

											/*
											 * Add the initial condition
											 */
											*where_p = node_p -> stn_term.st_key_column_id;
											*condition_ss = node_p -> stn_term.st_key_buffer_s;

											* (++ where_p) = node_p -> stn_term.st_value_column_id;
											* (++ condition_ss) = node_p -> stn_term.st_value_buffer_s;

											node_p = (SearchTermNode *) (node_p -> stn_node.ln_next_p);

											while (node_p)
												{
													/* The key clause */
													* (++ where_p) = node_p -> stn_term.st_key_column_id;
													* (++ condition_ss) = node_p -> stn_term.st_key_buffer_s;

													* (++ where_p) = node_p -> stn_term.st_value_column_id;
													* (++ condition_ss) = node_p -> stn_term.st_value_buffer_s;

													node_p = (SearchTermNode *) (node_p -> stn_node.ln_next_p);
												}


											input_query.sqlCondInp.inx = where_options_p;
											input_query.sqlCondInp.value = conditions_ss;
											input_query.sqlCondInp.len = num_clauses << 1;

											/*
											 * Add extra options if needed
											 */
											if (upper_case_flag)
												{
													input_query.options = UPPER_CASE_WHERE;
												}

											if (zone_s)
												{
													/** @REPLACE IRODS CALL */
													char *zone_key_s = EasyCopyToNewString (ZONE_KW);

													if (zone_key_s)
														{
															addKeyVal (&input_query.condInput, zone_key_s, zone_s);
															FreeCopiedString (zone_key_s);
														}
												}

											input_query.maxRows = 10;
											input_query.continueInx = 0;
											input_query.condInput.len = 0;

											#if META_SEARCH_DEBUG >= STM_LEVEL_FINE
											printf ("BEGIN input_query\n");
											printGenQI (&input_query);
											printf ("END input_query\n");
											fflush (stdout);
											#endif

											/** @REPLACE IRODS CALL */
											/* Do the search */
											status = rcGenQuery (connection_p -> ic_connection_p, &input_query, &query_output_p);

											switch (status)
											{
												case 0:
													results_p = GenerateQueryResults (query_output_p);
													break;

												case CAT_NO_ROWS_FOUND:
													PrintLog (STM_LEVEL_INFO, __FILE__, __LINE__, "No results for meta search\n");
													break;

												default:
													break;
											}

											FreeMemory (conditions_ss);
										}		/* if (conditions_ss) */

									FreeByteBuffer (buffer_p);
								}		/* if (buffer_p) */

							FreeMemory (where_options_p);
						}		/* if (where_options_p) */

				}		/* if (num_clauses > 0) */

			FreeMemory (select_options_p);
		}		/* if (select_options_p) */

	return results_p;
}
