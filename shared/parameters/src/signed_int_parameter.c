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
 * signed_int_parameter.c
 *
 *  Created on: 30 Jan 2020
 *      Author: billy
 */

#include "signed_int_parameter.h"
#include "memory_allocations.h"
#include "service.h"
#include "json_util.h"


/*
 * STATIC DECLARATIONS
 */

static bool SetSignedIntParameterValue (int32 **param_value_pp, const int32 *new_value_p);

static void ClearSignedIntParameter (Parameter *param_p);

static bool AddSignedIntParameterDetailsToJSON (const struct Parameter *param_p, json_t *param_json_p, const bool full_definition_flag);

static bool GetSignedIntParameterDetailsFromJSON (Parameter *param_p, const json_t *param_json_p);

static bool SetValueFromJSON (int32 **value_pp, const json_t *param_json_p, const char *key_s);

static bool SetSignedIntParameterCurrentValueFromString (Parameter *param_p, const char *value_s);

static SignedIntParameter *GetNewSignedIntParameter (const int32 *current_value_p, const int32 *default_value_p);

/*
 * API DEFINITIONS
 */

static SignedIntParameter *GetNewSignedIntParameter (const int32 *current_value_p, const int32 *default_value_p)
{
	SignedIntParameter *param_p = (SignedIntParameter *) AllocMemory (sizeof (SignedIntParameter));

	if (param_p)
		{
			param_p -> sip_current_value_p = NULL;
			param_p -> sip_default_value_p = NULL;
			param_p -> sip_min_value_p = NULL;
			param_p -> sip_max_value_p = NULL;


			if (SetSignedIntParameterValue (& (param_p -> sip_current_value_p), current_value_p))
				{
					if (SetSignedIntParameterValue (& (param_p -> sip_default_value_p), default_value_p))
						{
							return param_p;
						}

					if (param_p -> sip_current_value_p)
						{
							FreeMemory (param_p -> sip_current_value_p);
						}
				}

			FreeMemory (param_p);
		}

	return NULL;
}


SignedIntParameter *AllocateSignedIntParameter (const struct ServiceData *service_data_p, const ParameterType pt,
																																			const char * const name_s, const char * const display_name_s,
																																			const char * const description_s,
																																			const int32 *default_value_p, const int32 *current_value_p,
																																			ParameterLevel level)
{
	SignedIntParameter *param_p = GetNewSignedIntParameter (current_value_p, default_value_p);

	if (param_p)
		{
			if (InitParameter (& (param_p -> sip_base_param), service_data_p, pt, name_s, display_name_s, description_s, level,
												 ClearSignedIntParameter, AddSignedIntParameterDetailsToJSON,
												 NULL, SetSignedIntParameterCurrentValueFromString))
				{
					if (service_data_p)
						{
//									GetParameterDefaultValueFromConfig (service_data_p, name_s, param_p -> pa_type, &default_value);
						}

					return param_p;
				}

			FreeParameter (& (param_p -> sip_base_param));
		}

	return NULL;
}



