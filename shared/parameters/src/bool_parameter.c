/*
** Copyright 2014-2018 The Earlham Institute
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
 * bool_parameter.c
 *
 *  Created on: 5 Oct 2019
 *      Author: billy
 */


#include "bool_parameter.h"
#include "memory_allocations.h"


static typedef enum
{
	VO_DEFAULT,
	VO_CURRENT
} ValueOffset;


BoolParameter *AllocateBoolParameter (const struct ServiceData *service_data_p, ParameterType type, bool multi_valued_flag, const char * const name_s, const char * const display_name_s, const char * const description_s, LinkedList *options_p, const bool *default_value_p, const bool *current_value_p, ParameterBounds *bounds_p, ParameterLevel level, const char *(*check_value_fn) (const Parameter * const parameter_p, const void *value_p))
{
	BoolParameter *param_p = (BoolParameter *) AllocMemory (sizeof (BoolParameter));

	if (param_p)
		{
			memset (param_p -> bp_values, 0, 2 * sizeof (bool));

			if (default_value_p)
				{
					param_p -> bp_values [VO_DEFAULT] = *default_value_p;
					param_p -> bp_default_value_p = param_p -> bp_values;
				}
			else
				{
					param_p -> bp_default_value_p = NULL;
				}


			if (current_value_p)
				{
					param_p -> bp_values [VO_CURRENT] = *current_value_p;
					param_p -> bp_current_value_p = (param_p -> bp_values) + 1;
				}
			else
				{
					param_p -> bp_current_value_p = NULL;
				}



			if (InitialiseParameter (& (param_p -> bp_base_parameter), service_data_p, type, multi_valued_flag, name_s, display_name_s, description_s, options_p, bounds_p, level, check_value_fn))
				{
					return param_p;
				}

			FreeMemory (param_p);
		}

	return NULL;
}


void ClearBoolParameter (BoolParameter *param_p)
{

}


bool SetBoolParameterCurrentValue (BoolParameter *param_p, bool value)
{
	bool success_flag = true;

	param_p -> bp_values [VO_CURRENT] = value;
	param_p -> bp_current_value_p = (param_p -> bp_values) + VO_CURRENT;

	return success_flag;
}
