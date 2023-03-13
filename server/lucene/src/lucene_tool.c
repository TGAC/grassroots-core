/*
 * Copyright 2014-2016 The Earlham Institute
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

#include <stdio.h>
#include <string.h>

#define ALLOCATE_LUCENE_TOOL_CONSTANTS (1)
#include "lucene_tool.h"
#include "memory_allocations.h"
#include "string_utils.h"
#include "grassroots_server.h"
#include "byte_buffer.h"
#include "json_util.h"
#include "key_value_pair.h"
#include "string_utils.h"
#include "filesystem_utils.h"
#include "math_utils.h"
#include "lucene_facet.h"
#include "uuid_util.h"
#include "async_task.h"


static bool LoadDocument (const json_t *result_p, LuceneDocument *document_p);

static bool AddNumericArgument (ByteBuffer *buffer_p, const char * const key_s, const uint32 value, const uint32 default_value);

static bool ReplaceValidString (const char *src_s, char **dest_ss);

static bool ParseFacetResults (LuceneTool *tool_p, const json_t *results_p);



LuceneTool *AllocateLuceneTool (GrassrootsServer *grassroots_p, uuid_t id)
{
	LuceneTool *tool_p = (LuceneTool *) AllocMemory (sizeof (LuceneTool));

	if (tool_p)
		{
			const json_t *lucene_config_p = GetGlobalConfigValue (grassroots_p, "lucene");

			if (lucene_config_p)
				{
					const char *classpath_s = GetJSONString (lucene_config_p, "classpath");

					if (classpath_s)
						{
							const char *index_s = GetJSONString (lucene_config_p, "index");

							if (index_s)
								{
									const char *taxonomy_s = GetJSONString (lucene_config_p, "taxonomy");

									if (taxonomy_s)
										{
											const char *search_class_s = GetJSONString (lucene_config_p, "search_class");

											if (search_class_s)
												{
													const char *index_class_s = GetJSONString (lucene_config_p, "index_class");

													if (index_class_s)
														{
															const char *delete_class_s = GetJSONString (lucene_config_p, "delete_class");

															if (delete_class_s)
																{
																	const char *working_directory_s = GetJSONString (lucene_config_p, "working_directory");

																	if (working_directory_s)
																		{
																			const char *facet_key_s = GetJSONString (lucene_config_p, "facet_key");

																			if (facet_key_s)
																				{
																					LinkedList *facet_results_p = AllocateLinkedList (FreeLuceneFacetNode);

																					if (facet_results_p)
																						{
																							tool_p -> lt_name_s = NULL;
																							tool_p -> lt_search_class_s = search_class_s;
																							tool_p -> lt_delete_class_s = delete_class_s;
																							tool_p -> lt_index_class_s = index_class_s;
																							tool_p -> lt_classpath_s = classpath_s;
																							tool_p -> lt_index_s = index_s;
																							tool_p -> lt_taxonomy_s = taxonomy_s;
																							tool_p -> lt_working_directory_s = working_directory_s;
																							tool_p -> lt_facet_key_s = facet_key_s;
																							tool_p -> lt_output_file_s = NULL;
																							tool_p -> lt_num_total_hits = 0;
																							tool_p -> lt_hits_from_index = 0;
																							tool_p -> lt_hits_to_index = 0;

																							uuid_copy (tool_p -> lt_id, id);

																							tool_p -> lt_facet_results_p = facet_results_p;

																							return tool_p;

																						}
																					else
																						{
																							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate list for storing LuceneFacets");
																						}

																				}
																			else
																				{
																					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, lucene_config_p, "Failed to find \"facet_key\" in lucene config");
																				}

																		}
																	else
																		{
																			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, lucene_config_p, "Failed to find \"working_directory\" in lucene config");
																		}

																}		/* if (delete_class_s) */
															else
																{
																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, lucene_config_p, "Failed to find \"delete_class\" in lucene config");
																}

														}		/* if (index_class_s) */
													else
														{
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, lucene_config_p, "Failed to find \"index_class\" in lucene config");
														}

												}		/* if (search_class_s) */
											else
												{
													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, lucene_config_p, "Failed to find \"search_class\" in lucene config");
												}

										}		/* if (taxonomy_s) */
									else
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, lucene_config_p, "Failed to find \"taxonomy\" in lucene config");
										}

								}		/* if (index_s) */
							else
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, lucene_config_p, "Failed to find \"index\" in lucene config");
								}

						}		/* if (classpath_s) */
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, lucene_config_p, "Failed to find \"classpath\" in lucene config");
						}

				}		/* if (lucene_config_p) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to find \"lucene\" in global config");
				}

			FreeMemory (tool_p);
		}		/* if (tool_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate memory for LuceneTool");
		}

	return NULL;
}


void FreeLuceneTool (LuceneTool *tool_p)
{
	if (tool_p -> lt_output_file_s)
		{
			FreeCopiedString (tool_p -> lt_output_file_s);
		}

	if (tool_p -> lt_name_s)
		{
			FreeCopiedString (tool_p -> lt_name_s);
		}

	FreeLinkedList (tool_p -> lt_facet_results_p);

	FreeMemory (tool_p);
}


bool SearchLucene (LuceneTool *tool_p, const char *query_s, LinkedList *facets_p, const char *search_type_s, const uint32 page_index, const uint32 page_size, const QueryMode qm)
{
	bool success_flag = false;
	ByteBuffer *buffer_p = AllocateByteBuffer (1024);

	if (buffer_p)
		{
			if (AppendStringsToByteBuffer (buffer_p, "java -classpath ", tool_p -> lt_classpath_s, " ", tool_p -> lt_search_class_s, " -index ", tool_p -> lt_index_s, " -tax ", tool_p -> lt_taxonomy_s,  NULL))
				{
					bool run_flag = true;

					if (facets_p)
						{
							KeyValuePairNode *node_p = (KeyValuePairNode *) (facets_p -> ll_head_p);

							while (run_flag && node_p)
								{
									KeyValuePair *pair_p = node_p -> kvpn_pair_p;

									if (!AppendStringsToByteBuffer (buffer_p, " -facet \"", pair_p -> kvp_key_s, ":", pair_p -> kvp_value_s, "\"", NULL))
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add facet pair \"%s\": \"%s\" to lucene tool buffer", pair_p -> kvp_key_s, pair_p -> kvp_value_s);
											run_flag = false;
										}

									node_p = (KeyValuePairNode *) (node_p -> kvpn_node.ln_next_p);
								}
						}

					/*
					 * Add Search type
					 */
					if (run_flag)
						{
							if (search_type_s)
								{
									if (!AppendStringsToByteBuffer (buffer_p, " -search_type ", search_type_s, NULL))
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add search_type \"%s\" to lucene tool buffer", search_type_s);
											run_flag = false;
										}
								}
						}


					if (run_flag)
						{
							if (qm == QM_TERMS)
								{
									if (!AppendStringToByteBuffer (buffer_p, " -terms_query "))
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add -terms_query to lucene tool buffer");
											run_flag = false;
										}
								}
						}


					if (run_flag)
						{
							const uint32 DEFAULT_PAGE_INDEX = 0;

							if (AddNumericArgument (buffer_p, "-page", page_index, DEFAULT_PAGE_INDEX))
								{
									const uint32 DEFAULT_PAGE_SIZE = 10;

									if (AddNumericArgument (buffer_p, "-page_size", page_size, DEFAULT_PAGE_SIZE))
										{
											/*
											 * Now add the output file
											 */
											char uuid_s [UUID_STRING_BUFFER_SIZE];
											char *full_filename_stem_s = NULL;
											char *file_s = uuid_s;

											ConvertUUIDToString (tool_p -> lt_id, uuid_s);

											if (tool_p -> lt_name_s)
												{
													if (! (file_s = ConcatenateVarargsStrings (uuid_s, "_", tool_p -> lt_name_s, NULL)))
														{
															PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to concatenate Varargs Strings \"%s\", \"_\", \"%s\"", uuid_s, "_", tool_p -> lt_name_s);
														}
												}

											if (file_s)
												{
													full_filename_stem_s = MakeFilename (tool_p -> lt_working_directory_s, file_s);

													if (full_filename_stem_s)
														{
															char *output_s = ConcatenateStrings (full_filename_stem_s, ".out");

															if (output_s)
																{
																	if (SetLuceneToolOutput (tool_p, output_s))
																		{
																			if (AppendStringsToByteBuffer (buffer_p, " -out ", output_s, " >> ", full_filename_stem_s, ".log", NULL))
																				{
																					if ((IsStringEmpty (query_s)) || AppendStringsToByteBuffer (buffer_p, " -query ", query_s, NULL))
																						{
																							const char *command_s = GetByteBufferData (buffer_p);
																							OperationStatus status = RunProcess (command_s);

																							if (status == OS_SUCCEEDED)
																								{
																									success_flag = true;
																									PrintLog (STM_LEVEL_FINE, __FILE__, __LINE__, "\"%s\" ran successfully", command_s);
																								}
																							else
																								{
																									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "\"%s\" failed with status %d", command_s, status);
																								}

																						}		/* if ((IsStringEmpty (query_s)) || AppendStringsToByteBuffer (buffer_p, " -query ", query_s, NULL)) */

																				}		/* if (AppendStringsToByteBuffer (buffer_p, " -out ", output_s, " >> ", full_filename_stem_s, ".log", NULL)) */

																		}		/* if (SetLuceneToolOutput (tool_p, output_s)) */

																	FreeCopiedString (output_s);
																}		/* if (output_s) */

															FreeCopiedString (full_filename_stem_s);
														}		/* if (full_filename_stem_s) */

													if (file_s != uuid_s)
														{
															FreeCopiedString (file_s);
														}

												}		/* if (file_s) */


										}		/* if (AddNumericArgument (buffer_p, "-page_size", page_size, DEFAULT_PAGE_SIZE)) */

								}		/* if (AddNumericArgument (buffer_p, "-page", page_index, DEFAULT_PAGE_INDEX)) */

						}		/* if (run_flag) */

				}		/* if (AppendStringsToByteBuffer (buffer_p, "java -classpath ", tool_p -> lt_classpath_s, " ", tool_p -> lt_search_class_s, " -index ", tool_p -> lt_index_s, " -tax ", tool_p -> lt_taxonomy_s, " -query ", query_s, NULL)) */

			FreeByteBuffer (buffer_p);
		}		/* if (buffer_p) */

	return success_flag;
}


