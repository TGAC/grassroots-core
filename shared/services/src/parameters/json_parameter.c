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

#define ALLOCATE_JSON_PARAMETER_TAGS (1)
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

static JSONParameter *GetNewJSONParameter (const json_t *current_value_p, const json_t *default_value_p);


/*
 * API DEFINITIONS
 */


JSONParameter *AllocateJSONParameter (const struct ServiceData *service_data_p, const ParameterType pt, const char * const name_s, const char * const display_name_s, const char * const description_s, const json_t *default_value_p, const json_t *current_value_p, ParameterLevel level)
{
	JSONParameter *param_p = GetNewJSONParameter (current_value_p, default_value_p);

	if (param_p)
		{
			if (InitParameter (& (param_p -> jp_base_param), service_data_p, pt, name_s, display_name_s, description_s, level,
												 ClearJSONParameter, AddJSONParameterDetailsToJSON,
												 NULL, SetJSONParameterCurrentValueFromString))
				{
					if (service_data_p)
						{
//									GetParameterDefaultValueFromConfig (service_data_p, name_s, param_p -> pa_type, &default_value);
						}

					return param_p;
				}

			FreeParameter (& (param_p -> jp_base_param));
		}

	return NULL;
}



JSONParameter *AllocateJSONParameterFromJSON (const json_t *param_json_p, const struct Service *service_p, const bool concise_flag, const ParameterType *pt_p)
{
	JSONParameter *param_p = NULL;
	const json_t *current_value_p = json_object_get (param_json_p, PARAM_CURRENT_VALUE_S);
	const json_t *default_value_p = NULL;

	if (!concise_flag)
		{
			default_value_p = json_object_get (param_json_p, PARAM_DEFAULT_VALUE_S);
		}

	param_p = GetNewJSONParameter (current_value_p, default_value_p);

	if (param_p)
		{
			if (InitParameterFromJSON (& (param_p -> jp_base_param), param_json_p, service_p, concise_flag, pt_p))
				{
					SetParameterCallbacks (& (param_p -> jp_base_param), ClearJSONParameter, AddJSONParameterDetailsToJSON,
																 NULL, SetJSONParameterCurrentValueFromString);

					return param_p;

				}
			else
				{
					ClearJSONParameter (& (param_p -> jp_base_param));
					FreeMemory (param_p);
				}
		}


	return NULL;
}


Parameter *EasyCreateAndAddJSONParameterToParameterSet (const ServiceData *service_data_p, ParameterSet *params_p, ParameterGroup *group_p, ParameterType type,
																											const char * const name_s, const char * const display_name_s, const char * const description_s,
																											const json_t *default_value_p, uint8 level)
{
	return CreateAndAddJSONParameterToParameterSet (service_data_p, params_p, group_p, type, name_s, display_name_s, description_s, default_value_p, default_value_p, level);
}


Parameter *CreateAndAddJSONParameterToParameterSet (const ServiceData *service_data_p, ParameterSet *params_p, ParameterGroup *group_p, ParameterType type,
																											const char * const name_s, const char * const display_name_s, const char * const description_s,
																											const json_t *default_value_p, const json_t *current_value_p, uint8 level)
{
	JSONParameter *json_param_p = AllocateJSONParameter (service_data_p, type, name_s, display_name_s, description_s, default_value_p, current_value_p, level);
	Parameter *base_param_p = NULL;

	if (json_param_p)
		{
			base_param_p = & (json_param_p -> jp_base_param);

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

			if (!AddParameterToParameterSet (params_p, base_param_p))
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add param \"%s\" to set \"%s\"", name_s, params_p -> ps_name_s);
					FreeParameter (base_param_p);
					base_param_p = NULL;
				}

		}		/* if (param_p) */

	return base_param_p;
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
					*value_pp = GetJSONParameterCurrentValue ((const JSONParameter *) param_p);
					success_flag = true;
				}
		}

	return success_flag;
}


bool SetJSONParameterCurrentValueFromJSON (JSONParameter *param_p, const json_t *value_p)
{
	bool success_flag = false;

	if (value_p)
		{
			json_t *copied_value_p = json_deep_copy (value_p);

			if (copied_value_p)
				{
					if (param_p -> jp_current_value_p)
						{
							json_decref (param_p -> jp_current_value_p);
						}

					param_p -> jp_current_value_p = copied_value_p;
					success_flag = true;
				}
		}
	else
		{
			if (param_p -> jp_current_value_p)
				{
					json_decref (param_p -> jp_current_value_p);
				}

			param_p -> jp_current_value_p = NULL;
			success_flag = true;
		}

	return success_flag;

}


/*
 * STATIC DEFINITIONS
 */

static bool SetJSONParameterValue (json_t **param_value_pp, const json_t *new_value_p)
{
	bool success_flag = false;

	if (new_value_p)
		{
			json_t *copied_value_p = json_deep_copy (new_value_p);

			if (copied_value_p)
				{
					if (*param_value_pp)
						{
							json_decref (*param_value_pp);
						}

					*param_value_pp = copied_value_p;
					success_flag = true;
				}
		}
	else
		{
			if (*param_value_pp)
				{
					json_decref (*param_value_pp);
				}

			*param_value_pp = NULL;
			success_flag = true;
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
			else
				{
					success_flag = true;
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
			success_flag = AddNullParameterValueToJSON (dest_p, key_s);
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


static JSONParameter *GetNewJSONParameter (const json_t *current_value_p, const json_t *default_value_p)
{
	JSONParameter *param_p = (JSONParameter *) AllocMemory (sizeof (JSONParameter));

	if (param_p)
		{
			param_p -> jp_current_value_p = NULL;
			param_p -> jp_default_value_p = NULL;

			NullifyParameter (& (param_p -> jp_base_param));


			if (SetJSONParameterValue (& (param_p -> jp_current_value_p), current_value_p))
				{
					if (SetJSONParameterValue (& (param_p -> jp_default_value_p), default_value_p))
						{
							return param_p;
						}

					if (param_p -> jp_current_value_p)
						{
							json_decref (param_p -> jp_current_value_p);
						}
				}

			FreeMemory (param_p);
		}

	return NULL;
}

