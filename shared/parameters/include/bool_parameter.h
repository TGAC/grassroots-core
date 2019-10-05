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
 * bool_parameter.h
 *
 *  Created on: 5 Oct 2019
 *      Author: billy
 */

#ifndef CORE_SHARED_PARAMETERS_INCLUDE_BOOL_PARAMETER_H_
#define CORE_SHARED_PARAMETERS_INCLUDE_BOOL_PARAMETER_H_

#include "parameter.h"
#include "grassroots_params_library.h"



typedef struct BoolParameter
{
	Parameter bp_base_parameter;

	bool *bp_current_value_p;

	bool *bp_default_value_p;

	bool bp_values [2];
} BoolParameter;



#ifdef __cplusplus
extern "C"
{
#endif


GRASSROOTS_PARAMS_API BoolParameter *AllocateBoolParameter (const struct ServiceData *service_data_p, ParameterType type, bool multi_valued_flag, const char * const name_s, const char * const display_name_s, const char * const description_s, LinkedList *options_p, const void *default_value_p, const void *current_value_p, ParameterBounds *bounds_p, ParameterLevel level, const char *(*check_value_fn) (const Parameter * const parameter_p, const void *value_p));


GRASSROOTS_PARAMS_API void ClearBoolParameter (BoolParameter *param_p);


GRASSROOTS_PARAMS_API bool SetBoolParameterCurrentValue (BoolParameter *param_p, bool value);



#ifdef __cplusplus
}
#endif


#endif /* CORE_SHARED_PARAMETERS_INCLUDE_BOOL_PARAMETER_H_ */