SignedIntParameter *AllocateSignedIntParameterFromJSON (const json_t *param_json_p, const struct Service *service_p)
{
	SignedIntParameter *param_p = NULL;
	int32 *current_value_p = NULL;

	if (SetValueFromJSON (&current_value_p, param_json_p, PARAM_CURRENT_VALUE_S))
		{
			int32 *default_value_p = NULL;
			bool success_flag = true;
			bool full_definition_flag = ! (IsJSONParameterConcise (param_json_p));

			if (full_definition_flag)
				{
					if (!SetValueFromJSON (&default_value_p, param_json_p, PARAM_DEFAULT_VALUE_S))
						{
							success_flag = false;
						}
				}

			if (success_flag)
				{
					param_p = GetNewSignedIntParameter (current_value_p, default_value_p);

					if (param_p)
						{
							if (InitParameterFromJSON (& (param_p -> sip_base_param), param_json_p, service_p, full_definition_flag))
								{
									SetParameterCallbacks (& (param_p -> sip_base_param), ClearSignedIntParameter, AddSignedIntParameterDetailsToJSON,
																				 NULL,
																				 SetSignedIntParameterCurrentValueFromString);

									return param_p;
								}

							ClearSignedIntParameter (& (param_p -> sip_base_param));
							FreeMemory (param_p);
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


Parameter *EasyCreateAndAddSignedIntParameterToParameterSet (const ServiceData *service_data_p, ParameterSet *params_p, ParameterGroup *group_p, ParameterType type,
																								const char * const name_s, const char * const display_name_s, const char * const description_s,
																								const int32 *default_value_p, uint8 level)
{
	return CreateAndAddSignedIntParameterToParameterSet (service_data_p, params_p, group_p, type, name_s, display_name_s, description_s, default_value_p, NULL, level);
}


Parameter *CreateAndAddSignedIntParameterToParameterSet (const ServiceData *service_data_p, ParameterSet *params_p, ParameterGroup *group_p,  ParameterType type,
																								const char * const name_s, const char * const display_name_s, const char * const description_s,
																								const int32 *default_value_p, const int32 *current_value_p, uint8 level)
{
	SignedIntParameter *int_param_p = AllocateSignedIntParameter (service_data_p, type, name_s, display_name_s, description_s, default_value_p, current_value_p, level);

	if (int_param_p)
		{
			Parameter *base_param_p = & (int_param_p -> sip_base_param);

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

		}		/* if (bool_param_p) */

	return NULL;
}




const int32 *GetSignedIntParameterCurrentValue (const SignedIntParameter *param_p)
{
	return param_p -> sip_current_value_p;
}


bool SetSignedIntParameterCurrentValue (SignedIntParameter *param_p, const int32 *value_p)
{
	return SetSignedIntParameterValue (& (param_p -> sip_current_value_p), value_p);
}


const int32 *GetSignedIntParameterDefaultValue (const SignedIntParameter *param_p)
{
	return param_p -> sip_default_value_p;
}


bool SetSignedIntParameterDefaultValue (SignedIntParameter *param_p, const int32 *value_p)
{
	return SetSignedIntParameterValue (& (param_p -> sip_default_value_p), value_p);
}


bool SetSignedIntParameterBounds (SignedIntParameter *param_p, const int32 min_value, const int32 max_value)
{
	if (! (param_p -> sip_min_value_p))
		{
			param_p -> sip_min_value_p = (int32 *) AllocMemory (sizeof (int32));

			if (! (param_p -> sip_min_value_p))
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate min value");
					return false;
				}
		}

	if (! (param_p -> sip_max_value_p))
		{
			param_p -> sip_max_value_p = (int32 *) AllocMemory (sizeof (int32));

			if (! (param_p -> sip_max_value_p))
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate max value");
					return false;
				}
		}

	* (param_p -> sip_min_value_p) = min_value;
	* (param_p -> sip_max_value_p) = max_value;

	return true;
}


bool IsSignedIntParameterBounded (const SignedIntParameter *param_p)
{
	return ((param_p -> sip_min_value_p) && (param_p -> sip_max_value_p));
}





bool GetSignedIntParameterBounds (const SignedIntParameter *param_p, int32 *min_p, int32 *max_p)
{
	bool success_flag = false;

	if (IsSignedIntParameterBounded (param_p))
		{
			*min_p = * (param_p -> sip_min_value_p);
			*max_p = * (param_p -> sip_max_value_p);

			success_flag = true;
		}

	return success_flag;
}


bool IsSignedIntParameter (const Parameter *param_p)
{
	bool signed_int_param_flag = false;

	switch (param_p -> pa_type)
		{
			case PT_SIGNED_INT:
			case PT_NEGATIVE_INT:
				signed_int_param_flag = true;
				break;

			default:
				break;
		}

	return signed_int_param_flag;
}


bool GetCurrentSignedIntParameterValueFromParameterSet (const ParameterSet * const params_p, const char * const name_s, const int32 **value_pp)
{
	bool success_flag = false;
	Parameter *param_p = GetParameterFromParameterSetByName (params_p, name_s);

	if (param_p)
		{
			if (IsSignedIntParameter (param_p))
				{
					*value_pp = GetSignedIntParameterCurrentValue ((const SignedIntParameter *) param_p);
					success_flag = true;
				}
		}

	return success_flag;
}


/*
 * STATIC DEFINITIONS
 */

static bool SetSignedIntParameterValue (int32 **param_value_pp, const int32 *new_value_p)
{
	bool success_flag = true;

	if (new_value_p)
		{
			if (! (*param_value_pp))
				{
					*param_value_pp = (int32 *) AllocMemory (sizeof (int32));

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



static void ClearSignedIntParameter (Parameter *param_p)
{
	SignedIntParameter *int_param_p = (SignedIntParameter *) param_p;

	if (int_param_p -> sip_current_value_p)
		{
			FreeMemory (int_param_p -> sip_current_value_p);
			int_param_p -> sip_current_value_p = NULL;
		}

	if (int_param_p -> sip_default_value_p)
		{
			FreeMemory (int_param_p -> sip_default_value_p);
			int_param_p -> sip_current_value_p = NULL;
		}

	if (int_param_p -> sip_min_value_p)
		{
			FreeMemory (int_param_p -> sip_min_value_p);
			int_param_p -> sip_min_value_p = NULL;
		}

	if (int_param_p -> sip_max_value_p)
		{
			FreeMemory (int_param_p -> sip_max_value_p);
			int_param_p -> sip_max_value_p = NULL;
		}
}


static bool AddSignedIntParameterDetailsToJSON (const struct Parameter *param_p, json_t *param_json_p, const bool full_definition_flag)
{
	SignedIntParameter *int_param_p = (SignedIntParameter *) param_p;
	bool success_flag = false;

	if (int_param_p -> sip_current_value_p != NULL)
		{
			success_flag = SetJSONInteger (param_json_p, PARAM_CURRENT_VALUE_S, * (int_param_p -> sip_current_value_p));
		}
	else
		{
			success_flag = SetJSONNull (param_json_p, PARAM_CURRENT_VALUE_S);
		}

	if (full_definition_flag)
		{
			success_flag = false;

			if ((int_param_p -> sip_default_value_p == NULL ) || (SetJSONInteger (param_json_p, PARAM_DEFAULT_VALUE_S, * (int_param_p -> sip_default_value_p))))
				{
					if ((int_param_p -> sip_min_value_p == NULL ) || (SetJSONInteger (param_json_p, PARAM_MIN_S, * (int_param_p -> sip_min_value_p))))
						{
							if ((int_param_p -> sip_max_value_p == NULL ) || (SetJSONInteger (param_json_p, PARAM_MAX_S, * (int_param_p -> sip_max_value_p))))
								{
									success_flag = true;
								}
						}
				}

		}

	return success_flag;
}


static bool GetSignedIntParameterDetailsFromJSON (Parameter *param_p, const json_t *param_json_p)
{
	SignedIntParameter *int_param_p = (SignedIntParameter *) param_p;
	bool success_flag = false;

	if (SetValueFromJSON (& (int_param_p -> sip_current_value_p), param_json_p, PARAM_CURRENT_VALUE_S))
		{
			if (SetValueFromJSON (& (int_param_p -> sip_default_value_p), param_json_p, PARAM_DEFAULT_VALUE_S))
				{
					if (SetValueFromJSON (& (int_param_p -> sip_min_value_p), param_json_p, PARAM_MIN_S))
						{
							if (SetValueFromJSON (& (int_param_p -> sip_max_value_p), param_json_p, PARAM_MAX_S))
								{
									success_flag = true;
								}
						}
				}
		}

	return success_flag;
}


static bool SetValueFromJSON (int32 **value_pp, const json_t *param_json_p, const char *key_s)
{
	bool success_flag = false;
	const json_t *value_p = json_object_get (param_json_p, key_s);

	if (value_p)
		{
			if (json_is_integer (value_p))
				{
					int32 i = json_integer_value (value_p);
					success_flag = SetSignedIntParameterValue (value_pp, &i);
				}
			else if (json_is_null (value_p))
				{
					success_flag = SetSignedIntParameterValue (value_pp, NULL);
				}
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, value_p, "JSON value is of the wrong type, %d not integer", value_p -> type);
				}
		}
	else
		{
			success_flag = SetSignedIntParameterValue (value_pp, NULL);
		}

	return success_flag;
}


static bool SetSignedIntParameterCurrentValueFromString (Parameter *param_p, const char *value_s)
{
	bool success_flag = false;
	SignedIntParameter *int_param_p = (SignedIntParameter *) param_p;
	int32 *value_p = NULL;
	int32 value = 0;

	if (value_s)
		{
			if (sscanf (value_s, UINT32_FMT, &value) > 0)
				{
					value_p = &value;
				}
		}

	success_flag = SetSignedIntParameterCurrentValue (int_param_p, value_p);

	return success_flag;
}

