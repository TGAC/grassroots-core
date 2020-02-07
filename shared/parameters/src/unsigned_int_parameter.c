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
 * unsigned_int_parameter.c
 *
 *  Created on: 31 Jan 2020
 *      Author: billy
 */

#include "unsigned_int_parameter.h"
#include "memory_allocations.h"
#include "service.h"
#include "json_util.h"


/*
 * STATIC DECLARATIONS
 */

static bool SetUnsignedIntParameterValue (uint32 **param_value_pp, const uint32 *new_value_p);

static void ClearUnsignedIntParameter (Parameter *param_p);

static bool AddUnsignedIntParameterDetailsToJSON (const Parameter *param_p, json_t *param_json_p, const bool full_definition_flag);

static bool GetUnsignedIntParameterDetailsFromJSON (Parameter *param_p, const json_t *param_json_p);

static bool SetValueFromJSON (uint32 **value_pp, const json_t *param_json_p, const char *key_s);


static bool SetUnsignedIntParameterCurrentValueFromString (Parameter *param_p, const char *value_s);



/*
 * API DEFINITIONS
 */

UnsignedIntParameter *AllocateUnsignedIntParameter (const struct ServiceData *service_data_p, const char * const name_s, const char * const display_name_s, const char * const description_s, LinkedList *options_p, uint32 *default_value_p, uint32 *current_value_p, ParameterLevel level)
{
	UnsignedIntParameter *param_p = (UnsignedIntParameter *) AllocMemory (sizeof (UnsignedIntParameter));

	if (param_p)
		{
			bool success_flag = true;

			if (current_value_p)
				{
					param_p -> uip_current_value_p = (uint32 *) AllocMemory (sizeof (uint32));

					if (param_p -> uip_current_value_p)
						{
							* (param_p -> uip_current_value_p) = *current_value_p;
						}
					else
						{
							success_flag = false;
						}
				}
			else
				{
					param_p -> uip_current_value_p = NULL;
				}


			if (success_flag)
				{
					if (default_value_p)
						{
							param_p -> uip_default_value_p = (uint32 *) AllocMemory (sizeof (uint32));

							if (param_p -> uip_default_value_p)
								{
									* (param_p -> uip_default_value_p) = *default_value_p;
								}
							else
								{
									success_flag = false;
								}
						}
					else
						{
							param_p -> uip_default_value_p = NULL;
						}
				}

			if (success_flag)
				{
					if (InitParameter (& (param_p -> uip_base_param), service_data_p, PT_UNSIGNED_INT, name_s, display_name_s, description_s, options_p, level,
														 ClearUnsignedIntParameter, AddUnsignedIntParameterDetailsToJSON, GetUnsignedIntParameterDetailsFromJSON,
														 NULL, SetUnsignedIntParameterCurrentValueFromString))
						{
							if (service_data_p)
								{
//									GetParameterDefaultValueFromConfig (service_data_p, name_s, param_p -> pa_type, &default_value);
								}

							param_p -> uip_min_value_p = NULL;
							param_p -> uip_max_value_p = NULL;

							return param_p;
						}
				}

			if (param_p -> uip_current_value_p)
				{
					FreeMemory (param_p -> uip_current_value_p);
				}

			if (param_p -> uip_default_value_p)
				{
					FreeMemory (param_p -> uip_default_value_p);
				}

			FreeMemory (param_p);
		}		/* if (param_p) */

	return NULL;
}



