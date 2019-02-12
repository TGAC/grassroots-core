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


static bool LoadDocument (FILE *results_f, LuceneDocument *document_p);

static bool GetTokens (char *buffer_s, char **key_ss, char **value_ss);



LuceneTool *AllocateLuceneTool (void)
{
	LuceneTool *tool_p = (LuceneTool *) AllocMemory (sizeof (LuceneTool));

	if (tool_p)
		{

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

	return success_flag;
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

