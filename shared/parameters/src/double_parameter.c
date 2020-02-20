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
 * double_parameter.c
 *
 *  Created on: 30 Jan 2020
 *      Author: billy
 */

#include "double_parameter.h"
#include "memory_allocations.h"
#include "service.h"
#include "json_util.h"


/*
 * STATIC DECLARATIONS
 */

static bool SetDoubleParameterValue (double64 **param_value_pp, const double64 *new_value_p);

static void ClearDoubleParameter (Parameter *param_p);

static bool AddDoubleParameterDetailsToJSON (const struct Parameter *param_p, json_t *param_json_p, const bool full_definition_flag);

static bool GetDoubleParameterDetailsFromJSON (Parameter *param_p, const json_t *param_json_p);

static bool SetValueFromJSON (double **value_pp, const json_t *param_json_p, const char *key_s);

static bool SetDoubleParameterCurrentValueFromString (Parameter *param_p, const char *value_s);

static DoubleParameter *GetNewDoubleParameter (const double64 *current_value_p, const double64 *default_value_p);

static bool AddDoubleValueToJSON (json_t *param_json_p, const char *key_s, const double64 *value_p);

/*
 * API DEFINITIONS
 */



DoubleParameter *AllocateDoubleParameter (const struct ServiceData *service_data_p, const ParameterType pt, const char * const name_s, const char * const display_name_s, const char * const description_s, const double64 *default_value_p, const double64 *current_value_p, ParameterLevel level)
{
	DoubleParameter *param_p = GetNewDoubleParameter (current_value_p, default_value_p);

	if (param_p)
		{
			if (InitParameter (& (param_p -> dp_base_param), service_data_p, pt, name_s, display_name_s, description_s, level,
												 ClearDoubleParameter, AddDoubleParameterDetailsToJSON,
												 NULL, SetDoubleParameterCurrentValueFromString))
				{
					if (service_data_p)
						{
//									GetParameterDefaultValueFromConfig (service_data_p, name_s, param_p -> pa_type, &default_value);
						}

					return param_p;
				}

			FreeParameter (& (param_p -> dp_base_param));
		}

	return NULL;
}




DoubleParameter *AllocateDoubleParameterFromJSON (const json_t *param_json_p, const struct Service *service_p)
{
	double64 *current_value_p = NULL;

	if (SetValueFromJSON (&current_value_p, param_json_p, PARAM_CURRENT_VALUE_S))
		{
			double64 *default_value_p = NULL;
			bool success_flag = true;
			bool full_definition_flag = ! (IsJSONParameterConcise (param_json_p));
			DoubleParameter *param_p = GetNewDoubleParameter (current_value_p, default_value_p);

			if (param_p)
				{
					Parameter *base_param_p = & (param_p -> dp_base_param);

					if (InitParameterFromJSON (& (param_p -> dp_base_param), param_json_p, service_p, full_definition_flag))
						{
							if (GetDoubleParameterDetailsFromJSON (param_p, param_json_p))
								{
									SetParameterCallbacks (& (param_p -> dp_base_param), ClearDoubleParameter, AddDoubleParameterDetailsToJSON, NULL,
																							 SetDoubleParameterCurrentValueFromString);

									return param_p;
								}
							else
								{
									FreeParameter (base_param_p);
								}
						}
					else
						{
							ClearDoubleParameter (& (param_p -> dp_base_param));
							FreeMemory (param_p);
						}
				}

		}		/* if (SetValueFromJSON (&current_value_p, param_json_p, PARAM_CURRENT_VALUE_S)) */

	return NULL;
}




const double64 *GetDoubleParameterCurrentValue (const DoubleParameter *param_p)
{
	return param_p -> dp_current_value_p;
}


bool SetDoubleParameterCurrentValue (DoubleParameter *param_p, const double64 *value_p)
{
	return SetDoubleParameterValue (& (param_p -> dp_current_value_p), value_p);
}


const double64 *GetDoubleParameterDefaultValue (const DoubleParameter *param_p)
{
	return param_p -> dp_default_value_p;
}


bool SetDoubleParameterDefaultValue (DoubleParameter *param_p, const double64 *value_p)
{
	return SetDoubleParameterValue (& (param_p -> dp_default_value_p), value_p);
}