UnsignedIntParameter *AllocateUnsignedIntParameterFromJSON (const json_t *param_json_p, const struct Service *service_p)
{
	UnsignedIntParameter *param_p = NULL;
	uint32 *current_value_p = NULL;

	if (SetValueFromJSON (&current_value_p, param_json_p, PARAM_CURRENT_VALUE_S))
		{
			uint32 *default_value_p = NULL;
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
					param_p = (UnsignedIntParameter *) AllocMemory (sizeof (UnsignedIntParameter));

					if (param_p)
						{
							if (InitParameterFromJSON (& (param_p -> uip_base_param), param_json_p, service_p, full_definition_flag))
								{
									SetParameterCallbacks (& (param_p -> uip_base_param), ClearUnsignedIntParameter, AddUnsignedIntParameterDetailsToJSON,
																				 GetUnsignedIntParameterDetailsFromJSON, NULL, SetUnsignedIntParameterCurrentValueFromString);

									param_p -> uip_current_value_p = current_value_p;
									param_p -> uip_default_value_p = default_value_p;

									return param_p;
								}

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



const uint32 *GetUnsignedIntParameterCurrentValue (const UnsignedIntParameter *param_p)
{
	return param_p -> uip_current_value_p;
}


bool SetUnsignedIntParameterCurrentValue (UnsignedIntParameter *param_p, const uint32 *value_p)
{
	return SetUnsignedIntParameterValue (& (param_p -> uip_current_value_p), value_p);
}


const uint32 *GetUnsignedIntParameterDefaultValue (const UnsignedIntParameter *param_p)
{
	return param_p -> uip_default_value_p;
}


bool SetUnsignedIntParameterDefaultValue (UnsignedIntParameter *param_p, const uint32 *value_p)
{
	return SetUnsignedIntParameterValue (& (param_p -> uip_default_value_p), value_p);
}


bool SetUnsignedIntParameterBounds (UnsignedIntParameter *param_p, const uint32 min_value, const uint32 max_value)
{
	if (! (param_p -> uip_min_value_p))
		{
			param_p -> uip_min_value_p = (uint32 *) AllocMemory (sizeof (uint32));

			if (! (param_p -> uip_min_value_p))
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate min value");
					return false;
				}
		}

	if (! (param_p -> uip_max_value_p))
		{
			param_p -> uip_max_value_p = (uint32 *) AllocMemory (sizeof (uint32));

			if (! (param_p -> uip_max_value_p))
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate max value");
					return false;
				}
		}

	* (param_p -> uip_min_value_p) = min_value;
	* (param_p -> uip_max_value_p) = max_value;

	return true;
}


bool IsUnsignedIntParameterBounded (const UnsignedIntParameter *param_p)
{
	return ((param_p -> uip_min_value_p) && (param_p -> uip_max_value_p));
}


bool GetUnsignedIntParameterBounds (const UnsignedIntParameter *param_p, uint32 *min_p, uint32 *max_p)
{
	bool success_flag = false;

	if (IsUnsignedIntParameterBounded (param_p))
		{
			*min_p = * (param_p -> uip_min_value_p);
			*max_p = * (param_p -> uip_max_value_p);

			success_flag = true;
		}

	return success_flag;
}


bool IsUnsignedIntParameter (Parameter *param_p)
{
	bool unsigned_int_param_flag = false;

	switch (param_p -> pa_type)
		{
			case PT_UNSIGNED_INT:
				unsigned_int_param_flag = true;
				break;

			default:
				break;
		}

	return unsigned_int_param_flag;
}



bool GetCurrentUnsignedIntParameterValueFromParameterSet (const ParameterSet * const params_p, const char * const name_s, uint32 *value_p)
{
	bool success_flag = false;
	Parameter *param_p = GetParameterFromParameterSetByName (params_p, name_s);

	if (param_p)
		{
			if (IsUnsignedParameter (param_p))
				{
					const uint32 *current_value_p = GetUnsignedIntParameterCurrentValue ((const UnsignedIntParameter *) param_p);

					*value_p = *current_value_p;
					success_flag = true;
				}
		}

	return success_flag;
}

/*
 * STATIC DEFINITIONS
 */

static bool SetUnsignedIntParameterValue (uint32 **param_value_pp, const uint32 *new_value_p)
{
	bool success_flag = true;

	if (new_value_p)
		{
			if (! (*param_value_pp))
				{
					*param_value_pp = (uint32 *) AllocMemory (sizeof (uint32));

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



static void ClearUnsignedIntParameter (Parameter *param_p)
{
	UnsignedIntParameter *int_param_p = (UnsignedIntParameter *) param_p;

	if (int_param_p -> uip_current_value_p)
		{
			FreeMemory (int_param_p -> uip_current_value_p);
			int_param_p -> uip_current_value_p = NULL;
		}

	if (int_param_p -> uip_default_value_p)
		{
			FreeMemory (int_param_p -> uip_default_value_p);
			int_param_p -> uip_current_value_p = NULL;
		}

	if (int_param_p -> uip_min_value_p)
		{
			FreeMemory (int_param_p -> uip_min_value_p);
			int_param_p -> uip_min_value_p = NULL;
		}

	if (int_param_p -> uip_max_value_p)
		{
			FreeMemory (int_param_p -> uip_max_value_p);
			int_param_p -> uip_max_value_p = NULL;
		}

}


static bool AddUnsignedIntParameterDetailsToJSON (const Parameter *param_p, json_t *param_json_p, const bool full_definition_flag)
{
	UnsignedIntParameter *int_param_p = (UnsignedIntParameter *) param_p;
	bool success_flag = false;

	if ((int_param_p -> uip_current_value_p == NULL ) || (SetJSONInteger (param_json_p, PARAM_CURRENT_VALUE_S, * (int_param_p -> uip_current_value_p))))
		{
			if (full_definition_flag)
				{
					if ((int_param_p -> uip_default_value_p == NULL ) || (SetJSONInteger (param_json_p, PARAM_DEFAULT_VALUE_S, * (int_param_p -> uip_default_value_p))))
						{
							if ((int_param_p -> uip_min_value_p == NULL ) || (SetJSONInteger (param_json_p, PARAM_MIN_S, * (int_param_p -> uip_min_value_p))))
								{
									if ((int_param_p -> uip_max_value_p == NULL ) || (SetJSONInteger (param_json_p, PARAM_MAX_S, * (int_param_p -> uip_max_value_p))))
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



static bool GetUnsignedIntParameterDetailsFromJSON (Parameter *param_p, const json_t *param_json_p)
{
	UnsignedIntParameter *int_param_p = (UnsignedIntParameter *) param_p;
	bool success_flag = false;

	if (SetValueFromJSON (& (int_param_p -> uip_current_value_p), param_json_p, PARAM_CURRENT_VALUE_S))
		{
			if (SetValueFromJSON (& (int_param_p -> uip_default_value_p), param_json_p, PARAM_DEFAULT_VALUE_S))
				{
					if (SetValueFromJSON (& (int_param_p -> uip_min_value_p), param_json_p, PARAM_MIN_S))
						{
							if (SetValueFromJSON (& (int_param_p -> uip_max_value_p), param_json_p, PARAM_MAX_S))
								{
									success_flag = true;
								}
						}
				}
		}

	return success_flag;
}



static bool SetValueFromJSON (uint32 **value_pp, const json_t *param_json_p, const char *key_s)
{
	bool success_flag = false;
	int32 i;

	if (GetJSONInteger (param_json_p, key_s, &i))
		{
			if (i >= 0)
				{
					uint32 u = (uint32) i;
					success_flag = SetUnsignedIntParameterValue (value_pp, &u);
				}
		}
	else
		{
			success_flag = SetUnsignedIntParameterValue (value_pp, NULL);
		}

	return success_flag;
}


static bool SetUnsignedIntParameterCurrentValueFromString (Parameter *param_p, const char *value_s)
{
	bool success_flag = false;
	UnsignedIntParameter *int_param_p = (UnsignedIntParameter *) param_p;
	uint32 *value_p = NULL;
	uint32 value = 0;

	if (value_s)
		{
			if (sscanf (value_s, UINT32_FMT, &value) > 0)
				{
					value_p = &value;
				}
		}

	success_flag = SetUnsignedIntParameterCurrentValue (int_param_p, value_p);

	return success_flag;
}

