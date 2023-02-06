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
 * resource_parameter.c
 *
 *  Created on: 31 Jan 2020
 *      Author: billy
 */

#include "resource_parameter.h"
#include "memory_allocations.h"
#include "../../services/include/service.h"
#include "json_util.h"


/*
 * STATIC DECLARATIONS
 */

static bool SetResourceParameterValue (DataResource **param_value_pp, const DataResource *new_value_p);

static void ClearResourceParameter (Parameter *param_p);

static bool AddResourceParameterDetailsToJSON (const Parameter *param_p, json_t *param_json_p, const bool full_definition_flag);

static bool GetResourceParameterDetailsFromJSON (Parameter *param_p, const json_t *param_json_p);

static bool AddResourceValueToJSON (const DataResource *resource_p, json_t *param_json_p, const char *key_s);

static bool SetResourceParameterCurrentValueFromString (Parameter *param_p, const char *value_s);

static bool SetResourceParameterValueFromJSON (DataResource **res_pp, const json_t *param_json_p, const char *key_s);

static ResourceParameter *GetNewResourceParameter (const DataResource *current_value_p, const DataResource *default_value_p);

/*
 * API DEFINITIONS
 */


static ResourceParameter *GetNewResourceParameter (const DataResource *current_value_p, const DataResource *default_value_p)
{
	ResourceParameter *param_p = (ResourceParameter *) AllocMemory (sizeof (ResourceParameter));

	if (param_p)
		{
			param_p -> rp_current_value_p = NULL;
			param_p -> rp_default_value_p = NULL;

			if (SetResourceParameterValue (& (param_p -> rp_current_value_p), current_value_p))
				{
					if (SetResourceParameterValue (& (param_p -> rp_default_value_p), default_value_p))
						{
							return param_p;
						}

					if (param_p -> rp_current_value_p)
						{
							FreeDataResource (param_p -> rp_current_value_p);
						}
				}

			FreeMemory (param_p);
		}

	return NULL;
}


ResourceParameter *AllocateResourceParameter (const struct ServiceData *service_data_p, const ParameterType pt, const char * const name_s, const char * const display_name_s, const char * const description_s, DataResource *default_value_p, DataResource *current_value_p, ParameterLevel level)
{
	ResourceParameter *param_p = GetNewResourceParameter (current_value_p, default_value_p);

	if (param_p)
		{
			if (InitParameter (& (param_p -> rp_base_param), service_data_p, pt, name_s, display_name_s, description_s, level,
												 ClearResourceParameter, AddResourceParameterDetailsToJSON,
												 NULL, SetResourceParameterCurrentValueFromString))
				{
					if (service_data_p)
						{
//									GetParameterDefaultValueFromConfig (service_data_p, name_s, param_p -> pa_type, &default_value);
						}

					return param_p;
				}

			FreeParameter (& (param_p -> rp_base_param));
		}

	return NULL;
}




ResourceParameter *AllocateResourceParameterFromJSON (const json_t *param_json_p, const Service *service_p, const bool concise_flag)
{
	ResourceParameter *param_p = NULL;
	DataResource *current_value_p = NULL;

	if (SetResourceParameterValueFromJSON (&current_value_p, param_json_p, PARAM_CURRENT_VALUE_S))
		{
			DataResource *default_value_p = NULL;
			bool success_flag = true;

			if (!concise_flag)
				{
					if (!SetResourceParameterValueFromJSON (&default_value_p, param_json_p, PARAM_DEFAULT_VALUE_S))
						{
							success_flag = false;
						}
				}

			if (success_flag)
				{
					param_p = GetNewResourceParameter (current_value_p, default_value_p);

					if (param_p)
						{

							if (InitParameterFromJSON (& (param_p -> rp_base_param), param_json_p, service_p, concise_flag, NULL))
								{
									SetParameterCallbacks (& (param_p -> rp_base_param), ClearResourceParameter, AddResourceParameterDetailsToJSON,
																				 NULL, SetResourceParameterCurrentValueFromString);
								}
							else
								{
									ClearResourceParameter (& (param_p -> rp_base_param));
									FreeMemory (param_p);
									param_p = NULL;
								}

						}
				}

			if (default_value_p)
				{
					FreeDataResource (default_value_p);
				}
		}

	if (current_value_p)
		{
			FreeDataResource (current_value_p);
		}

	return param_p;
}



