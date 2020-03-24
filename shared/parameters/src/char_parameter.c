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

static bool AddCharParameterDetailsToJSON (const Parameter *param_p, json_t *param_json_p, const bool full_definition_flag);

static bool GetCharParameterDetailsFromJSON (Parameter *param_p, const json_t *param_json_p);

static bool AddJSONValue (const char *c_p, const char *key_s, json_t *dest_p);

static bool SetValueFromJSON (char **value_pp, const json_t *param_json_p, const char *key_s);

static bool SetCharParameterCurrentValueFromString (Parameter *param_p, const char *value_s);

static CharParameter *GetNewCharParameter (const char *current_value_p, const char *default_value_p);


/*
 * API DEFINITIONS
 */


CharParameter *AllocateCharParameterFromJSON (const json_t *param_json_p, const struct Service *service_p, const bool concise_flag)
{
	char *current_value_p = NULL;

	if (SetValueFromJSON (&current_value_p, param_json_p, PARAM_CURRENT_VALUE_S))
		{
			char *default_value_p = NULL;
			bool success_flag = true;

			if (!concise_flag)
				{
					if (!SetValueFromJSON (&default_value_p, param_json_p, PARAM_DEFAULT_VALUE_S))
						{
							success_flag = false;
						}
				}

			if (success_flag)
				{
					CharParameter *param_p = GetNewCharParameter (current_value_p, default_value_p);

					if (param_p)
						{
							if (InitParameterFromJSON (& (param_p -> cp_base_param), param_json_p, service_p, concise_flag))
								{
									if (!SetParameterCallbacks (& (param_p -> cp_base_param), ClearCharParameter, AddCharParameterDetailsToJSON,
																				 NULL, SetCharParameterCurrentValueFromString))
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "SetParameterCallbacks failed for \"%s\ in service \"%s\"", param_p -> cp_base_param.pa_name_s, GetServiceName (service_p));
											FreeParameter (param_p);
											param_p = NULL;
										}
								}
						}


					if (default_value_p)
						{
							FreeMemory (default_value_p);
						}
				}		/* if (SetValueFromJSON (&default_value_p, param_json_p, PARAM_DEFAULT_VALUE_S)) */

			if (current_value_p)
				{
					FreeMemory (current_value_p);
				}
		}		/* if (SetValueFromJSON (&current_value_p, param_json_p, PARAM_CURRENT_VALUE_S)) */

	return NULL;
}


CharParameter *AllocateCharParameter (const struct ServiceData *service_data_p, const char * const name_s, const char * const display_name_s, const char * const description_s, const char *default_value_p, const char *current_value_p, ParameterLevel level)
{
	CharParameter *param_p = GetNewCharParameter (current_value_p, default_value_p);

	if (param_p)
		{
			if (InitParameter (& (param_p -> cp_base_param), service_data_p, PT_CHAR, name_s, display_name_s, description_s, level,
												 ClearCharParameter, AddCharParameterDetailsToJSON,
												 NULL, SetCharParameterCurrentValueFromString))
				{
					if (service_data_p)
						{
//									GetParameterDefaultValueFromConfig (service_data_p, name_s, param_p -> pa_type, &default_value);
						}

					return param_p;
				}

			FreeParameter (& (param_p -> cp_base_param));
		}

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


bool IsCharParameter (Parameter *param_p)
{
	bool char_param_flag = false;

	switch (param_p -> pa_type)
		{
			case PT_CHAR:
				char_param_flag = true;
				break;

			default:
				break;
		}

	return char_param_flag;
}


bool GetCurrentCharParameterValueFromParameterSet (const ParameterSet * const params_p, const char * const name_s, const char **value_pp)
{
	bool success_flag = false;
	Parameter *param_p = GetParameterFromParameterSetByName (params_p, name_s);

	if (param_p)
		{
			if (IsCharParameter (param_p))
				{
					*value_pp = GetCharParameterCurrentValue ((const CharParameter *) param_p);
					success_flag = true;
				}
		}

	return success_flag;
}


Parameter *EasyCreateAndAddCharParameterToParameterSet (const ServiceData *service_data_p, ParameterSet *params_p, ParameterGroup *group_p,
																								const char * const name_s, const char * const display_name_s, const char * const description_s,
																								const char *default_value_p, uint8 level)
{
	return CreateAndAddCharParameterToParameterSet (service_data_p, params_p, group_p, name_s, display_name_s, description_s, default_value_p, default_value_p, level);
}


Parameter *CreateAndAddCharParameterToParameterSet (const ServiceData *service_data_p, ParameterSet *params_p, ParameterGroup *group_p,
																								const char * const name_s, const char * const display_name_s, const char * const description_s,
																								const char *default_value_p, const char *current_value_p, uint8 level)
{
	CharParameter *char_param_p = AllocateCharParameter (service_data_p, name_s, display_name_s, description_s, default_value_p, current_value_p, level);

	if (char_param_p)
		{
			Parameter *base_param_p = & (char_param_p -> cp_base_param);

			if (group_p)
				{
					/*
					 * If the parameter fails to get added to the group, it's
					 * not a terminal error so still carry on
					 */
					if (!AddParameterToParameterGroup (group_p, base_param_p))
						{
							PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to add param \"%s\" to group \"%s\"", name_s, group_p -> pg_name_s);
						}
				}

			if (AddParameterToParameterSet (params_p, base_param_p))
				{
					return base_param_p;
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add param \"%s\" to set \"%s\"", name_s, params_p -> ps_name_s);
					FreeParameter (base_param_p);
				}

		}		/* if (char_param_p) */

	return NULL;
}


