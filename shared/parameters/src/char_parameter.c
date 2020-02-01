/*
** Copyright 2014-2020 The Earlham Institute
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
 * char_parameter.c
 *
 *  Created on: 30 Jan 2020
 *      Author: billy
 */

#include <string.h>

#include "char_parameter.h"
#include "memory_allocations.h"

#include "service.h"
#include "json_util.h"


/*
 * STATIC DECLARATIONS
 */

static bool SetCharParameterValue (char **param_value_pp, const char *new_value_p);

static void ClearCharParameter (Parameter *param_p);

static bool AddCharParameterDetailsToJSON (const Parameter *param_p, json_t *param_json_p);

static bool GetCharParameterDetailsFromJSON (Parameter *param_p, const json_t *param_json_p);


/*
 * API DEFINITIONS
 */

CharParameter *AllocateCharParameter (const struct ServiceData *service_data_p, const char * const name_s, const char * const display_name_s, const char * const description_s, LinkedList *options_p, char *default_value_p, char *current_value_p, ParameterLevel level, const char *(*check_value_fn) (const Parameter * const parameter_p, const void *value_p))
{
	CharParameter *param_p = (CharParameter *) AllocMemory (sizeof (CharParameter));

	if (param_p)
		{
			bool success_flag = true;

			if (current_value_p)
				{
					param_p -> cp_current_value_p = (char *) AllocMemory (sizeof (char));

					if (param_p -> cp_current_value_p)
						{
							* (param_p -> cp_current_value_p) = *current_value_p;
						}
					else
						{
							success_flag = false;
						}
				}
			else
				{
					param_p -> cp_current_value_p = NULL;
				}


			if (success_flag)
				{
					if (default_value_p)
						{
							param_p -> cp_default_value_p = (char *) AllocMemory (sizeof (char));

							if (param_p -> cp_default_value_p)
								{
									* (param_p -> cp_default_value_p) = *default_value_p;
								}
							else
								{
									success_flag = false;
								}
						}
					else
						{
							param_p -> cp_default_value_p = NULL;
						}
				}

			if (success_flag)
				{
					if (InitParameter (& (param_p -> cp_base_param), service_data_p, PT_CHAR, name_s, display_name_s, description_s, options_p, level, check_value_fn))
						{
							if (service_data_p)
								{
//									GetParameterDefaultValueFromConfig (service_data_p, name_s, param_p -> pa_type, &default_value);
								}

							param_p -> cp_min_value_p = NULL;
							param_p -> cp_max_value_p = NULL;

							return param_p;
						}
				}

			if (param_p -> cp_current_value_p)
				{
					FreeMemory (param_p -> cp_current_value_p);
				}

			if (param_p -> cp_default_value_p)
				{
					FreeMemory (param_p -> cp_default_value_p);
				}

			FreeMemory (param_p);
		}		/* if (param_p) */

	return NULL;
}


const char *GetCharParameterCurrentValue (const CharParameter *param_p)
{
	return param_p -> cp_current_value_p;
}


bool SetCharParameterCurrentValue (CharParameter *param_p, const char *value_p)
{
	return SetCharParameterValue (& (param_p -> cp_current_value_p), value_p);
}


const char *GetCharParameterDefaultValue (const CharParameter *param_p)
{
	return param_p -> cp_default_value_p;
}


bool SetCharParameterDefaultValue (CharParameter *param_p, const char *value_p)
{
	return SetCharParameterValue (& (param_p -> cp_default_value_p), value_p);
}


/*
 * STATIC DEFINITIONS
 */

static bool SetCharParameterValue (char **param_value_pp, const char *new_value_p)
{
	bool success_flag = true;

	if (new_value_p)
		{
			if (! (*param_value_pp))
				{
					*param_value_pp = (char *) AllocMemory (sizeof (char));

					if (! (*param_value_pp))
						{
							success_flag = false;
						}
				}

			if (success_flag)
				{
					**param_value_pp = *new_value_p;
				}
		}
	else
		{
			if (*param_value_pp)
				{
					FreeMemory (*param_value_pp);
					*param_value_pp = NULL;
				}
		}

	return success_flag;
}