bool SetDoubleParameterMinimumValue (DoubleParameter *param_p, const double64 *min_value_p)
{
	return SetDoubleParameterValue (& (param_p -> dp_min_value_p), min_value_p);
}


bool SetDoubleParameterMaximumValue (DoubleParameter *param_p, const double64 *max_value_p)
{
	return SetDoubleParameterValue (& (param_p -> dp_max_value_p), max_value_p);
}



bool SetDoubleParameterBounds (DoubleParameter *param_p, const double64 *min_value_p, const double64 *max_value_p)
{
	if (SetDoubleParameterMinimumValue (param_p, min_value_p))
		{
			if (SetDoubleParameterMaximumValue (param_p, max_value_p))
				{
					return true;
				}
		}

	return false;
}


bool IsDoubleParameterBounded (const DoubleParameter *param_p)
{
	return ((param_p -> dp_min_value_p) && (param_p -> dp_max_value_p));
}


bool GetDoubleParameterBounds (const DoubleParameter *param_p, double64 *min_value_p,  double64 *max_value_p)
{
	bool success_flag = false;

	if (IsDoubleParameterBounded (param_p))
		{
			*min_value_p = * (param_p -> dp_min_value_p);
			*max_value_p = * (param_p -> dp_max_value_p);

			success_flag = true;
		}

	return success_flag;
}


bool IsDoubleParameter (const Parameter *param_p)
{
	bool double_param_flag = false;

	switch (param_p -> pa_type)
		{
			case PT_SIGNED_REAL:
			case PT_UNSIGNED_REAL:
				double_param_flag = true;
				break;

			default:
				break;
		}

	return double_param_flag;
}


bool GetCurrentDoubleParameterValueFromParameterSet (const ParameterSet * const params_p, const char * const name_s, const double64 **value_pp)
{
	bool success_flag = false;
	Parameter *param_p = GetParameterFromParameterSetByName (params_p, name_s);

	if (param_p)
		{
			if (IsDoubleParameter (param_p))
				{
					*value_pp = GetDoubleParameterCurrentValue ((const DoubleParameter *) param_p);
					success_flag = true;
				}
		}

	return success_flag;
}



Parameter *EasyCreateAndAddDoubleParameterToParameterSet (const struct ServiceData *service_data_p, ParameterSet *params_p, ParameterGroup *group_p, const ParameterType pt,
																								const char * const name_s, const char * const display_name_s, const char * const description_s,
																								const double64 *default_value_p, uint8 level)
{
	return CreateAndAddDoubleParameterToParameterSet (service_data_p, params_p, group_p, pt, name_s, display_name_s, description_s, default_value_p, NULL, level);
}


