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

#include "lucene_tool.h"
#include "memory_allocations.h"
#include "string_utils.h"
#include "grassroots_config.h"
#include "byte_buffer.h"
#include "json_util.h"
#include "key_value_pair.h"
#include "string_utils.h"
#include "filesystem_utils.h"


static bool LoadDocument (FILE *results_f, LuceneDocument *document_p);

static bool GetTokens (char *buffer_s, char **key_ss, char **value_ss);



LuceneTool *AllocateLuceneTool (uuid_t id)
{
	LuceneTool *tool_p = (LuceneTool *) AllocMemory (sizeof (LuceneTool));

	if (tool_p)
		{
			const json_t *lucene_config_p = GetGlobalConfigValue ("lucene");

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
															tool_p -> lt_search_class_s = search_class_s;
															tool_p -> lt_classpath_s = classpath_s;
															tool_p -> lt_index_s = index_s;
															tool_p -> lt_taxonomy_s = taxonomy_s;
															tool_p -> lt_working_directory_s = working_directory_s;
															tool_p -> lt_output_file_s = NULL;
															tool_p -> lt_id = id;

															return tool_p;
														}
												}		/* if (search_class_s) */

										}		/* if (taxonomy_s) */

								}		/* if (index_s) */

						}		/* if (classpath_s) */

				}		/* if (lucene_config_p) */

			FreeMemory (tool_p);
		}		/* if (tool_p) */

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


bool RunLuceneTool (LuceneTool *tool_p, const char *query_s, LinkedList *facets_p)
{
	bool success_flag = false;
	ByteBuffer *buffer_p = AllocateByteBuffer (1024);

	if (buffer_p)
		{
			if (AppendStringsToByteBuffer (buffer_p, "java -classpath ", tool_p -> lt_classpath_s, " ", tool_p -> lt_search_class_s, " -index ", tool_p -> lt_index_s, " -tax ", tool_p -> lt_taxonomy_s, " -query ", query_s, NULL))
				{
					bool run_flag = true;

					if (facets_p)
						{
							KeyValuePairNode *node_p = (KeyValuePairNode *) (facets_p -> ll_head_p);

							while (run_flag && node_p)
								{
									node_p = (KeyValuePairNode *) (node_p -> kvpn_node.ln_next_p);
								}
						}

					if (run_flag)
						{
							/*
							 * Now add the output file
							 */
							char uuid_s [UUID_STRING_BUFFER_SIZE];
							char *full_filename_stem_s = NULL;

							ConvertUUIDToString (* (tool_p -> lt_id_p), uuid_s);

							full_filename_stem_s = MakeFilename (tool_p -> lt_working_directory_s, uuid_s);

							if (full_filename_stem_s)
								{
									char *output_s = ConcatenateStrings (full_filename_stem_s, ".out");

									if (output_s)
										{
											SetLuceneToolOutput (tool_p, output_s);

											if (AppendStringsToByteBuffer (buffer_p, " -out ", output_s, " >> ", full_filename_stem_s, ".log", NULL))
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

										}

									FreeCopiedString (full_filename_stem_s);
								}

						}		/* if (run_flag) */

				}

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


bool ParseLuceneResults (LuceneTool *tool_p, bool (*lucene_results_callback_fn) (LuceneDocument *document_p, const uint32 index))
{
	bool success_flag = false;

	if (tool_p -> lt_output_file_s)
		{
			FILE *results_f = fopen (tool_p -> lt_output_file_s, "r");

			if (results_f)
				{
					LuceneDocument *document_p = AllocateLuceneDocument ();

					if (document_p)
						{
							bool loop_flag = true;
							uint32 i = 0;

							while (loop_flag)
								{
									if (LoadDocument (results_f, document_p))
										{
											if (!lucene_results_callback_fn (document_p, i))
												{
													loop_flag = false;
												}
										}
									else
										{
											loop_flag = false;
										}

									++ i;
									ClearLuceneDocument (document_p);
								}

							FreeLuceneDocument (document_p);
						}

					fclose (results_f);
				}		/* if (results_f) */


		}		/* if (tool_p -> lt_output_file_s) */


	return success_flag;
}



static bool LoadDocument (FILE *results_f, LuceneDocument *document_p)
{
	bool success_flag = false;
	char *buffer_s = NULL;
	bool loop_flag = true;
	bool found_flag = false;

	/*
	 * Scroll to start of document
	 */
	while (loop_flag)
		{
			if (GetLineFromFile (results_f, &buffer_s))
				{
					if (!IsStringEmpty (buffer_s))
						{
							if (strcmp (buffer_s, "{") == 0)
								{
									found_flag = true;
									loop_flag = false;
								}		/* if (strcmp (buffer_s, "{") == 0) */

						}		/* if (!IsStringEmpty (buffer_s)) */

					FreeCopiedString (buffer_s);
				}		/* if (GetLineFromFile (results_f, buffer_s)) */
			else
				{
					loop_flag = false;
				}

		}		/* while (loop_flag) */


	if (found_flag)
		{
			loop_flag = true;

			/*
			 * Read in the document
			 */
			while (loop_flag)
				{
					if (GetLineFromFile (results_f, &buffer_s))
						{
							if (!IsStringEmpty (buffer_s))
								{
									if (strcmp (buffer_s, "}") != 0)
										{
											char *key_s = NULL;
											char *value_s = NULL;

											if (GetTokens (buffer_s, &key_s, &value_s))
												{
													if (!AddFieldToLuceneDocument (document_p, key_s, value_s))
														{

														}
												}
											else
												{
													loop_flag = false;
												}

										}		/* if (strcmp (buffer_s, "{") == 0) */
									else
										{
											loop_flag = false;
											success_flag = true;
										}
								}		/* if (!IsStringEmpty (buffer_s)) */

							FreeCopiedString (buffer_s);
						}		/* if (GetLineFromFile (results_f, buffer_s)) */
					else
						{
							loop_flag = false;
						}

				}		/* while (loop_flag) */

		}		/* if (found_flag) */


	return success_flag;

}


static bool GetTokens (char *buffer_s, char **key_ss, char **value_ss)
{
	bool success_flag = false;

	char *equals_p = strchr (buffer_s, '=');

	if (equals_p)
		{
			char *key_s = NULL;

			*equals_p = '\0';

			if ((key_s = CopyToNewString (buffer_s, 0, true)) != NULL)
				{
					char *value_s = NULL;

					if ((value_s = CopyToNewString (equals_p  + 1, 0, true)) != NULL)
						{
							*key_ss = key_s;
							*value_ss = value_s;

							success_flag = true;
						}		/* if ((value_s = CopyToNewString (equals_p  + 1, 0, true)) != NULL) */
					else
						{
							FreeCopiedString (key_s);
						}

				}		/* if ((key_s = CopyToNewString (buffer_s, 0, true)) != NULL) */

			*equals_p = '=';
		}

	return success_flag;
}