OperationStatus DeleteLucene (LuceneTool *tool_p, const char *query_s, const QueryMode qm)
{
	OperationStatus status = OS_FAILED;
	ByteBuffer *buffer_p = AllocateByteBuffer (1024);

	if (buffer_p)
		{
			if (AppendStringsToByteBuffer (buffer_p, "java -classpath ", tool_p -> lt_classpath_s, " ", tool_p -> lt_delete_class_s, " -index ", tool_p -> lt_index_s, " -query ", query_s, NULL))
				{
					/*
					 * Now add the output file
					 */
					char uuid_s [UUID_STRING_BUFFER_SIZE];
					char *full_filename_stem_s = NULL;
					char *file_s = uuid_s;

					ConvertUUIDToString (tool_p -> lt_id, uuid_s);

					if (tool_p -> lt_name_s)
						{
							if (! (file_s = ConcatenateVarargsStrings (uuid_s, "_", tool_p -> lt_name_s, NULL)))
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to concatenate Varargs Strings \"%s\", \"_\", \"%s\"", uuid_s, "_", tool_p -> lt_name_s);
								}
						}

					if (file_s)
						{
							full_filename_stem_s = MakeFilename (tool_p -> lt_working_directory_s, file_s);

							if (full_filename_stem_s)
								{
									char *output_s = ConcatenateStrings (full_filename_stem_s, ".out");

									if (output_s)
										{
											if (SetLuceneToolOutput (tool_p, output_s))
												{
													if (AppendStringsToByteBuffer (buffer_p, " -out ", output_s, " >> ", full_filename_stem_s, ".log", NULL))
														{
															if ((IsStringEmpty (query_s)) || AppendStringsToByteBuffer (buffer_p, " -query ", query_s, NULL))
																{
																	const char *command_s = GetByteBufferData (buffer_p);
																	status = RunProcess (command_s);

																	if (status == OS_SUCCEEDED)
																		{
																		PrintLog (STM_LEVEL_FINE, __FILE__, __LINE__, "\"%s\" ran successfully", command_s);
																		}
																	else
																		{
																		PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "\"%s\" failed with status %d", command_s, status);
																		}


																}		/* if ((IsStringEmpty (query_s)) || AppendStringsToByteBuffer (buffer_p, " -query ", query_s, NULL)) */

														}		/* if (AppendStringsToByteBuffer (buffer_p, " -out ", output_s, " >> ", full_filename_stem_s, ".log", NULL)) */

												}		/* if (SetLuceneToolOutput (tool_p, output_s)) */

											FreeCopiedString (output_s);
										}		/* if (output_s) */

									FreeCopiedString (full_filename_stem_s);
								}		/* if (full_filename_stem_s) */

							if (file_s != uuid_s)
								{
									FreeCopiedString (file_s);
								}

						}		/* if (file_s) */

				}		/* if (AppendStringsToByteBuffer (buffer_p, "java -classpath ", tool_p -> lt_classpath_s, " ", tool_p -> lt_search_class_s, " -index ", tool_p -> lt_index_s, " -tax ", tool_p -> lt_taxonomy_s, " -query ", query_s, NULL)) */

			FreeByteBuffer (buffer_p);
		}		/* if (buffer_p) */

	return status;
}


