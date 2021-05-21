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
 * json_parameter.h
 *
 *  Created on: 31 Jan 2020
 *      Author: billy
 */

#ifndef CORE_SHARED_PARAMETERS_INCLUDE_JSON_PARAMETER_H_
#define CORE_SHARED_PARAMETERS_INCLUDE_JSON_PARAMETER_H_

#include "parameter.h"
#include "grassroots_params_library.h"
#include "parameter_set.h"



typedef struct JSONParameter
{
	Parameter jp_base_param;

	json_t *jp_current_value_p;

	json_t *jp_default_value_p;
} JSONParameter;


#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifdef GRASSROOTS_PARAMS_LIBRARY_EXPORTS /* defined if we are building the LIB DLL (instead of using it) */

	#ifdef ALLOCATE_JSON_PARAMETER_TAGS
		#define JSON_PARAM_PREFIX GRASSROOTS_PARAMS_API
		#define JSON_PARAM_VAL(x)	= x
	#else
		#define JSON_PARAM_PREFIX extern
		#define JSON_PARAM_VAL(x)
	#endif
#else
	#define JSON_PARAM_PREFIX GRASSROOTS_PARAMS_API
	#define JSON_PARAM_VAL(x)
#endif


#endif 		/* #ifndef DOXYGEN_SHOULD_SKIP_THIS */





JSON_PARAM_PREFIX const char *TABLE_PARAM_ROW_S JSON_PARAM_VAL("row");
JSON_PARAM_PREFIX const char *TABLE_PARAM_COLUMN_S JSON_PARAM_VAL("column");


#ifdef __cplusplus
extern "C" {
#endif

/**
 * Allocate a JSONParameter
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
 * @memberof JSONParameter
 */
GRASSROOTS_PARAMS_API JSONParameter *AllocateJSONParameter (const struct ServiceData *service_data_p, const ParameterType pt, const char * const name_s, const char * const display_name_s, const char * const description_s, const json_t *default_value_p, const json_t *current_value_p, ParameterLevel level);



GRASSROOTS_PARAMS_API JSONParameter *AllocateJSONParameterFromJSON (const json_t *param_json_p, const struct Service *service_p, const bool concise_flag, const ParameterType *pt_p);


GRASSROOTS_PARAMS_API Parameter *EasyCreateAndAddJSONParameterToParameterSet (const struct ServiceData *service_data_p, ParameterSet *params_p, ParameterGroup *group_p, ParameterType type,
																											const char * const name_s, const char * const display_name_s, const char * const description_s,
																											const json_t *default_value_p, uint8 level);


GRASSROOTS_PARAMS_API Parameter *CreateAndAddJSONParameterToParameterSet (const struct ServiceData *service_data_p, ParameterSet *params_p, ParameterGroup *group_p, ParameterType type,
																											const char * const name_s, const char * const display_name_s, const char * const description_s,
																											const json_t *default_value_p, const json_t *current_value_p, uint8 level);


GRASSROOTS_PARAMS_API void FreeJSONParameter (JSONParameter *param_p);


GRASSROOTS_PARAMS_API const json_t *GetJSONParameterCurrentValue (const JSONParameter *param_p);


GRASSROOTS_PARAMS_API bool SetJSONParameterCurrentValue (JSONParameter *param_p, const json_t *value_p);


GRASSROOTS_PARAMS_API const json_t *GetJSONParameterDefaultValue (const JSONParameter *param_p);


GRASSROOTS_PARAMS_API bool SetJSONParameterDefaultValue (JSONParameter *param_p, const json_t *value_p);


GRASSROOTS_PARAMS_API bool IsJSONParameter (Parameter *param_p);


GRASSROOTS_PARAMS_API bool GetCurrentJSONParameterValueFromParameterSet (const ParameterSet * const params_p, const char * const name_s, const json_t **value_pp);


GRASSROOTS_PARAMS_API bool SetJSONParameterCurrentValueFromJSON (JSONParameter *param_p, const json_t *value_p);


#ifdef __cplusplus
}
#endif

#endif /* CORE_SHARED_PARAMETERS_INCLUDE_JSON_PARAMETER_H_ */
