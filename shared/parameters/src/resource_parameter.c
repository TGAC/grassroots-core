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
#include "service.h"
#include "json_util.h"


/*
 * STATIC DECLARATIONS
 */

static bool SetResourceParameterValue (Resource **param_value_pp, const Resource *new_value_p);

static void ClearResourceParameter (Parameter *param_p);

static bool AddResourceParameterDetailsToJSON (const Parameter *param_p, json_t *param_json_p, const bool full_definition_flag);

static bool GetResourceParameterDetailsFromJSON (Parameter *param_p, const json_t *param_json_p);

static bool AddResourceValueToJSON (const Resource *resource_p, json_t *param_json_p, const char *key_s);

static bool SetResourceParameterCurrentValueFromString (Parameter *param_p, const char *value_s);

/*
 * API DEFINITIONS
 */

ResourceParameter *AllocateResourceParameter (const struct ServiceData *service_data_p, const ParameterType pt, const char * const name_s, const char * const display_name_s, const char * const description_s, LinkedList *options_p, Resource *default_value_p, Resource *current_value_p, ParameterLevel level)
{
	ResourceParameter *param_p = (ResourceParameter *) AllocMemory (sizeof (ResourceParameter));

	if (param_p)
		{
			bool success_flag = true;

			if (current_value_p)
				{
					param_p -> rp_current_value_p = CloneResource (current_value_p);

					if (!param_p -> rp_current_value_p)
						{
							success_flag = false;
						}
				}


			if (success_flag)
				{
					if (default_value_p)
						{
							param_p -> rp_default_value_p = CloneResource (default_value_p);

							if (!param_p -> rp_default_value_p)
								{
									success_flag = false;
								}
						}
				}

			if (success_flag)
				{
					if (InitParameter (& (param_p -> rp_base_param), service_data_p, pt, name_s, display_name_s, description_s, options_p, level,
														 ClearResourceParameter, AddResourceParameterDetailsToJSON, GetResourceParameterDetailsFromJSON,
														 NULL, SetResourceParameterCurrentValueFromString))
						{
							if (service_data_p)
								{
//									GetParameterDefaultValueFromConfig (service_data_p, name_s, param_p -> pa_type, &default_value);
								}

							return param_p;
						}
				}

			if (param_p -> rp_current_value_p)
				{
					FreeResource (param_p -> rp_current_value_p);
				}

			if (param_p -> rp_default_value_p)
				{
					FreeResource (param_p -> rp_default_value_p);
				}

			FreeMemory (param_p);
		}		/* if (param_p) */

	return NULL;
}



ResourceParameter *AllocateResourceParameterFromJSON (const json_t *param_json_p, const Service *service_p)
{
	ResourceParameter *param_p = NULL;
	Resource *current_value_p = NULL;
	const json_t *value_p = json_object_get (param_json_p, PARAM_CURRENT_VALUE_S);
	bool success_flag = true;

	if (value_p)
		{
			current_value_p = GetResourceFromJSON (value_p);

			if (!current_value_p)
				{
					success_flag = false;
				}
		}

	if (success_flag)
		{
			Resource *default_value_p = NULL;
			value_p = json_object_get (param_json_p, PARAM_DEFAULT_VALUE_S);

			if (value_p)
				{
					default_value_p = GetResourceFromJSON (value_p);

					if (!default_value_p)
						{
							success_flag = false;
						}
				}


			if (success_flag)
				{
					bool full_definition_flag = ! (IsJSONParameterConcise (param_json_p));

					if (full_definition_flag)
						{
							if (!SetTimeValueFromJSON (param_json_p, PARAM_DEFAULT_VALUE_S, &current_value_p))
								{
									success_flag = false;
								}
						}

					if (success_flag)
						{
							param_p = (ResourceParameter *) AllocMemory (sizeof (ResourceParameter));

							if (param_p)
								{
									if (InitParameterFromJSON (& (param_p -> rp_base_param), param_json_p, service_p, full_definition_flag))
										{
											SetParameterCallbacks (& (param_p -> rp_base_param), ClearResourceParameter, AddResourceParameterDetailsToJSON,
																						 GetResourceParameterDetailsFromJSON, NULL, SetResourceParameterCurrentValueFromString);

											param_p -> rp_current_value_p = current_value_p;
											param_p -> rp_default_value_p = default_value_p;

											return param_p;
										}

									FreeMemory (param_p);
								}


							if (default_value_p)
								{
									FreeResource (default_value_p);
								}
						}		/* if (SetValueFromJSON (&default_value_p, param_json_p, PARAM_DEFAULT_VALUE_S)) */

				}		/* if (SetValueFromJSON (&current_value_p, param_json_p, PARAM_CURRENT_VALUE_S)) */

		}

	if (current_value_p)
		{
			FreeResource (current_value_p);
		}

	return NULL;
}