OperationStatus IndexLucene (LuceneTool *tool_p, const json_t *data_p, bool update_flag)
{
	OperationStatus status = OS_FAILED;
	ByteBuffer *buffer_p = AllocateByteBuffer (1024);

	if (buffer_p)
		{
			if (AppendStringsToByteBuffer (buffer_p, "java -classpath ", tool_p -> lt_classpath_s, " ", tool_p -> lt_index_class_s, " -index ", tool_p -> lt_index_s, " -tax ", tool_p -> lt_taxonomy_s,  NULL))
				{
					bool run_flag = true;

					if (update_flag)
						{
							if (!AppendStringToByteBuffer (buffer_p, " -update"))
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, data_p, "Failed to add update flag to buffer");
									run_flag = false;
								}
						}

					if (run_flag)
						{
							char uuid_s [UUID_STRING_BUFFER_SIZE];
							char *full_filename_stem_s = NULL;

							char *file_s = uuid_s;

							ConvertUUIDToString (tool_p -> lt_id, uuid_s);

							if (tool_p -> lt_name_s)
								{
									if (! (file_s = ConcatenateVarargsStrings (uuid_s, "_", tool_p -> lt_name_s, NULL)))
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to concatenate Varargs Strings \"%s\", \"_\", \"%s\"", uuid_s, "_", tool_p -> lt_name_s);
										}
								}

							if (file_s)
								{
									full_filename_stem_s = MakeFilename (tool_p -> lt_working_directory_s, file_s);

									if (full_filename_stem_s)
										{
											char *input_s = ConcatenateStrings (full_filename_stem_s, ".json");

											if (input_s)
												{
													if (json_dump_file (data_p, input_s, JSON_INDENT (2)) == 0)
														{
															if (AppendStringsToByteBuffer (buffer_p, " -data ", input_s, NULL))
																{
																	char *output_s = ConcatenateStrings (full_filename_stem_s, ".out");

																	if (output_s)
																		{
																			char *error_s = ConcatenateStrings (full_filename_stem_s, ".err");

																			if (error_s)
																				{
																					char *results_s = ConcatenateStrings (full_filename_stem_s, ".results");

																					if (results_s)
																						{
																							if (AppendStringsToByteBuffer (buffer_p, " -out ", output_s, " -err ", error_s, " -results ", results_s, NULL))
																								{
																									const char *command_s = GetByteBufferData (buffer_p);
																									char *command_filename_s = ConcatenateStrings (full_filename_stem_s, ".command");
																									OperationStatus local_status;

																									if (command_filename_s)
																										{
																											FILE *command_f = fopen (command_filename_s, "w");

																											if (command_f)
																												{
																													fputs (command_s, command_f);
																													fclose (command_f);
																												}

																											FreeCopiedString (command_filename_s);
																										}

																									SetLuceneToolOutput (tool_p, output_s);


																									local_status = RunProcess (command_s);

																									if (local_status == OS_SUCCEEDED)
																										{
																											/*
																												* Do we have a results file?
																												*/
																											json_error_t err;
																											json_t *results_p = json_load_file (results_s, 0, &err);

																											if (results_p)
																												{
																													json_int_t successes;

																													if (GetJSONInteger (results_p, "successes", &successes))
																														{
																															json_int_t total;

																															if (GetJSONInteger (results_p, "successes", &total))
																																{
																																	if (total != 0)
																																		{
																																			if (successes == total)
																																				{
																																					status = OS_SUCCEEDED;
																																				}
																																		}
																																	else
																																		{
																																			if (total > 0)
																																				{
																																					status = OS_PARTIALLY_SUCCEEDED;
																																				}
																																		}
																																}		/* if (GetJSONInteger (results_p, "successes", &successes)) */

																														}		/* if (GetJSONInteger (results_p, "successes", &successes)) */

																													json_decref (results_p);
																												}		/* if (results_p) */


																											if (status != OS_SUCCEEDED)
																												{
																													/*
																														* check error file
																														*/
																													FileInformation info;

																													InitFileInformation (&info);

																													if (CalculateFileInformation (error_s, &info))
																														{
																															if (info.fi_size == 0)
																																{
																																	PrintLog (STM_LEVEL_FINE, __FILE__, __LINE__, "\"%s\" ran successfully", command_s);
																																}
																															else
																																{
																																	char *error_details_s = GetFileContentsAsStringByFilename (error_s);

																																	if (error_details_s)
																																		{
																																			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "\"%s\" had errors whilst running:\n%s", command_s, error_details_s);

																																			FreeCopiedString (error_details_s);
																																		}
																																	else
																																		{
																																			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "\"%s\" had errors whilst running contained in \"%s\"", command_s, error_s);
																																		}
																																}

																														}		/* if (CalculateFileInformation (error_s, &info)) */
																													else
																														{
																															PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "CalculateFileInformation failed for \"%s\"", error_s);
																														}
																												}
																										}
																									else
																										{
																											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed running \"%s\" with return code %d", command_s, local_status);
																										}

																								}		/* if (AppendStringsToByteBuffer (buffer_p, " -out ", output_s, " >> ", full_filename_stem_s, ".log", NULL)) */
																							else
																								{
																									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, data_p, "Failed to append output and log params for \"%s\"", full_filename_stem_s);
																								}																							FreeCopiedString (results_s);

																						}
																					else
																						{
																							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, data_p, "Failed to create results filename for stem \"%s\"", full_filename_stem_s);
																						}


																					FreeCopiedString (error_s);
																				}		/* if (error_s) */
																			else
																				{
																					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, data_p, "Failed to create error filename for stem \"%s\"", full_filename_stem_s);
																				}


																			FreeCopiedString (output_s);
																		}		/* if (output_s) */
																	else
																		{
																			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, data_p, "Failed to create output filename for stem \"%s\"", full_filename_stem_s);
																		}

																}		/* if (AppendStringsToByteBuffer (buffer_p, " -data ", input_s, NULL)) */
															else
																{
																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, data_p, "Failed to add data parameter \"%s\"", input_s);
																}

														}		/* if (json_dump_file (data_p, input_s, JSON_INDENT (2)) == 0) */
													else
														{
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, data_p, "Failed to write data to index to \"%s\"", input_s);
														}

													FreeCopiedString (input_s);
												}		/* if (input_s) */
											else
												{
													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, data_p, "Failed to create input filename for stem \"%s\"", full_filename_stem_s);
												}

											FreeCopiedString (full_filename_stem_s);
										}		/* if (full_filename_stem_s) */
									else
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, data_p, "Failed to make full filename stem for \"%s\" and \"%s\"", tool_p -> lt_working_directory_s, uuid_s);
										}


									if (file_s != uuid_s)
										{
											FreeCopiedString (file_s);
										}

								}		/* if (file_s) */

						}		/* iflfn_node (run_flag) */

				}		/* if (AppendStringsToByteBuffer (buffer_p, "java -classpath ", tool_p -> lt_classpath_s, " ", tool_p -> lt_index_s, " -index ", tool_p -> lt_index_s, " -tax ", tool_p -> lt_taxonomy_s,  NULL)) */
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, data_p, "Failed to add command params");
				}

			FreeByteBuffer (buffer_p);
		}		/* if (buffer_p) */
	else
		{
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, data_p, "Failed to allocate ByteBuffer");
		}

	return status;
}


