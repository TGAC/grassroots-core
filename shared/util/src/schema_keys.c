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

/**
 * schema_keys.c
 *
 *  Created on: 4 Mar 2017
 *      Author: billy
 *
 * @file
 * @brief
 */

#include <string.h>

#define ALLOCATE_SCHEMA_KEYS_TAGS
#include "schema_keys.h"

#include "string_utils.h"
#include "streams.h"



static char *ExpandTerm (const char *term_s, const char *prefix_s, const char *expanded_prefix_s);


char *GetExpandedContextTerm (const char *term_s)
{
	char *result_s = NULL;

	if (term_s)
		{
			if ((result_s = ExpandTerm (term_s, CONTEXT_PREFIX_SCHEMA_ORG_S, CONTEXT_URL_SCHEMA_ORG_S)) == NULL)
				{
					if ((result_s = ExpandTerm (term_s, CONTEXT_PREFIX_EDAM_ONTOLOGY_S, CONTEXT_URL_EDAM_ONOTOLOGY_S)) == NULL)
						{
							if ((result_s = ExpandTerm (term_s, CONTEXT_PREFIX_EXPERIMENTAL_FACTOR_ONTOLOGY_S, CONTEXT_URL_EXPERIMENTAL_FACTOR_ONOTOLOGY_S)) == NULL)
								{
									if ((result_s = ExpandTerm (term_s, CONTEXT_PREFIX_SOFTWARE_ONTOLOGY_S, CONTEXT_URL_SOFTWARE_ONOTOLOGY_S)) == NULL)
										{

										}

								}

						}

				}

		}

	return result_s;
}


static char *ExpandTerm (const char *term_s, const char *prefix_s, const char *expanded_prefix_s)
{
	char *result_s = NULL;
	const size_t l  = strlen (prefix_s);

	if (strncmp (term_s, prefix_s, l) == 0)
		{
			term_s += l;

			result_s = ConcatenateStrings (expanded_prefix_s, term_s);

			if (!result_s)
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to expand term \%s\" using \"%s\" and \"%s\"", term_s, prefix_s, expanded_prefix_s);
				}
		}

	return result_s;
}



