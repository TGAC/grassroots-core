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
 * json_parameter.c
 *
 *  Created on: 31 Jan 2020
 *      Author: billy
 */

#include "json_parameter.h"
#include "memory_allocations.h"
#include "service.h"
#include "json_util.h"


/*
 * STATIC DECLARATIONS
 */

static bool SetJSONParameterValue (json_t **param_value_pp, const json_t *new_value_p);

static void ClearJSONParameter (Parameter *param_p);

static bool AddJSONParameterDetailsToJSON (const Parameter *param_p, json_t *param_json_p, const bool full_definition_flag);

static bool GetJSONParameterDetailsFromJSON (Parameter *param_p, const json_t *param_json_p);

static bool AddJSONValue (const json_t *value_to_add_p, const char *key_s, json_t *dest_p);

static bool SetJSONParameterCurrentValueFromString (Parameter *param_p, const char *value_s);


/*
 * API DEFINITIONS
 */

JSONParameter *AllocateJSONParameter (const struct ServiceData *service_data_p, const ParameterType pt, const char * const name_s, const char * const display_name_s, const char * const description_s, LinkedList *options_p, json_t *default_value_p, json_t *current_value_p, ParameterLevel level)
{
	JSONParameter *param_p = (JSONParameter *) AllocMemory (sizeof (JSONParameter));

	if (param_p)
		{
			bool success_flag = true;

			if (current_value_p)
				{
					param_p -> jp_current_value_p = json_deep_copy (current_value_p);

					if (! (param_p -> jp_current_value_p))
						{
							success_flag = false;
						}
				}
			else
				{
					param_p -> jp_current_value_p = NULL;
				}


			if (success_flag)
				{
					if (default_value_p)
						{
							param_p -> jp_default_value_p = json_deep_copy (default_value_p);

							if (! (param_p -> jp_default_value_p))
								{
									success_flag = false;
								}
						}
					else
						{
							param_p -> jp_default_value_p = NULL;
						}
				}

			if (success_flag)
				{
					if (InitParameter (& (param_p -> jp_base_param), service_data_p, pt, name_s, display_name_s, description_s, options_p, level,
														 ClearJSONParameter, AddJSONParameterDetailsToJSON,
														 GetJSONParameterDetailsFromJSON, NULL, SetJSONParameterCurrentValueFromString))
						{
							if (service_data_p)
								{
//									GetParameterDefaultValueFromConfig (service_data_p, name_s, param_p -> pa_type, &default_value);
								}

							return param_p;
						}
				}

			if (param_p -> jp_current_value_p)
				{
					json_decref (param_p -> jp_current_value_p);
				}

			if (param_p -> jp_default_value_p)
				{
					json_decref (param_p -> jp_default_value_p);
				}

			FreeMemory (param_p);
		}		/* if (param_p) */

	return NULL;
}


const json_t *GetJSONParameterCurrentValue (const JSONParameter *param_p)
{
	return param_p -> jp_current_value_p;
}


bool SetJSONParameterCurrentValue (JSONParameter *param_p, const json_t *value_p)
{
	return SetJSONParameterValue (& (param_p -> jp_current_value_p), value_p);
}


const json_t *GetJSONParameterDefaultValue (const JSONParameter *param_p)
{
	return param_p -> jp_default_value_p;
}


bool SetJSONParameterDefaultValue (JSONParameter *param_p, const json_t *value_p)
{
	return SetJSONParameterValue (& (param_p -> jp_default_value_p), value_p);
}


bool IsJSONParameter (Parameter *param_p)
{
	bool json_param_flag = false;

	switch (param_p -> pa_type)
		{
			case PT_JSON:
			case PT_JSON_TABLE:
				json_param_flag = true;
				break;

			default:
				break;
		}

	return json_param_flag;
}


