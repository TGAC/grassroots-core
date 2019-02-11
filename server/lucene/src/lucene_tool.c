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


static bool LoadDocument (FILE *results_f, HashTable *doc_p);


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


bool ParseLuceneResults (LuceneTool *tool_p, bool (*lucene_results_callback_fn) (const HashTable * const document_p, const uint32 index))
{
	bool success_flag = false;

	if (tool_p -> lt_output_file_s)
		{
			FILE *results_f = fopen (tool_p -> lt_output_file_s, "r");

			if (results_f)
				{

					fclose (results_f);
				}		/* if (results_f) */


		}		/* if (tool_p -> lt_output_file_s) */


	return success_flag;
}



static bool LoadDocument (FILE *results_f, HashTable *doc_p)
{
	bool success_flag = false;

	return success_flag;

}