/*
 * STATIC DEFINITIONS
 */



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


static bool AddCharParameterDetailsToJSON (const Parameter *param_p, json_t *param_json_p, const bool full_definition_flag)
{
	CharParameter *char_param_p = (CharParameter *) param_p;
	bool success_flag = false;

	if ((char_param_p -> cp_current_value_p == NULL ) || (AddJSONValue (char_param_p -> cp_current_value_p, PARAM_CURRENT_VALUE_S, param_json_p)))
		{
			if (full_definition_flag)
				{
					if ((char_param_p -> cp_default_value_p == NULL ) || (AddJSONValue (char_param_p -> cp_default_value_p, PARAM_DEFAULT_VALUE_S, param_json_p)))
						{
							if ((char_param_p -> cp_min_value_p == NULL ) || (AddJSONValue (char_param_p -> cp_min_value_p, PARAM_MIN_S, param_json_p)))
								{
									if ((char_param_p -> cp_max_value_p == NULL ) || (AddJSONValue (char_param_p -> cp_max_value_p, PARAM_MAX_S, param_json_p)))
										{
											success_flag = true;
										}
								}
						}
				}
			else
				{
					success_flag = true;
				}
		}

	return success_flag;
}


static bool AddJSONValue (const char *c_p, const char *key_s, json_t *dest_p)
{
	bool success_flag = false;

	if (c_p)
		{
			char buffer_s [2];

			*buffer_s = *c_p;
			* (buffer_s + 1) = '\0';

			if (SetJSONString (dest_p, key_s, buffer_s))
				{
					success_flag = true;
				}
		}
	else
		{
			success_flag = true;
		}

	return success_flag;
}


static bool GetCharParameterDetailsFromJSON (Parameter *param_p, const json_t *param_json_p)
{
	CharParameter *char_param_p = (CharParameter *) param_p;
	bool success_flag = false;

	if (SetValueFromJSON (& (char_param_p -> cp_current_value_p), param_json_p, PARAM_CURRENT_VALUE_S))
		{
			if (SetValueFromJSON (& (char_param_p -> cp_default_value_p), param_json_p, PARAM_DEFAULT_VALUE_S))
				{
					if (SetValueFromJSON (& (char_param_p -> cp_min_value_p), param_json_p, PARAM_MIN_S))
						{
							if (SetValueFromJSON (& (char_param_p -> cp_max_value_p), param_json_p, PARAM_MAX_S))
								{
									success_flag = true;
								}
						}
				}
		}

	return success_flag;
}


static bool SetValueFromJSON (char **value_pp, const json_t *param_json_p, const char *key_s)
{
	bool success_flag = false;
	const json_t *value_p = json_object_get (param_json_p, key_s);

	if (value_p)
		{
			if (json_is_string (value_p))
				{
					const char *value_s = json_string_value (value_p);

					if (value_s)
						{
							if (strlen (value_s) == 1)
								{
									char buffer_s [2];

									*buffer_s = *value_s;
									* (buffer_s + 1) = '\0';

									success_flag = SetCharParameterValue (value_pp, buffer_s);
								}
						}
				}
			else if (json_is_null (value_p))
				{
					success_flag = SetCharParameterValue (value_pp, NULL);
				}
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, value_p, "JSON value is of the wrong type, %d not string", value_p -> type);
				}
		}
	else
		{
			success_flag = SetCharParameterValue (value_pp, NULL);
		}



	return success_flag;
}


static bool SetCharParameterCurrentValueFromString (Parameter *param_p, const char *value_s)
{
	CharParameter *char_param_p = (CharParameter *) param_p;
	bool success_flag = false;

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

	return success_flag;
}



static CharParameter *GetNewCharParameter (const char *current_value_p, const char *default_value_p)
{
	CharParameter *param_p = (CharParameter *) AllocMemory (sizeof (CharParameter));

	if (param_p)
		{
			param_p -> cp_current_value_p = NULL;
			param_p -> cp_default_value_p = NULL;

			if (SetCharParameterValue (& (param_p -> cp_current_value_p), current_value_p))
				{
					if (SetCharParameterValue (& (param_p -> cp_default_value_p), default_value_p))
						{
							param_p -> cp_min_value_p = NULL;
							param_p -> cp_max_value_p = NULL;

							return param_p;
						}

					if (param_p -> cp_current_value_p)
						{
							FreeMemory (param_p -> cp_current_value_p);
						}
				}

			FreeMemory (param_p);
		}

	return NULL;
}




