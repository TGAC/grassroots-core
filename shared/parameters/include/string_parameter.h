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
 * char_parameter.h
 *
 *  Created on: 30 Jan 2020
 *      Author: billy
 */

#ifndef CORE_SHARED_PARAMETERS_INCLUDE_STRING_PARAMETER_H_
#define CORE_SHARED_PARAMETERS_INCLUDE_STRING_PARAMETER_H_


#include "parameter.h"
#include "parameter_set.h"
#include "grassroots_params_library.h"


typedef struct StringParameter
{
	Parameter sp_base_param;

	char *sp_current_value_s;

	char *sp_default_value_s;

	char *sp_min_value_s;

	char *sp_max_value_s;

} StringParameter;



typedef struct StringParameterOption
{
	/** The internal value for this option */
	char *spo_value_s;

	/** The user-friendly description for this value */
	char *spo_description_s;

} StringParameterOption;


typedef struct StringParameterOptionNode
{
	ListItem spon_node;

	StringParameterOption *spon_option_p;

} StringParameterOptionNode;


#ifdef __cplusplus
extern "C"
{
#endif

/*
 * Allocate a StringParameter
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
 * @memberof BooleanParameter
 */
GRASSROOTS_PARAMS_API StringParameter *AllocateStringParameter (const struct ServiceData *service_data_p, const ParameterType pt, const char * const name_s, const char * const display_name_s, const char * const description_s, const char *default_value_p, const char *current_value_p, ParameterLevel level);


GRASSROOTS_PARAMS_API StringParameter *AllocateStringParameterFromJSON (const json_t *param_json_p, const struct Service *service_p, const bool concise_flag);


GRASSROOTS_PARAMS_API Parameter *EasyCreateAndAddStringParameterToParameterSet (const struct ServiceData *service_data_p, ParameterSet *params_p, ParameterGroup *group_p, ParameterType type,
																											const char * const name_s, const char * const display_name_s, const char * const description_s,
																											const char *default_value_s, uint8 level);


GRASSROOTS_PARAMS_API Parameter *CreateAndAddStringParameterToParameterSet (const struct ServiceData *service_data_p, ParameterSet *params_p, ParameterGroup *group_p, ParameterType type,
																											const char * const name_s, const char * const display_name_s, const char * const description_s,
																											const char *default_value_s, const char *current_value_s, uint8 level);



GRASSROOTS_PARAMS_API void FreeStringParameter (StringParameter *param_p);


GRASSROOTS_PARAMS_API const char *GetStringParameterCurrentValue (const StringParameter *param_p);


GRASSROOTS_PARAMS_API bool SetStringParameterCurrentValue (StringParameter *param_p, const char *value_p);


GRASSROOTS_PARAMS_API const char *GetStringParameterDefaultValue (const StringParameter *param_p);


GRASSROOTS_PARAMS_API bool SetStringParameterDefaultValue (StringParameter *param_p, const char *value_p);


GRASSROOTS_PARAMS_API bool SetStringParameterBounds (StringParameter *param_p, const char *min_value_s, const char *max_value_s);


GRASSROOTS_PARAMS_API bool IsStringParameterBounded (const StringParameter *param_p);


GRASSROOTS_PARAMS_API bool GetStringParameterBounds (const StringParameter *param_p, const char **min_pp, const char **max_pp);


GRASSROOTS_PARAMS_API bool CreateAndAddStringParameterOption (StringParameter *param_p, const char *value_s, const char *description_s);


GRASSROOTS_PARAMS_API StringParameterOption *AllocateStringParameterOption (const char *value_s, const char *description_s);


GRASSROOTS_PARAMS_API StringParameterOptionNode *AllocateStringParameterOptionNode (StringParameterOption *option_p);


GRASSROOTS_PARAMS_API void FreeStringParameterOption (StringParameterOption *option_p);


GRASSROOTS_PARAMS_API bool IsStringParameter (const Parameter *param_p);


/**
 * Get the value of a StringParameter within a ParameterSet
 *
 * @param params_p The ParameterSet to get the Parameter from.
 * @param name_s The Parameter name to try and match.
 * @param value_pp Where the StringParameter value will be stored upon success.
 * @return <code>true</code> if the Parameter value was retrieved successfully, <code>false</code>
 * otherwise.
 * @memberof ParameterSet
 */
GRASSROOTS_PARAMS_API bool GetCurrentStringParameterValueFromParameterSet (const ParameterSet * const params_p, const char * const name_s, const char **value_pp);



GRASSROOTS_PARAMS_API bool GetCurrentStringParameterValueFromParameterGroup (const ParameterGroup * const params_p, const char * const name_s, const char **value_pp);

#ifdef __cplusplus
}
#endif

#endif /* CORE_SHARED_PARAMETERS_INCLUDE_STRING_PARAMETER_H_ */
