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
 * regular_expressions.c
 *
 *  Created on: 25 Jan 2017
 *      Author: billy
 *
 * @file
 * @brief
 */

#include <string.h>

#include "regular_expressions.h"
#include "streams.h"
#include "memory_allocations.h"
#include "string_utils.h"


RegExp *AllocateRegExp (uint32 num_vectors)
{
	PCRE2_SIZE *vectors_p = (PCRE2_SIZE *) AllocMemory (num_vectors * sizeof (PCRE2_SIZE));

	if (vectors_p)
		{
			RegExp *reg_exp_p = (RegExp *) AllocMemory (sizeof (RegExp));

			if (reg_exp_p)
				{
					reg_exp_p -> re_compiled_expression_p = NULL;
					reg_exp_p -> re_num_matches = -1;
					reg_exp_p -> re_substring_vectors_p = vectors_p;
					reg_exp_p -> re_num_vectors = num_vectors;
					reg_exp_p -> re_current_substring_index = -1;

					return reg_exp_p;
				}

			FreeMemory (vectors_p);
		}

	return NULL;
}


void FreeRegExp (RegExp *reg_ex_p)
{
	ClearRegExp (reg_ex_p);
	FreeMemory (reg_ex_p);
}


void ClearRegExp (RegExp *reg_ex_p)
{
	if (reg_ex_p -> re_compiled_expression_p)
		{
			pcre2_code_free (reg_ex_p -> re_compiled_expression_p);
			reg_ex_p -> re_compiled_expression_p = NULL;
		}

	if (reg_ex_p -> re_match_data_p)
		{
			pcre2_match_data_free (reg_ex_p -> re_match_data_p);
		}

	reg_ex_p -> re_num_matches =  0;
	reg_ex_p -> re_current_substring_index = 0;
	memset (reg_ex_p -> re_substring_vectors_p, 0, (reg_ex_p -> re_num_vectors) * sizeof (int));

	reg_ex_p -> re_target_s = NULL;
}


bool SetPattern (RegExp *reg_ex_p, const unsigned char *pattern_s, uint32 options)
{
	bool success_flag = false;
	int error = -1;
	PCRE2_SIZE offset = -1;

	reg_ex_p -> re_compiled_expression_p = pcre2_compile (pattern_s, PCRE2_ZERO_TERMINATED, options, &error, &offset, NULL);

	if (reg_ex_p -> re_compiled_expression_p)
		{
			reg_ex_p -> re_match_data_p = pcre2_match_data_create_from_pattern (reg_ex_p -> re_compiled_expression_p, NULL);

			if (reg_ex_p -> re_match_data_p)
				{
					success_flag = true;
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get match data for regular expression for \"%s\"", pattern_s);
				}
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to compile regular expression for \"%s\"", pattern_s);
		}

	return success_flag;
}



bool MatchPattern (RegExp *reg_ex_p, const char *value_s)
{
	bool success_flag = false;

	if (reg_ex_p -> re_compiled_expression_p)
		{
			int res = pcre2_match (reg_ex_p -> re_compiled_expression_p, value_s, strlen (value_s), 0, 0, reg_ex_p -> re_match_data_p, NULL);

			if (res > 0)
				{
					reg_ex_p -> re_substring_vectors_p = pcre2_get_ovector_pointer (reg_ex_p -> re_match_data_p);
					reg_ex_p -> re_num_matches = res - 1;
					success_flag = true;
				}
			else if (res == 0)
				{
					/*
					 * There are too many substrings to fit in vectors_p
					 * so use the maximum possible
					 */
					reg_ex_p -> re_num_matches =  (reg_ex_p -> re_num_vectors) / 3;
					success_flag = true;
				}
			else
				{
					reg_ex_p -> re_num_matches  = 0;

					switch (res)
						{
							case PCRE2_ERROR_NOMATCH:
								PrintLog (STM_LEVEL_FINER, __FILE__, __LINE__, "No match for \"%s\"", value_s);
								break;

							case PCRE2_ERROR_NULL:
								PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "An object was NULL");
								break;

							case PCRE2_ERROR_NOMEMORY:
								PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Ran out of memory during regular expression matching");
								break;

							case PCRE2_ERROR_BADOPTION:
								PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "An incorrect option was passed to regular expression matching");
								break;

							case PCRE2_ERROR_BADMAGIC:
								PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "An bad magic error occurred during regular expression matching");
								break;

							default:
								PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "An unknown error 5d occurred during regular expression matching", res);
								break;
						}		/* switch (res) */

				}

			if (success_flag)
				{
					reg_ex_p -> re_target_s = value_s;
				}

			reg_ex_p -> re_current_substring_index = 0;

		}		/* if (reg_ex_p -> re_compiled_expression_p) */

	return success_flag;
}


uint32 GetNumberOfMatches (const RegExp *reg_ex_p)
{
	return reg_ex_p -> re_num_matches;
}



/*
 * Show substrings stored in the output vector by number. Obviously, in a real
	application you might want to do things other than print them.

for (i = 0; i < rc; i++)
  {
  PCRE2_SPTR substring_start = subject + ovector[2*i];
  PCRE2_SIZE substring_length = ovector[2*i+1] - ovector[2*i];
  printf("%2d: %.*s\n", i, (int)substring_length, (char *)substring_start);
  }

 */

char *GetNextMatch (RegExp *reg_ex_p)
{
	char *value_s = NULL;

	if (reg_ex_p -> re_current_substring_index < ((reg_ex_p -> re_num_matches) << 1))
		{
			const PCRE2_SIZE start = * ((reg_ex_p -> re_substring_vectors_p) + (reg_ex_p -> re_current_substring_index));
		  PCRE2_SIZE substring_length = (* ((reg_ex_p -> re_substring_vectors_p) + 1)) - start;

			value_s = CopyToNewString ((reg_ex_p -> re_target_s) + start, substring_length, false);

			if (value_s)
				{
					reg_ex_p -> re_current_substring_index += 2;
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__,
						"Failed to copy " SIZET_FMT " characters of \"%s\" for regular expression match", substring_length, (reg_ex_p -> re_target_s) + start);
				}
		}		/*  if (reg_ex_p -> re_current_substring_index < reg_ex_p -> re_num_matches) */

	return value_s;
}
