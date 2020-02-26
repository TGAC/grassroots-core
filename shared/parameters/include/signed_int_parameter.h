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
 * signed_int_parameter.h
 *
 *  Created on: 30 Jan 2020
 *      Author: billy
 */

#ifndef CORE_SHARED_PARAMETERS_INCLUDE_SIGNED_INT_PARAMETER_H_
#define CORE_SHARED_PARAMETERS_INCLUDE_SIGNED_INT_PARAMETER_H_



#include "parameter.h"
#include "grassroots_params_library.h"
#include "parameter_set.h"


typedef struct SignedIntParameter
{
	Parameter sip_base_param;

	int32 *sip_current_value_p;

	int32 *sip_default_value_p;

	int32 *sip_min_value_p;

	int32 *sip_max_value_p;

} SignedIntParameter;


#ifdef __cplusplus
extern "C" {
#endif

/**
 * Allocate a SignedIntParameter
 *
 * @param service_data_p The ServiceData for the Service that is allocating this Parameter.
 * @param name_s The name of the Parameter. The Parameter will store a copy of this string so this value does not need to remain in scope.
 * @param display_name_s An optional name to display for the Parameter for use in Clients. The Parameter will store a copy of this string so this value does not need to remain in scope.
 * This can be <code>NULL</code>.
 * @param description_s The description of the Parameter. The Parameter will store a copy of this string so this value does not need to remain in scope.
 * @param options_p This can be used to constrain the Parameter to a fixed set of values. If this is <code>NULL</code> then the Parameter can be set to any value.
 * @param default_value_p The default value for this Parameter.
 * @param current_value_p If this is not <code>NULL</code>, then copy this value as the current value of the Parameter. If this is <code>NULL</code>, then current value for this Parameter
 * will be set to be a copy of its default value.
 * @param bounds_p If this is not <code>NULL</code>, then this will be used to specify the minimum and maximum values that this Parameter can take. If this is <code>NULL</code>,
 * then the Parameter can take any value.
 * @param level The ParameterLevel for this Parameter. This determines when the Client should display this Parameter to the user.
 * @param check_value_fn If this is not <code>NULL</code>, then this will be used to check whether the Parameter has been set to a valid value.
 * @return A newly-allocated Parameter or <code>NULL</code> upon error.
 * @memberof SignedIntParameter
 */
GRASSROOTS_PARAMS_API SignedIntParameter *AllocateSignedIntParameter (const struct ServiceData *service_data_p, const ParameterType pt,
																																			const char * const name_s, const char * const display_name_s,
																																			const char * const description_s,
																																			const int32 *default_value_p, const int32 *current_value_p,
																																			ParameterLevel level);


GRASSROOTS_PARAMS_API SignedIntParameter *AllocateSignedIntParameterFromJSON (const json_t *param_json_p, const struct Service *service_p, const bool concise_flag);



GRASSROOTS_PARAMS_API void FreeSignedIntParameter (SignedIntParameter *param_p);


GRASSROOTS_PARAMS_API Parameter *EasyCreateAndAddSignedIntParameterToParameterSet (const struct ServiceData *service_data_p, ParameterSet *params_p, ParameterGroup *group_p, ParameterType type,
																								const char * const name_s, const char * const display_name_s, const char * const description_s,
																								const int32 *default_value_p, uint8 level);


GRASSROOTS_PARAMS_API Parameter *CreateAndAddSignedIntParameterToParameterSet (const struct ServiceData *service_data_p, ParameterSet *params_p, ParameterGroup *group_p,  ParameterType type,
																								const char * const name_s, const char * const display_name_s, const char * const description_s,
																								const int32 *default_value_p, const int32 *current_value_p, uint8 level);


GRASSROOTS_PARAMS_API const int32 *GetSignedIntParameterCurrentValue (const SignedIntParameter *param_p);


GRASSROOTS_PARAMS_API bool SetSignedIntParameterCurrentValue (SignedIntParameter *param_p, const int32 *value_p);


GRASSROOTS_PARAMS_API const int32 *GetSignedIntParameterDefaultValue (const SignedIntParameter *param_p);


GRASSROOTS_PARAMS_API bool SetSignedIntParameterDefaultValue (SignedIntParameter *param_p, const int32 *value_p);


GRASSROOTS_PARAMS_API bool SetSignedIntParameterBounds (SignedIntParameter *param_p, const int32 min_value, const int32 max_value);


GRASSROOTS_PARAMS_API bool IsSignedIntParameterBounded (const SignedIntParameter *param_p);


GRASSROOTS_PARAMS_API bool GetSignedIntParameterBounds (const SignedIntParameter *param_p, int32 *min_p, int32 *max_p);


GRASSROOTS_PARAMS_API bool IsSignedIntParameter (const Parameter *param_p);


GRASSROOTS_PARAMS_API bool GetCurrentSignedIntParameterValueFromParameterSet (const ParameterSet * const params_p, const char * const name_s, const int32 **value_pp);

#ifdef __cplusplus
}
#endif


#endif /* CORE_SHARED_PARAMETERS_INCLUDE_SIGNED_INT_PARAMETER_H_ */