const DataResource *GetResourceParameterCurrentValue (const ResourceParameter *param_p)
{
	return param_p -> rp_current_value_p;
}


bool SetResourceParameterCurrentValue (ResourceParameter *param_p, const DataResource *value_p)
{
	return SetResourceParameterValue (& (param_p -> rp_current_value_p), value_p);
}


const DataResource *GetResourceParameterDefaultValue (const ResourceParameter *param_p)
{
	return param_p -> rp_default_value_p;
}


bool SetResourceParameterDefaultValue (ResourceParameter *param_p, const DataResource *value_p)
{
	return SetResourceParameterValue (& (param_p -> rp_default_value_p), value_p);
}


bool IsResourceParameter (const Parameter *param_p)
{
	bool resource_param_flag = false;

	switch (param_p -> pa_type)
		{
			case PT_FILE_TO_READ:
			case PT_FILE_TO_WRITE:
			case PT_DIRECTORY:
				resource_param_flag = true;
				break;

			default:
				break;
		}

	return resource_param_flag;
}


bool GetCurrentResourceParameterValueFromParameterSet (const ParameterSet * const params_p, const char * const name_s, const DataResource **value_pp)
{
	bool success_flag = false;
	Parameter *param_p = GetParameterFromParameterSetByName (params_p, name_s);

	if (param_p)
		{
			if (IsResourceParameter (param_p))
				{
					*value_pp = GetResourceParameterCurrentValue ((const ResourceParameter *) param_p);
					success_flag = true;
				}
		}

	return success_flag;
}


bool SetResourceParameterCurrentValueFromJSON (ResourceParameter *param_p, const json_t *value_p)
{
	bool success_flag = true;

	if (value_p)
		{
			if (param_p -> rp_current_value_p)
				{
					DataResource *temp_p = GetDataResourceFromJSON (value_p);

					if (temp_p)
						{
							FreeDataResource (param_p -> rp_current_value_p);
							param_p -> rp_current_value_p = temp_p;
						}
					else
						{
							success_flag = false;
							PrintJSONToErrors (STM_LEVEL_SEVERE,__FILE__, __LINE__, value_p, "SetResourceParameterCurrentValueFromJSON () for \"%s\"", param_p -> rp_base_param.pa_name_s);
						}
				}
			else
				{
					param_p -> rp_current_value_p = GetDataResourceFromJSON (value_p);

					if (! (param_p -> rp_current_value_p))
						{
							success_flag = false;
							PrintJSONToErrors (STM_LEVEL_SEVERE,__FILE__, __LINE__, value_p, "SetResourceParameterCurrentValueFromJSON () for \"%s\"", param_p -> rp_base_param.pa_name_s);
						}
				}
		}
	else
		{
			if (param_p -> rp_current_value_p)
				{
					FreeDataResource (param_p -> rp_current_value_p);
					param_p -> rp_current_value_p = NULL;
				}
		}

	return success_flag;

}


/*
 * STATIC DEFINITIONS
 */

static bool SetResourceParameterValue (DataResource **param_value_pp, const DataResource *new_value_p)
{
	bool success_flag = true;

	if (new_value_p)
		{
			if (*param_value_pp)
				{
					success_flag = CopyDataResource (new_value_p, *param_value_pp);
				}
			else
				{
					*param_value_pp = CloneDataResource (new_value_p);

					if (! (*param_value_pp))
						{
							success_flag = false;
						}
				}
		}
	else
		{
			if (*param_value_pp)
				{
					FreeDataResource (*param_value_pp);
					*param_value_pp = NULL;
				}
		}

	return success_flag;
}



static void ClearResourceParameter (Parameter *param_p)
{
	ResourceParameter *int_param_p = (ResourceParameter *) param_p;

	if (int_param_p -> rp_current_value_p)
		{
			FreeMemory (int_param_p -> rp_current_value_p);
			int_param_p -> rp_current_value_p = NULL;
		}

	if (int_param_p -> rp_default_value_p)
		{
			FreeMemory (int_param_p -> rp_default_value_p);
			int_param_p -> rp_current_value_p = NULL;
		}
}


