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

static bool AddSignedIntParameterDetailsToJSON (const Parameter *param_p, json_t *param_json_p);

static bool GetSignedIntParameterDetailsFromJSON (Parameter *param_p, const json_t *param_json_p);

static bool SetValueFromJSON (int32 **value_pp, const json_t *param_json_p, const char *key_s);

static bool SetSignedIntParameterCurrentValueFromString (SignedIntParameter *param_p, const char *value_s);

/*
 * API DEFINITIONS
 */

SignedIntParameter *AllocateSignedIntParameter (const struct ServiceData *service_data_p, const ParameterType pt, const ParameterType pt, const char * const name_s, const char * const display_name_s, const char * const description_s, LinkedList *options_p, int32 *default_value_p, int32 *current_value_p, ParameterLevel level)
{
	SignedIntParameter *param_p = (SignedIntParameter *) AllocMemory (sizeof (SignedIntParameter));

	if (param_p)
		{
			bool success_flag = true;

			if (current_value_p)
				{
					param_p -> sip_current_value_p = (int32 *) AllocMemory (sizeof (int32));

					if (param_p -> sip_current_value_p)
						{
							* (param_p -> sip_current_value_p) = *current_value_p;
						}
					else
						{
							success_flag = false;
						}
				}
			else
				{
					param_p -> sip_current_value_p = NULL;
				}

			if (success_flag)
				{
					if (default_value_p)
						{
							param_p -> sip_default_value_p = (int32 *) AllocMemory (sizeof (int32));

							if (param_p -> sip_default_value_p)
								{
									* (param_p -> sip_default_value_p) = *default_value_p;
								}
							else
								{
									success_flag = false;
								}
						}
					else
						{
							param_p -> sip_default_value_p = NULL;
						}
				}

			if (success_flag)
				{
					if (InitParameter (& (param_p -> sip_base_param), service_data_p, pt, name_s, display_name_s, description_s, options_p, level,
														 ClearSignedIntParameter, AddSignedIntParameterDetailsToJSON, GetSignedIntParameterDetailsFromJSON,
														 NULL, SetSignedIntParameterCurrentValueFromString))
						{
							if (service_data_p)
								{
//									GetParameterDefaultValueFromConfig (service_data_p, name_s, param_p -> pa_type, &default_value);
								}

							param_p -> sip_min_value_p = NULL;
							param_p -> sip_max_value_p = NULL;

							return param_p;
						}
				}

			if (param_p -> sip_current_value_p)
				{
					FreeMemory (param_p -> sip_current_value_p);
				}

			if (param_p -> sip_default_value_p)
				{
					FreeMemory (param_p -> sip_default_value_p);
				}

			FreeMemory (param_p);
		}		/* if (param_p) */

	return NULL;
}


SignedIntParameter *AllocateSignedIntParameterFromJSON (const json_t *param_json_p, const struct Service *service_p)
{
	SignedIntParameter *param_p = NULL;
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
					param_p = (SignedIntParameter *) AllocMemory (sizeof (SignedIntParameter));

					if (param_p)
						{
							if (InitParameterFromJSON (& (param_p -> sip_base_param), param_json_p, service_p, full_definition_flag))
								{
									SetParameterCallbacks (& (param_p -> sip_base_param), ClearSignedIntParameter, AddSignedIntParameterDetailsToJSON, GetSignedIntParameterDetailsFromJSON, NULL,
																				 SetSignedIntParameterCurrentValueFromString);

									param_p -> sip_current_value_p = current_value_p;
									param_p -> sip_default_value_p = default_value_p;

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


bool GetSignedIntParameterrBounds (const SignedIntParameter *param_p, int32 *min_p, int32 *max_p)
{
	bool success_flag = false;

	if (IsDoubleParameterBounded (param_p))
		{
			*min_p = * (param_p -> sip_min_value_p);
			*max_p = * (param_p -> sip_max_value_p);

			success_flag = true;
		}

	return success_flag;
}


bool SetSignedIntParameterBounds (SignedIntParameter *param_p, const int32 min_value, const int32 max_value)
{
	if (! (param_p -> sip_min_value_p))
		{
			param_p -> sip_min_value_p = (uint32 *) AllocMemory (sizeof (uint32));

			if (! (param_p -> sip_min_value_p))
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate min value");
					return false;
				}
		}

	if (! (param_p -> sip_max_value_p))
		{
			param_p -> sip_max_value_p = (uint32 *) AllocMemory (sizeof (uint32));

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


bool IsSignedIntParameter (Parameter *param_p)
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


bool GetCurrentSignedIntParameterValueFromParameterSet (const ParameterSet * const params_p, const char * const name_s, int32 *value_p)
{
	bool success_flag = false;
	Parameter *param_p = GetParameterFromParameterSetByName (params_p, name_s);

	if (param_p)
		{
			if (IsSignedParameter (param_p))
				{
					const int32 *current_value_p = GetSignedIntParameterCurrentValue ((const SignedIntParameter *) param_p);

					*value_p = *current_value_p;
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

	if ((int_param_p -> sip_current_value_p == NULL ) || (SetJSONInteger (param_json_p, PARAM_CURRENT_VALUE_S, * (int_param_p -> sip_current_value_p))))
		{
			if (full_definition_flag)
				{
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
			else
				{
					success_flag = true;
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
	int32 i;

	if (GetJSONInteger (param_json_p, key_s, &i))
		{
			success_flag = SetSignedIntParameterValue (value_pp, &i);
		}
	else
		{
			success_flag = SetSignedIntParameterValue (value_pp, NULL);
		}

	return success_flag;
}


static bool SetSignedIntParameterCurrentValueFromString (SignedIntParameter *param_p, const char *value_s)
{
	bool success_flag = false;

	if (value_s)
		{
			int32 value = 0;

			if (sscanf (value_s, INT32_FMT, &value) > 0)
				{
					success_flag = SetSignedIntParameterCurrentValue (param_p, &value);
				}
		}
	else
		{
			success_flag = SetSignedIntParameterCurrentValue (param_p, NULL);
		}

	return success_flag;
}


