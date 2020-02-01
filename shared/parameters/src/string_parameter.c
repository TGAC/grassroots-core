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

#include "string_parameter.h"
#include "memory_allocations.h"

#include "service.h"
#include "json_util.h"


/*
 * STATIC DECLARATIONS
 */

static void ClearStringParameter (Parameter *param_p);

static bool AddStringParameterDetailsToJSON (const Parameter *param_p, json_t *param_json_p);

static bool GetStringParameterDetailsFromJSON (Parameter *param_p, const json_t *param_json_p);


/*
 * API DEFINITIONS
 */

StringParameter *AllocateStringParameter (const struct ServiceData *service_data_p, const ParameterType pt, const char * const name_s, const char * const display_name_s, const char * const description_s, LinkedList *options_p, char *default_value_p, char *current_value_p, ParameterBounds *bounds_p, ParameterLevel level, const char *(*check_value_fn) (const Parameter * const parameter_p, const void *value_p))
{
	StringParameter *param_p = (StringParameter *) AllocMemory (sizeof (StringParameter));

	if (param_p)
		{
			bool success_flag = true;

			if (current_value_p)
				{
					param_p -> sp_current_value_s = EasyCopyToNewString (current_value_p);

					if (! (param_p -> sp_current_value_s))
						{
							success_flag = false;
						}
				}
			else
				{
					param_p -> sp_current_value_s = NULL;
				}


			if (success_flag)
				{
					if (default_value_p)
						{
							param_p -> sp_default_value_s = EasyCopyToNewString (default_value_p);

							if (! (param_p -> sp_default_value_s))
								{
									success_flag = false;
								}
						}
					else
						{
							param_p -> sp_default_value_s = NULL;
						}
				}

			if (success_flag)
				{
					if (InitParameter (& (param_p -> sp_base_param), service_data_p, pt, name_s, display_name_s, description_s, options_p, bounds_p, level, check_value_fn))
						{
							if (service_data_p)
								{
//									GetParameterDefaultValueFromConfig (service_data_p, name_s, param_p -> pa_type, &default_value);
								}

							return param_p;
						}
				}

			if (param_p -> sp_current_value_s)
				{
					FreeCopiedString (param_p -> sp_current_value_s);
				}

			if (param_p -> sp_default_value_s)
				{
					FreeCopiedString (param_p -> sp_default_value_s);
				}

			FreeMemory (param_p);
		}		/* if (param_p) */

	return NULL;
}


const char *GetStringParameterCurrentValue (const StringParameter *param_p)
{
	return param_p -> sp_current_value_s;
}


bool SetStringParameterCurrentValue (StringParameter *param_p, const char *value_p)
{
	return SetStringParameterValue (& (param_p -> sp_current_value_s), value_p);
}


const char *GetStringParameterDefaultValue (const StringParameter *param_p)
{
	return param_p -> sp_default_value_s;
}


bool SetStringParameterDefaultValue (StringParameter *param_p, const char *value_p)
{
	return SetStringParameterValue (& (param_p -> sp_default_value_s), value_p);
}


bool SetStringParameterBounds (StringParameter *param_p, const char *min_value_s, const char *max_value_s)
{
	bool success_flag = false;

	if (SetStringParameterValue (& (param_p -> sp_min_value_s), min_value_s))
		{
			if (SetStringParameterValue (& (param_p -> sp_max_value_s), max_value_s))
				{
					success_flag = true;
				}
		}


	return success_flag;
}


bool IsStringParameterBounded (const StringParameter *param_p)
{
	return ((param_p -> sp_min_value_s) && (param_p -> sp_max_value_s));
}


bool GetStringParameterBounds (const StringParameter *param_p, const char *min_p, const char *max_p)
{
	bool success_flag = false;

	if (IsStringParameterBounded (param_p))
		{
			*min_p = * (param_p -> sp_min_value_s);
			*max_p = * (param_p -> sp_max_value_s);

			success_flag = true;
		}

	return success_flag;
}



/*
 * STATIC DEFINITIONS
 */

static bool SetStringParameterValue (char **param_value_pp, const char *new_value_p);

static bool SetStringParameterValue (char **param_value_ss, const char *new_value_s)
{
	bool success_flag = false;

	if (new_value_s)
		{
			char *copied_value_s = EasyCopyToNewString (new_value_s);

			if (copied_value_s)
				{
					if (*param_value_ss)
						{
							FreeCopiedString (*param_value_ss);
						}

					*param_value_ss = copied_value_s;
					success_flag = true;
				}
		}
	else
		{
			if (*param_value_ss)
				{
					FreeCopiedString (*param_value_ss);
					*param_value_ss = NULL;
				}

			success_flag = true;
		}

	return success_flag;
}



static void ClearStringParameter (Parameter *param_p)
{
	StringParameter *string_param_p = (StringParameter *) param_p;

	if (string_param_p -> sp_current_value_s)
		{
			FreeCopiedString (string_param_p -> sp_current_value_s);
			string_param_p -> sp_current_value_s = NULL;
		}

	if (string_param_p -> sp_default_value_s)
		{
			FreeCopiedString (string_param_p -> sp_default_value_s);
			string_param_p -> sp_default_value_s = NULL;
		}

	if (string_param_p -> sp_min_value_s)
		{
			FreeCopiedString (string_param_p -> sp_min_value_s);
			string_param_p -> sp_min_value_s = NULL;
		}

	if (string_param_p -> sp_max_value_s)
		{
			FreeCopiedString (string_param_p -> sp_max_value_s);
			string_param_p -> sp_max_value_s = NULL;
		}

}


static bool AddStringParameterDetailsToJSON (const Parameter *param_p, json_t *param_json_p)
{
	StringParameter *string_param_p = (StringParameter *) param_p;
	bool success_flag = true;

	if (string_param_p -> sp_current_value_s)
		{
			success_flag = SetJSONString (param_json_p, PARAM_CURRENT_VALUE_S, string_param_p -> sp_current_value_s);
		}

	if (success_flag)
		{
			if (string_param_p -> sp_default_value_s)
				{
					success_flag = SetJSONString (param_json_p, PARAM_DEFAULT_VALUE_S, string_param_p -> sp_default_value_s);
				}
		}

	return success_flag;
}


static bool GetStringParameterDetailsFromJSON (Parameter *param_p, const json_t *param_json_p)
{
	StringParameter *string_param_p = (StringParameter *) param_p;
	bool success_flag = true;
	const char *value_s = GetJSONString (param_json_p, PARAM_CURRENT_VALUE_S);

	if (value_s)
		{
			success_flag = SetStringParameterCurrentValue (string_param_p, value_s);
		}
	else
		{
			success_flag = SetStringParameterCurrentValue (string_param_p, NULL);
		}

	if (success_flag)
		{
			value_s = GetJSONString (param_json_p, PARAM_DEFAULT_VALUE_S);

			if (value_s)
				{
					success_flag = SetStringParameterCurrentValue (string_param_p, value_s);
				}
		}
	else
		{
			success_flag = SetStringParameterDefaultValue (string_param_p, NULL);
		}


	return success_flag;
}