bool SetCharParameterBounds (CharParameter *param_p, const char min_value, const char max_value)
{
	if (! (param_p -> cp_min_value_p))
		{
			param_p -> cp_min_value_p = (char *) AllocMemory (sizeof (char));

			if (! (param_p -> cp_min_value_p))
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate min value");
					return false;
				}
		}

	if (! (param_p -> cp_max_value_p))
		{
			param_p -> cp_max_value_p = (char *) AllocMemory (sizeof (char));

			if (! (param_p -> cp_max_value_p))
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate max value");
					return false;
				}
		}

	* (param_p -> cp_min_value_p) = min_value;
	* (param_p -> cp_max_value_p) = max_value;

	return true;
}


bool IsCharParameterBounded (const CharParameter *param_p)
{
	return ((param_p -> cp_min_value_p) && (param_p -> cp_max_value_p));
}


bool GetCharParameterBounds (const CharParameter *param_p, char *min_p, char *max_p)
{
	bool success_flag = false;

	if (IsCharParameterBounded (param_p))
		{
			*min_p = * (param_p -> cp_min_value_p);
			*max_p = * (param_p -> cp_max_value_p);

			success_flag = true;
		}

	return success_flag;
}



static void ClearCharParameter (Parameter *param_p)
{
	CharParameter *char_param_p = (CharParameter *) param_p;

	if (char_param_p -> cp_current_value_p)
		{
			FreeMemory (char_param_p -> cp_current_value_p);
			char_param_p -> cp_current_value_p = NULL;
		}

	if (char_param_p -> cp_default_value_p)
		{
			FreeMemory (char_param_p -> cp_default_value_p);
			char_param_p -> cp_default_value_p = NULL;
		}

	if (char_param_p -> cp_min_value_p)
		{
			FreeMemory (char_param_p -> cp_min_value_p);
			char_param_p -> cp_min_value_p = NULL;
		}

	if (char_param_p -> cp_max_value_p)
		{
			FreeMemory (char_param_p -> cp_max_value_p);
			char_param_p -> cp_max_value_p = NULL;
		}

}


static bool AddCharParameterDetailsToJSON (const Parameter *param_p, json_t *param_json_p)
{
	CharParameter *char_param_p = (CharParameter *) param_p;
	bool success_flag = true;
	char buffer_s [2];

	* (buffer_s + 1) = '\0';

	if (char_param_p -> cp_current_value_p)
		{
			*buffer_s = * (char_param_p -> cp_current_value_p);

			success_flag = SetJSONString (param_json_p, PARAM_CURRENT_VALUE_S, buffer_s);
		}

	if (success_flag)
		{
			if (char_param_p -> cp_default_value_p)
				{
					*buffer_s = * (char_param_p -> cp_default_value_p);

					success_flag = SetJSONString (param_json_p, PARAM_DEFAULT_VALUE_S, buffer_s);
				}
		}

	return success_flag;
}


static bool GetCharParameterDetailsFromJSON (Parameter *param_p, const json_t *param_json_p)
{
	CharParameter *char_param_p = (CharParameter *) param_p;
	bool success_flag = true;
	const char *value_s = GetJSONString (param_json_p, PARAM_CURRENT_VALUE_S);

	if (value_s)
		{
			if (strlen (value_s) == 1)
				{
					success_flag = SetCharParameterCurrentValue (char_param_p, value_s);
				}
		}
	else
		{
			success_flag = SetCharParameterCurrentValue (char_param_p, NULL);
		}

	if (success_flag)
		{
			value_s = GetJSONString (param_json_p, PARAM_DEFAULT_VALUE_S);

			if (value_s)
				{
					if (strlen (value_s) == 1)
						{
							success_flag = SetCharParameterCurrentValue (char_param_p, value_s);
						}
				}
		}
	else
		{
			success_flag = SetCharParameterDefaultValue (char_param_p, NULL);
		}


	return success_flag;
}