static bool AddResourceValueToJSON (const DataResource *resource_p, json_t *param_json_p, const char *key_s)
{
	bool success_flag = false;

	if (resource_p)
		{
			json_t *res_json_p = GetDataResourceAsJSON (resource_p);

			if (res_json_p)
				{
					if (json_object_set_new (param_json_p, key_s, res_json_p) == 0)
						{
							success_flag = true;
						}
					else
						{
							json_decref (res_json_p);
						}
				}
		}
	else
		{
			success_flag = AddNullParameterValueToJSON (param_json_p, key_s);
		}

	return success_flag;
}


static bool AddResourceParameterDetailsToJSON (const Parameter *param_p, json_t *param_json_p, const bool full_definition_flag)
{
	ResourceParameter *res_param_p = (ResourceParameter *) param_p;
	bool success_flag = false;

	if (AddResourceValueToJSON (res_param_p -> rp_current_value_p, param_json_p, PARAM_CURRENT_VALUE_S))
		{
			if (full_definition_flag)
				{
					if (AddResourceValueToJSON (res_param_p -> rp_default_value_p, param_json_p, PARAM_DEFAULT_VALUE_S))
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


static bool GetResourceParameterDetailsFromJSON (Parameter *param_p, const json_t *param_json_p)
{
	ResourceParameter *res_param_p = (ResourceParameter *) param_p;
	bool success_flag = false;

	if (SetResourceParameterValueFromJSON (& (res_param_p -> rp_current_value_p), param_json_p, PARAM_CURRENT_VALUE_S))
		{
			if (SetResourceParameterValueFromJSON (& (res_param_p -> rp_default_value_p), param_json_p, PARAM_DEFAULT_VALUE_S))
				{
					success_flag = true;
				}
		}

	return success_flag;
}




/*
static bool GetResourceValueFromJSON (const json_t *param_json_p, const char *key_s, Resource **res_pp)
{
	bool success_flag = false;
	const char *value_s = GetJSONString (param_json_p, key_s);

	if (value_s)
		{
			if (*res_pp)
				{
					if (SetResourceFromString (*res_param_p, value_s))
						{
							success_flag = true;
						}
				}
			else
				{
					*res_param_p = GetTimeFromString (value_s);

					if (*res_param_p)
						{
							success_flag = true;
						}
				}
		}		 if (value_s)
	else
		{
			if (*res_param_p)
				{
					FreeResource (*res_param_p);
					*res_param_p = NULL;
				}
		}

	return success_flag;
}
*/


static bool SetResourceParameterCurrentValueFromString (Parameter *param_p, const char *value_s)
{
	ResourceParameter *res_param_p = (ResourceParameter *) param_p;
	bool success_flag = false;

	if (value_s)
		{
			DataResource *resource_p = ParseStringToDataResource (value_s);

			if (resource_p)
				{
					success_flag = SetResourceParameterCurrentValue (res_param_p, resource_p);
					FreeDataResource (resource_p);
				}
		}
	else
		{
			success_flag = SetResourceParameterCurrentValue (res_param_p, NULL);
		}

	return success_flag;
}


static bool SetResourceParameterValueFromJSON (DataResource **res_pp, const json_t *param_json_p, const char *key_s)
{
	bool success_flag = false;
	const json_t *value_p = json_object_get (param_json_p, key_s);

	if ((value_p == NULL) && (json_is_null (value_p)))
		{
			if (*res_pp)
				{
					FreeDataResource (*res_pp);
				}

			*res_pp = NULL;
			success_flag = true;
		}
	else
		{
			DataResource *res_p = GetDataResourceFromJSON (value_p);

			if (res_p)
				{
					if (*res_pp)
						{
							success_flag = CopyDataResource (res_p, *res_pp);
						}
					else
						{
							*res_pp = CloneDataResource (res_p);

							success_flag = (*res_pp != NULL);
						}

					FreeDataResource (res_p);
				}
		}

	return success_flag;
}
