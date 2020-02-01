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
 * doub;e_parameter.c
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


/*
 * API DEFINITIONS
 */

DoubleParameter *AllocateDoubleParameter (const struct ServiceData *service_data_p, const ParameterType pt, const char * const name_s, const char * const display_name_s, const char * const description_s, LinkedList *options_p, double64 *default_value_p, double64 *current_value_p, ParameterLevel level, const char *(*check_value_fn) (const Parameter * const parameter_p, const void *value_p))
{
	DoubleParameter *param_p = (DoubleParameter *) AllocMemory (sizeof (DoubleParameter));

	if (param_p)
		{
			bool success_flag = true;

			if (current_value_p)
				{
					param_p -> dp_current_value_p = (double64 *) AllocMemory (sizeof (double64));

					if (param_p -> dp_current_value_p)
						{
							* (param_p -> dp_current_value_p) = *current_value_p;
						}
					else
						{
							success_flag = false;
						}
				}
			else
				{
					param_p -> dp_current_value_p = NULL;
				}


			if (success_flag)
				{
					if (default_value_p)
						{
							param_p -> dp_default_value_p = (double64 *) AllocMemory (sizeof (double64));

							if (param_p -> dp_default_value_p)
								{
									* (param_p -> dp_default_value_p) = *default_value_p;
								}
							else
								{
									success_flag = false;
								}
						}
					else
						{
							param_p -> dp_default_value_p = NULL;
						}
				}

			if (success_flag)
				{
					if (InitParameter (& (param_p -> dp_base_param), service_data_p, pt, name_s, display_name_s, description_s, options_p, level, check_value_fn))
						{
							if (service_data_p)
								{
//									GetParameterDefaultValueFromConfig (service_data_p, name_s, param_p -> pa_type, &default_value);
								}

							param_p -> dp_base_param.pa_add_values_to_json_fn = AddDoubleParameterDetailsToJSON;
							param_p -> dp_min_value_p = NULL;
							param_p -> dp_max_value_p = NULL;

							return param_p;
						}
				}

			if (param_p -> dp_current_value_p)
				{
					FreeMemory (param_p -> dp_current_value_p);
				}

			if (param_p -> dp_default_value_p)
				{
					FreeMemory (param_p -> dp_default_value_p);
				}

			FreeMemory (param_p);
		}		/* if (param_p) */

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



bool SetDoubleParameterBounds (DoubleParameter *param_p, const char min_value, const char max_value)
{
	if (! (param_p -> dp_min_value_p))
		{
			param_p -> dp_min_value_p = (double64 *) AllocMemory (sizeof (double64));

			if (! (param_p -> dp_min_value_p))
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate min value");
					return false;
				}
		}

	if (! (param_p -> dp_max_value_p))
		{
			param_p -> dp_max_value_p = (double64 *) AllocMemory (sizeof (double64));

			if (! (param_p -> dp_max_value_p))
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate max value");
					return false;
				}
		}

	* (param_p -> dp_min_value_p) = min_value;
	* (param_p -> dp_max_value_p) = max_value;

	return true;
}


bool IsDoubleParameterBounded (const DoubleParameter *param_p)
{
	return ((param_p -> dp_min_value_p) && (param_p -> dp_max_value_p));
}


bool GetDoubleParameterBounds (const DoubleParameter *param_p, char *min_p, char *max_p)
{
	bool success_flag = false;

	if (IsDoubleParameterBounded (param_p))
		{
			*min_p = * (param_p -> dp_min_value_p);
			*max_p = * (param_p -> dp_max_value_p);

			success_flag = true;
		}

	return success_flag;
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

	if ((double_param_p -> dp_current_value_p == NULL ) || (SetJSONDouble (param_json_p, PARAM_CURRENT_VALUE_S, * (double_param_p -> dp_current_value_p))))
		{
			if (full_definition_flag)
				{
					if ((double_param_p -> dp_default_value_p == NULL ) || (SetJSONDouble (param_json_p, PARAM_DEFAULT_VALUE_S, * (double_param_p -> dp_default_value_p))))
						{
							if ((double_param_p -> dp_min_value_p == NULL ) || (SetJSONDouble (param_json_p, PARAM_MIN_S, * (double_param_p -> dp_min_value_p))))
								{
									if ((double_param_p -> dp_max_value_p == NULL ) || (SetJSONDouble (param_json_p, PARAM_MAX_S, * (double_param_p -> dp_max_value_p))))
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


static bool GetDoubleParameterDetailsFromJSON (Parameter *param_p, const json_t *param_json_p)
{
	DoubleParameter *double_param_p = (DoubleParameter *) param_p;
	bool success_flag = true;
	double64 d;

	if (GetJSONReal (param_json_p, PARAM_CURRENT_VALUE_S, &d))
		{
			success_flag = SetDoubleParameterCurrentValue (double_param_p, &d);
		}
	else
		{
			success_flag = SetDoubleParameterCurrentValue (double_param_p, NULL);
		}

	if (success_flag)
		{
			if (GetJSONReal (param_json_p, PARAM_DEFAULT_VALUE_S, &d))
				{
					success_flag = SetDoubleParameterDefaultValue (double_param_p, &d);
				}
			else
				{
					success_flag = SetDoubleParameterDefaultValue (double_param_p, NULL);
				}
		}

	return success_flag;
}