bool SetLuceneToolOutput (LuceneTool *tool_p, char *output_s)
{
	return ReplaceValidString (output_s, & (tool_p -> lt_output_file_s));
}


bool SetLuceneToolName (LuceneTool *tool_p, const char *name_s)
{
	return ReplaceValidString (name_s, & (tool_p -> lt_name_s));
}


void SetLuceneToolId (LuceneTool *tool_p, uuid_t id)
{
	uuid_copy (tool_p -> lt_id, id);
}



static bool ReplaceValidString (const char *src_s, char **dest_ss)
{
	bool success_flag = true;
	char *new_value_s = NULL;

	if (src_s)
		{
			new_value_s = EasyCopyToNewString (src_s);
			success_flag = (new_value_s != NULL);
		}

	if (success_flag)
		{
			if (*dest_ss)
				{
					FreeCopiedString (*dest_ss);
				}

			*dest_ss = new_value_s;
		}

	return success_flag;
}


OperationStatus ParseLuceneResults (LuceneTool *tool_p, const uint32 from, const uint32 to, bool (*lucene_results_callback_fn) (const json_t *document_p, const uint32 index, void *data_p), void *data_p)
{
	OperationStatus status = OS_FAILED;

	if (tool_p -> lt_output_file_s)
		{
			json_error_t err;
			json_t *results_p = json_load_file (tool_p -> lt_output_file_s, 0, &err);

			if (results_p)
				{

					json_t *docs_p = json_object_get (results_p, "documents");

					if (docs_p)
						{
							uint32 value;

							if (json_is_array (docs_p))
								{
									const size_t num_docs = json_array_size (docs_p);
									size_t i = 0;
									size_t num_successes = 0;

									while (i < num_docs)
										{
											const json_t *doc_p = json_array_get (docs_p, i);

											if (lucene_results_callback_fn (doc_p, i, data_p))
												{
													++ num_successes;
												}
											else
												{
													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, doc_p, "Lucene callback conversion function failed");
												}

											++ i;
										}		/* while (loop_flag && success_flag) */

									if (num_successes == num_docs)
										{
											status = OS_SUCCEEDED;
										}
									else if (num_successes > 0)
										{
											status = OS_PARTIALLY_SUCCEEDED;
										}

								}		/* if (json_is_array (docs_p)) */

							if (GetJSONUnsignedInteger (results_p, "total_hits", &value))
								{
									tool_p -> lt_num_total_hits = value;
								}
							else
								{
									PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, results_p, "Failed to get total number of hits");
								}

							if (GetJSONUnsignedInteger (results_p, "from", &value))
								{
									tool_p -> lt_hits_from_index = value;
								}
							else
								{
									PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, results_p, "Failed to get hits start index");
								}

							if (GetJSONUnsignedInteger (results_p, "to", &value))
								{
									tool_p -> lt_hits_to_index = value;
								}
							else
								{
									PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, results_p, "Failed to get hits end index");
								}

							ParseFacetResults (tool_p, results_p);
						}		/* if (docs_p) */

					json_decref (results_p);
				}		/* if (results_p) */

		}		/* if (tool_p -> lt_output_file_s) */


	return status;
}


