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
 * resource_parameter.h
 *
 *  Created on: 31 Jan 2020
 *      Author: billy
 */

#ifndef CORE_SHARED_PARAMETERS_INCLUDE_RESOURCE_PARAMETER_H_
#define CORE_SHARED_PARAMETERS_INCLUDE_RESOURCE_PARAMETER_H_

#include "parameter.h"
#include "grassroots_service_library.h"
#include "parameter_set.h"


typedef struct ResourceParameter
{
	Parameter rp_base_param;

	DataResource *rp_current_value_p;

	DataResource *rp_default_value_p;
} ResourceParameter;


#ifdef __cplusplus
extern "C" {
#endif

/**
 * Allocate a ResourceParameter
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
 * @memberof ResourceParameter
 */
GRASSROOTS_SERVICE_API ResourceParameter *AllocateResourceParameter (const struct ServiceData *service_data_p, const ParameterType pt, const char * const name_s, const char * const display_name_s, const char * const description_s, DataResource *default_value_p, DataResource *current_value_p, ParameterLevel level);


GRASSROOTS_SERVICE_API ResourceParameter *AllocateResourceParameterFromJSON (const json_t *param_json_p, const struct Service *service_p, const bool concise_flag);


GRASSROOTS_SERVICE_API void FreeResourceParameter (ResourceParameter *param_p);


GRASSROOTS_SERVICE_API const DataResource *GetResourceParameterCurrentValue (const ResourceParameter *param_p);


GRASSROOTS_SERVICE_API bool SetResourceParameterCurrentValue (ResourceParameter *param_p, const DataResource *value_p);


GRASSROOTS_SERVICE_API const DataResource *GetResourceParameterDefaultValue (const ResourceParameter *param_p);


GRASSROOTS_SERVICE_API bool SetResourceParameterDefaultValue (ResourceParameter *param_p, const DataResource *value_p);


GRASSROOTS_SERVICE_API bool GetCurrentResourceParameterValueFromParameterSet (const ParameterSet * const params_p, const char * const name_s, const DataResource **value_pp);


GRASSROOTS_SERVICE_API bool IsResourceParameter (const Parameter *param_p);


GRASSROOTS_SERVICE_API bool SetResourceParameterCurrentValueFromJSON (ResourceParameter *param_p, const json_t *value_p);

#ifdef __cplusplus
}
#endif

#endif /* CORE_SHARED_PARAMETERS_INCLUDE_RESOURCE_PARAMETER_H_ */