const Resource *GetResourceParameterCurrentValue (const ResourceParameter *param_p)
{
	return param_p -> rp_current_value_p;
}


bool SetResourceParameterCurrentValue (ResourceParameter *param_p, const Resource *value_p)
{
	return SetResourceParameterValue (& (param_p -> rp_current_value_p), value_p);
}


const Resource *GetResourceParameterDefaultValue (const ResourceParameter *param_p)
{
	return param_p -> rp_default_value_p;
}


bool SetResourceParameterDefaultValue (ResourceParameter *param_p, const Resource *value_p)
{
	return SetResourceParameterValue (& (param_p -> rp_default_value_p), value_p);
}


bool IsResourceParameter (Parameter *param_p)
{
	bool resource_param_flag = false;

	switch (param_p -> pa_type)
		{
			case PT_FILE_TO_READ:
				resource_param_flag = true;
				break;

			default:
				break;
		}

	return resource_param_flag;
}

/*
 * STATIC DEFINITIONS
 */

static bool SetResourceParameterValue (Resource **param_value_pp, const Resource *new_value_p)
{
	bool success_flag = true;

	if (new_value_p)
		{
			if (*param_value_pp)
				{
					success_flag = CopyResource (new_value_p, *param_value_pp);
				}
			else
				{
					*param_value_pp = CloneResource (new_value_p);

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
					FreeReesource (*param_value_pp);
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


static bool AddResourceValueToJSON (const Resource *resource_p, json_t *param_json_p, const char *key_s)
{
	bool success_flag = false;

	if (resource_p)
		{
			json_t *res_json_p = GetResourceAsJSON (resource_p);

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
			success_flag = true;
		}

	return success_flag;
}


static bool AddResourceParameterDetailsToJSON (const Parameter *param_p, json_t *param_json_p, const bool full_definition_flag)
{
	ResourceParameter *res_param_p = (ResourceParameter *) param_p;
	bool success_flag = false;

	if (AddResourceValueToJSON (res_param_p -> rp_current_value_p, param_json_p, PARAM_CURRENT_VALUE_S))
		{
			if (AddResourceValueToJSON (res_param_p -> rp_default_value_p, param_json_p, PARAM_DEFAULT_VALUE_S))
				{
					success_flag = true;
				}
		}

	return success_flag;
}


static bool GetResourceParameterDetailsFromJSON (Parameter *param_p, const json_t *param_json_p)
{
	ResourceParameter *res_param_p = (ResourceParameter *) param_p;
	bool success_flag = true;
	uint32 i;

	if (GetJSONInteger (param_json_p, PARAM_CURRENT_VALUE_S, &i))
		{
			success_flag = SetResourceParameterCurrentValue (res_param_p, &i);
		}
	else
		{
			success_flag = SetResourceParameterCurrentValue (res_param_p, NULL);
		}

	if (success_flag)
		{
			if (GetJSONInteger (param_json_p, PARAM_DEFAULT_VALUE_S, &i))
				{
					success_flag = SetResourceParameterDefaultValue (res_param_p, &i);
				}
			else
				{
					success_flag = SetResourceParameterDefaultValue (res_param_p, NULL);
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
			Resource *resource_p = ParseStringToResource (value_s);

			if (resource_p)
				{
					success_flag = SetResourceParameterCurrentValue (res_param_p, resource_p);
					FreeResource (resource_p);
				}
		}
	else
		{
			success_flag = SetDoubleParameterCurrentValue (res_param_p, NULL);
		}

	return success_flag;
}