bool AddLuceneFacetResultsToJSON (LuceneTool *tool_p, json_t *metadata_p)
{
	bool success_flag = false;
	LuceneFacetNode *node_p = (LuceneFacetNode *) (tool_p -> lt_facet_results_p -> ll_head_p);

	if (node_p)
		{
			json_t *facets_array_p = json_array ();

			if (facets_array_p)
				{
					if (json_object_set_new (metadata_p, "facets", facets_array_p) == 0)
						{
							while (node_p)
								{
									LuceneFacet *facet_p = node_p -> lfn_facet_p;
									json_t *facet_json_p = GetLuceneFacetAsJSON (facet_p);

									if (facet_json_p)
										{
											if (json_array_append_new (facets_array_p, facet_json_p) != 0)
												{
													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, facet_json_p, "Failed to add facets JSON to array");
													json_decref (facet_json_p);
												}		/*  if (json_array_append_new (facets_array_p, facet_json_p) != 0) */

										}		/* if (facet_json_p) */
									else
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "GetLuceneFacetAsJSON failed for \"%s\": " UINT32_FMT, facet_p -> lf_name_s, facet_p -> lf_count);
										}

									node_p = (LuceneFacetNode *) (node_p -> lfn_node.ln_next_p);
								}		/* while (node_p) */

							if (json_array_size (facets_array_p) == tool_p -> lt_facet_results_p -> ll_size)
								{
									success_flag = true;
								}

						}		/* if (json_object_set_new (metadata_p, "facets", facets_array_p) == 0) */
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, metadata_p, "Failed to add facets array to metadata JSON");
							json_decref (facets_array_p);
						}

				}		/* if (facets_array_p) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate facets array as JSON");
				}

		}		/* if (node_p) */
	else
		{
			success_flag = true;
		}

	return success_flag;
}