Parameter *CreateAndAddDoubleParameterToParameterSet (const ServiceData *service_data_p, ParameterSet *params_p, ParameterGroup *group_p, const ParameterType pt,
																								const char * const name_s, const char * const display_name_s, const char * const description_s,
																								const double64 *default_value_p, const double64 *current_value_p, uint8 level)
{
	DoubleParameter *double_param_p = AllocateDoubleParameter (service_data_p, pt, name_s, display_name_s, description_s, default_value_p, current_value_p, level);

	if (double_param_p)
		{
			Parameter *base_param_p = & (double_param_p -> dp_base_param);

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

static bool SetDoubleParameterValue (double64 **param_value_pp, const double64 *new_value_p)
{
	bool success_flag = true;

	if (new_value_p)
		{
			if (! (*param_value_pp))
				{
					*param_value_pp = (double64 *) AllocMemory (sizeof (double64));

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



static void ClearDoubleParameter (Parameter *param_p)
{
	DoubleParameter *double_param_p = (DoubleParameter *) param_p;

	if (double_param_p -> dp_current_value_p)
		{
			FreeMemory (double_param_p -> dp_current_value_p);
			double_param_p -> dp_current_value_p = NULL;
		}

	if (double_param_p -> dp_default_value_p)
		{
			FreeMemory (double_param_p -> dp_default_value_p);
			double_param_p -> dp_current_value_p = NULL;
		}

	if (double_param_p -> dp_min_value_p)
		{
			FreeMemory (double_param_p -> dp_min_value_p);
			double_param_p -> dp_min_value_p = NULL;
		}

	if (double_param_p -> dp_max_value_p)
		{
			FreeMemory (double_param_p -> dp_max_value_p);
			double_param_p -> dp_max_value_p = NULL;
		}

}


static bool AddDoubleParameterDetailsToJSON (const struct Parameter *param_p, json_t *param_json_p, const bool full_definition_flag)
{
	DoubleParameter *double_param_p = (DoubleParameter *) param_p;
	bool success_flag = false;

	if (AddDoubleValueToJSON (param_json_p, PARAM_CURRENT_VALUE_S, double_param_p -> dp_current_value_p))
		{
			if (full_definition_flag)
				{
					if (AddDoubleValueToJSON (param_json_p, PARAM_DEFAULT_VALUE_S, double_param_p -> dp_default_value_p))
						{
							if (AddDoubleValueToJSON (param_json_p, PARAM_MIN_S, double_param_p -> dp_min_value_p))
								{
									if (AddDoubleValueToJSON (param_json_p, PARAM_MAX_S, double_param_p -> dp_max_value_p))
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


static bool AddDoubleValueToJSON (json_t *param_json_p, const char *key_s, const double64 *value_p)
{
	bool success_flag = false;

	if (value_p)
		{
			success_flag = SetJSONReal (param_json_p, key_s, *value_p);
		}
	else
		{
			json_t *null_p = json_null ();

			if (json_object_set (param_json_p, key_s, null_p) == 0)
				{
					success_flag = true;
				}
		}

	return success_flag;
}


static bool GetDoubleParameterDetailsFromJSON (Parameter *param_p, const json_t *param_json_p)
{
	DoubleParameter *double_param_p = (DoubleParameter *) param_p;
	bool success_flag = false;

	if (SetValueFromJSON (& (double_param_p -> dp_current_value_p), param_json_p, PARAM_CURRENT_VALUE_S))
		{
			if (SetValueFromJSON (& (double_param_p -> dp_default_value_p), param_json_p, PARAM_DEFAULT_VALUE_S))
				{
					if (SetValueFromJSON (& (double_param_p -> dp_min_value_p), param_json_p, PARAM_MIN_S))
						{
							if (SetValueFromJSON (& (double_param_p -> dp_max_value_p), param_json_p, PARAM_MAX_S))
								{
									success_flag = true;
								}
						}
				}
		}

	return success_flag;
}


static bool SetValueFromJSON (double **value_pp, const json_t *param_json_p, const char *key_s)
{
	bool success_flag = false;
	const json_t *value_p = json_object_get (param_json_p, key_s);

	if (value_p)
		{
			if (json_is_number (value_p))
				{
					double d = json_number_value (value_p);
					success_flag = SetDoubleParameterValue (value_pp, &d);
				}
			else if (json_is_null (value_p))
				{
					success_flag = SetDoubleParameterValue (value_pp, NULL);
				}
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, value_p, "JSON value is of the wrong type, %d not double", value_p -> type);
				}
		}
	else
		{
			success_flag = SetDoubleParameterValue (value_pp, NULL);
		}

	return success_flag;
}


static bool SetDoubleParameterCurrentValueFromString (Parameter *param_p, const char *value_s)
{
	DoubleParameter *double_param_p = (DoubleParameter *) param_p;
	bool success_flag = false;

	if (value_s)
		{
			double64 value;

			if (sscanf (value_s, DOUBLE64_FMT, &value) > 0)
				{
					success_flag = SetDoubleParameterCurrentValue (double_param_p, &value);
				}
		}
	else
		{
			success_flag = SetDoubleParameterCurrentValue (double_param_p, NULL);
		}

	return success_flag;
}



static DoubleParameter *GetNewDoubleParameter (const double64 *current_value_p, const double64 *default_value_p)
{
	DoubleParameter *param_p = (DoubleParameter *) AllocMemory (sizeof (DoubleParameter));

	if (param_p)
		{
			param_p -> dp_current_value_p = NULL;
			param_p -> dp_default_value_p = NULL;

			if (SetDoubleParameterValue (& (param_p -> dp_current_value_p), current_value_p))
				{
					if (SetDoubleParameterValue (& (param_p -> dp_default_value_p), default_value_p))
						{
							param_p -> dp_min_value_p = NULL;
							param_p -> dp_max_value_p = NULL;

							return param_p;
						}

					if (param_p -> dp_current_value_p)
						{
							FreeMemory (param_p -> dp_current_value_p);
						}
				}

			FreeMemory (param_p);
		}

	return NULL;
}