bool GetCurrentJSONParameterValueFromParameterSet (const ParameterSet * const params_p, const char * const name_s, const json_t **value_pp)
{
	bool success_flag = false;
	Parameter *param_p = GetParameterFromParameterSetByName (params_p, name_s);

	if (param_p)
		{
			if (IsJSONParameter (param_p))
				{
					const json_t *current_value_p = GetJSONParameterCurrentValue ((const JSONParameter *) param_p);

					*value_pp = current_value_p;
					success_flag = true;
				}
		}

	return success_flag;
}


/*
 * STATIC DEFINITIONS
 */

static bool SetJSONParameterValue (json_t **param_value_pp, const json_t *new_value_p)
{
	bool success_flag = true;

	if (new_value_p)
		{
			if (! (*param_value_pp))
				{
					*param_value_pp = new_value_p;

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
					json_decref (*param_value_pp);
					*param_value_pp = NULL;
				}
		}

	return success_flag;
}



static void ClearJSONParameter (Parameter *param_p)
{
	JSONParameter *json_param_p = (JSONParameter *) param_p;

	if (json_param_p -> jp_current_value_p)
		{
			json_decref (json_param_p -> jp_current_value_p);
			json_param_p -> jp_current_value_p = NULL;
		}

	if (json_param_p -> jp_default_value_p)
		{
			json_decref (json_param_p -> jp_default_value_p);
			json_param_p -> jp_current_value_p = NULL;
		}
}


static bool AddJSONParameterDetailsToJSON (const Parameter *param_p, json_t *param_json_p, const bool full_definition_flag)
{
	JSONParameter *json_param_p = (JSONParameter *) param_p;
	bool success_flag = false;

	if (AddJSONValue (json_param_p -> jp_current_value_p, PARAM_CURRENT_VALUE_S, param_json_p))
		{
			if (full_definition_flag)
				{
					if (AddJSONValue (json_param_p -> jp_default_value_p, PARAM_DEFAULT_VALUE_S, param_json_p))
						{
							success_flag = true;
						}
				}
		}

	return success_flag;
}


static bool AddJSONValue (const json_t *value_to_add_p, const char *key_s, json_t *dest_p)
{
	bool success_flag = false;

	if (value_to_add_p)
		{
			json_t *value_p = json_deep_copy (value_to_add_p);

			if (value_p)
				{
					if (json_object_set_new (dest_p, key_s, value_p) == 0)
						{
							success_flag = true;
						}
				}
		}
	else
		{
			success_flag = true;
		}

	return success_flag;
}


static bool GetJSONParameterDetailsFromJSON (Parameter *param_p, const json_t *param_json_p)
{
	JSONParameter *json_param_p = (JSONParameter *) param_p;
	bool success_flag = true;
	json_t *value_p = json_object_get (param_json_p, PARAM_CURRENT_VALUE_S);

	if (value_p)
		{
			success_flag = SetJSONParameterCurrentValue (json_param_p, value_p);
		}
	else
		{
			success_flag = SetJSONParameterCurrentValue (json_param_p, NULL);
		}

	if (success_flag)
		{
			value_p = json_object_get (param_json_p, PARAM_DEFAULT_VALUE_S);

			if (value_p)
				{
					success_flag = SetJSONParameterDefaultValue (json_param_p, value_p);
				}
			else
				{
					success_flag = SetJSONParameterDefaultValue (json_param_p, NULL);
				}
		}

	return success_flag;
}


static bool SetJSONParameterCurrentValueFromString (Parameter *param_p, const char *value_s)
{
	JSONParameter *json_param_p = (JSONParameter *) param_p;
	bool success_flag = false;

	if (value_s)
		{
			json_error_t err;
			json_t *value_p = json_loads (value_s, 0, &err);

			if (value_p)
				{
					success_flag = SetJSONParameterCurrentValue (json_param_p, value_p);
				}
		}
	else
		{
			success_flag = SetJSONParameterCurrentValue (json_param_p, NULL);
		}

	return success_flag;
}