static bool LoadDocument (const json_t *result_p, LuceneDocument *document_p)
{
	bool success_flag = false;
	void *itr_p = json_object_iter (result_p);

	if (itr_p)
		{
			success_flag = true;

			while (itr_p && success_flag)
				{
					const char *key_s = json_object_iter_key (itr_p);
					json_t *value_p = json_object_iter_value (itr_p);

					if (json_is_string (value_p))
						{
							const char *value_s = json_string_value (value_p);

							if (!AddFieldToLuceneDocument (document_p, key_s, value_s))
								{
									success_flag = false;
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add string value \"%s\": \"%s\" to document", key_s, value_s);
								}

						}		/* if (json_is_string (value_p)) */
					else if ((json_is_object (value_p)) || (json_is_array (value_p)))
						{
							char *value_s = json_dumps (value_p, 0);

							if (value_s)
								{
									if (!AddFieldToLuceneDocument (document_p, key_s, value_s))
										{
											success_flag = false;
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add object/array value \"%s\": \"%s\" to document", key_s, value_s);
										}

									free (value_s);
								}
							else
								{
									success_flag = false;
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "json_dumps () failed, key \"%s\"", key_s);
								}
						}
					else if (json_is_boolean (value_p))
						{
							bool value = json_boolean_value (value_p);

							if (!AddFieldToLuceneDocument (document_p, key_s, value ? "true" : "false"))
								{
									success_flag = false;
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add boolean value \"%s\": \"%s\" to document", key_s, value ? "true" : "false");
								}
						}
					else if (json_is_integer (value_p))
						{
							json_int_t value = json_integer_value (value_p);

							char *value_s = ConvertIntegerToString (value);

							if (value_s)
								{
									if (!AddFieldToLuceneDocument (document_p, key_s, value_s))
										{
											success_flag = false;
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add integer value \"%s\": \"%s\" to document", key_s, value_s);
										}

									FreeCopiedString (value_s);
								}
							else
								{
									success_flag = false;
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "ConvertIntegerToString failed  for \"%d\", key \"%s\"", value, key_s);
								}

						}
					else if (json_is_number (value_p))
						{
							double value = json_number_value (value_p);
							char *value_s = ConvertDoubleToString (value);

							if (value_s)
								{
									if (!AddFieldToLuceneDocument (document_p, key_s, value_s))
										{
											success_flag = false;
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add double value \"%s\": \"%s\" to document", key_s, value_s);
										}

									FreeCopiedString (value_s);
								}
							else
								{
									success_flag = false;
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "ConvertDoubleToString failed for \"%lf\", key \"%s\"", value, key_s);
								}


						}

					itr_p = json_object_iter_next (result_p, itr_p);
				}		/* while (itr_p) */

		}		/* if (itr_p) */

	return success_flag;
}



