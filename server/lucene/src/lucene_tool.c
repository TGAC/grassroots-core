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


static bool LoadDocument (const json_t *result_p, LuceneDocument *document_p);

static bool AddNumericArgument (ByteBuffer *buffer_p, const char * const key_s, const uint32 value, const uint32 default_value);


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
													const char *working_directory_s = GetJSONString (lucene_config_p, "working_directory");

													if (working_directory_s)
														{
															const char *facet_key_s = GetJSONString (lucene_config_p, "facet_key");

															if (facet_key_s)
																{
																	tool_p -> lt_search_class_s = search_class_s;
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

																	return tool_p;
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

	FreeMemory (tool_p);
}


bool RunLuceneTool (LuceneTool *tool_p, const char *query_s, LinkedList *facets_p, const char *search_type_s, const uint32 page_index, const uint32 page_size)
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

									if (!AppendStringsToByteBuffer (buffer_p, " -facet_name \"", pair_p -> kvp_key_s, "\" -facet_value \"", pair_p -> kvp_value_s, "\"", NULL))
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add facet pair \"%s\": \"%s\" to lucene tool buffer", pair_p -> kvp_key_s, pair_p -> kvp_value_s);
											run_flag = false;
										}

									node_p = (KeyValuePairNode *) (node_p -> kvpn_node.ln_next_p);
								}
						}

					/*
					 * Add any facets
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

											ConvertUUIDToString (tool_p -> lt_id, uuid_s);

											full_filename_stem_s = MakeFilename (tool_p -> lt_working_directory_s, uuid_s);

											if (full_filename_stem_s)
												{
													char *output_s = ConcatenateStrings (full_filename_stem_s, ".out");

													if (output_s)
														{
															SetLuceneToolOutput (tool_p, output_s);

															if (AppendStringsToByteBuffer (buffer_p, " -out ", output_s, " -query ", query_s, " >> ", full_filename_stem_s, ".log", NULL))
																{
																	const char *command_s = GetByteBufferData (buffer_p);
																	int res = system (command_s);

																	if (res != -1)
																		{
																			int process_exit_code = WEXITSTATUS (res);

																			if (process_exit_code == 0)
																				{
																					success_flag = true;
																					PrintLog (STM_LEVEL_FINE, __FILE__, __LINE__, "\"%s\" ran successfully", command_s);
																				}
																			else
																				{
																					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "\"%s\" failed with return code %d", command_s, process_exit_code);
																				}
																		}
																	else
																		{
																			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed running \"%s\" with return code %d", command_s, res);
																		}

																}		/* if (output_s) */

														}		/* if (output_s) */

													FreeCopiedString (full_filename_stem_s);
												}		/* if (full_filename_stem_s) */

										}		/* if (AddNumericArgument (buffer_p, "-page_size", page_size, DEFAULT_PAGE_SIZE)) */

								}		/* if (AddNumericArgument (buffer_p, "-page", page_index, DEFAULT_PAGE_INDEX)) */

						}		/* if (run_flag) */

				}		/* if (AppendStringsToByteBuffer (buffer_p, "java -classpath ", tool_p -> lt_classpath_s, " ", tool_p -> lt_search_class_s, " -index ", tool_p -> lt_index_s, " -tax ", tool_p -> lt_taxonomy_s, " -query ", query_s, NULL)) */

			FreeByteBuffer (buffer_p);
		}		/* if (buffer_p) */

	return success_flag;
}



void SetLuceneToolOutput (LuceneTool *tool_p, char *output_s)
{
	if (tool_p -> lt_output_file_s)
		{
			FreeCopiedString (tool_p -> lt_output_file_s);
		}

	tool_p -> lt_output_file_s = output_s;
}


bool ParseLuceneResults (LuceneTool *tool_p, const uint32 from, const uint32 to, bool (*lucene_results_callback_fn) (LuceneDocument *document_p, const uint32 index, void *data_p), void *data_p)
{
	bool success_flag = false;

	if (tool_p -> lt_output_file_s)
		{
			json_error_t err;
			json_t *results_p = json_load_file (tool_p -> lt_output_file_s, 0, &err);

			if (results_p)
				{
					LuceneDocument *document_p = AllocateLuceneDocument ();

					if (document_p)
						{

							json_t *docs_p = json_object_get (results_p, "documents");

							if (docs_p)
								{
									int value;

									if (json_is_array (docs_p))
										{
											const size_t num_docs = json_array_size (docs_p);
											size_t i = 0;

											success_flag = true;

											while ((i < num_docs) && success_flag)
												{
													const json_t *result_p = json_array_get (docs_p, i);

													if (LoadDocument (result_p, document_p))
														{
															if (!lucene_results_callback_fn (document_p, i, data_p))
																{
																	success_flag = false;
																}

														}		/* if (LoadDocument (result_p, document_p)) */
													else
														{
															success_flag = false;
														}

													++ i;
													ClearLuceneDocument (document_p);
												}		/* while (loop_flag && success_flag) */

										}		/* if (json_is_array (docs_p)) */



									if (GetJSONInteger (results_p, "total_hits", &value))
										{
											tool_p -> lt_num_total_hits = value;
										}
									else
										{
											PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, results_p, "Failed to get total number of hits");
										}

									if (GetJSONInteger (results_p, "from", &value))
										{
											tool_p -> lt_hits_from_index = value;
										}
									else
										{
											PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, results_p, "Failed to get hits start index");
										}

									if (GetJSONInteger (results_p, "to", &value))
										{
											tool_p -> lt_hits_to_index = value;
										}
									else
										{
											PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, results_p, "Failed to get hits end index");
										}


								}		/* if (docs_p) */

							FreeLuceneDocument (document_p);
						}		/* if (document_p) */

					json_decref (results_p);
				}		/* if (results_p) */

		}		/* if (tool_p -> lt_output_file_s) */


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
								}

						}		/* if (json_is_string (value_p)) */

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