static bool AddNumericArgument (ByteBuffer *buffer_p, const char * const key_s, const uint32 value, const uint32 default_value)
{
	bool success_flag = false;

	if (value != default_value)
		{
			char *value_s = ConvertUnsignedIntegerToString (value);

			if (value_s)
				{
					if (AppendStringsToByteBuffer (buffer_p, " ", key_s, " ", value_s, NULL))
						{
							success_flag = true;
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to append %s %s to buffer", key_s, value_s);
						}

					FreeCopiedString (value_s);
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to convert " UINT32_FMT " to string for key \"%s\"", value, key_s);
				}
		}
	else
		{
			success_flag = true;
		}

	return success_flag;
}


static bool ParseFacetResults (LuceneTool *tool_p, const json_t *results_p)
{
	bool success_flag = false;
	const json_t *facets_json_p = json_object_get (results_p, "facets");

	if (facets_json_p)
		{
			if (json_is_array (facets_json_p))
				{
					if (json_array_size (facets_json_p) > 0)
						{
							const json_t *entry_p = json_array_get (facets_json_p, 0);
							const json_t *labels_json_p = json_object_get (entry_p, "labelValues");

							if (labels_json_p)
								{
									if (json_is_array (labels_json_p))
										{
											const size_t num_facets = json_array_size (labels_json_p);
											size_t i;

											for (i = 0; i < num_facets; ++ i)
												{
													const json_t *label_json_p = json_array_get (labels_json_p, i);
													LuceneFacet *facet_p = GetLuceneFacetFromResultsJSON (label_json_p);

													if (facet_p)
														{
															LuceneFacetNode *node_p = AllocateLuceneFacetNode (facet_p);

															if (node_p)
																{
																	LinkedListAddTail (tool_p -> lt_facet_results_p, & (node_p -> lfn_node));
																}
															else
																{
																	FreeLuceneFacet (facet_p);
																}

														}		/* if (facet_p) */
													else
														{
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, label_json_p, "failed to allocate LuceneFacet");
														}

												}		/* for (i = 0; i < num_facets; ++ i) */


											if (tool_p -> lt_facet_results_p -> ll_size == num_facets)
												{
													success_flag = true;
												}

										}		/* if (json_is_array (labels_json_p)) */
									else
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, entry_p, "\"labelValues\" is not an array");
										}

								}		/* if (labels_json_p) */
							else
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, entry_p, "Failed to get \"labelValues\" from json");
								}

						}		/* if (json_array_size (facets_json_p) > 0) */
					else
						{
							PrintJSONToLog (STM_LEVEL_SEVERE, __FILE__, __LINE__, results_p, "\"facets\" is an empty array");
						}

				}		/* if (json_is_array (facets_json_p)) */
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, results_p, "\"facets\" is not an array");
				}
		}
	else
		{
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, results_p, "Failed to get \"facets\" from json");
		}

	return success_flag;
}



bool AddFacetResultToLucene (LuceneTool *tool_p, const char *name_s, const uint32 count)
{
	LuceneFacet *facet_p = AllocateLuceneFacet (name_s, count);

	if (facet_p)
		{
			LuceneFacetNode *node_p = AllocateLuceneFacetNode (facet_p);

			if (node_p)
				{
					LinkedListAddTail (tool_p -> lt_facet_results_p, & (node_p -> lfn_node));
					return true;
				}

			FreeLuceneFacet (facet_p);
		}		/* if (facet_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate LuceneFacet for \"%s\": " UINT32_FMT, name_s, count);
		}

	return false;
}
