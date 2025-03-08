/*
 * string_array_parameter.h
 *
 *  Created on: 17 Jan 2021
 *      Author: billy
 */

#ifndef CORE_SHARED_PARAMETERS_INCLUDE_STRING_ARRAY_PARAMETER_H_
#define CORE_SHARED_PARAMETERS_INCLUDE_STRING_ARRAY_PARAMETER_H_



#include "parameter.h"
#include "parameter_set.h"
#include "grassroots_service_library.h"

#include "service.h"


typedef struct StringArrayParameter
{
	Parameter sap_base_param;

	char **sap_current_values_ss;


	char **sap_default_values_ss;

	size_t sap_num_values;

} StringArrayParameter;



#ifdef __cplusplus
extern "C"
{
#endif

/*
 * Allocate a StringArrayParameter
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
GRASSROOTS_SERVICE_API StringArrayParameter *AllocateStringArrayParameter (const struct ServiceData *service_data_p,  const char * const name_s, const char * const display_name_s, const char * const description_s, char **default_value_ss, char **current_value_ss, const size_t num_values, ParameterLevel level);


GRASSROOTS_SERVICE_API StringArrayParameter *AllocateStringArrayParameterFromJSON (const json_t *param_json_p, const struct Service *service_p, const bool concise_flag, const ParameterType *pt_p);



GRASSROOTS_SERVICE_API Parameter *EasyCreateAndAddStringArrayParameterToParameterSet (const ServiceData *service_data_p, ParameterSet *params_p, ParameterGroup *group_p,
																											const char * const name_s, const char * const display_name_s, const char * const description_s,
																											char **default_values_ss, uint32 num_entries, ParameterLevel level);

GRASSROOTS_SERVICE_API Parameter *CreateAndAddStringArrayParameterToParameterSet (const ServiceData *service_data_p, ParameterSet *params_p, ParameterGroup *group_p,
																											const char * const name_s, const char * const display_name_s, const char * const description_s,
																											char **default_value_ss, char **current_value_ss, uint32 num_entries, ParameterLevel level);


GRASSROOTS_SERVICE_API char *GetStringArrayParameterCurrentValuesAsFlattenedString (const StringArrayParameter *param_p);

GRASSROOTS_SERVICE_API const char **GetStringArrayValuesForParameter (ParameterSet *param_set_p, const char *param_s, size_t *num_entries_p);


GRASSROOTS_SERVICE_API void FreeStringArrayParameter (StringArrayParameter *param_p);

GRASSROOTS_SERVICE_API bool IsStringArrayParameter (const Parameter *param_p);


GRASSROOTS_SERVICE_API const char **GetStringArrayParameterCurrentValues (const StringArrayParameter *param_p);

GRASSROOTS_SERVICE_API const char **GetStringArrayParameterDefaultValues (const StringArrayParameter *param_p);


GRASSROOTS_SERVICE_API bool GetCurrentStringArrayParameterValuesFromParameterSet (const ParameterSet * const params_p, const char * const name_s, const char ***values_ppp, size_t *num_entries_p);


GRASSROOTS_SERVICE_API const char *GetStringArrayParameterCurrentValueAtIndex (const StringArrayParameter *param_p, const size_t index);



GRASSROOTS_SERVICE_API bool SetStringArrayParameterCurrentValues (StringArrayParameter *param_p, char **values_ss, const size_t num_values);

GRASSROOTS_SERVICE_API bool SetStringArrayParameterDefaultValues (StringArrayParameter *param_p, char **values_ss, const size_t num_values);


GRASSROOTS_SERVICE_API size_t GetNumberOfStringArrayCurrentParameterValues (const StringArrayParameter *param_p);



#ifdef __cplusplus
}
#endif

#endif /* CORE_SHARED_PARAMETERS_INCLUDE_STRING_ARRAY_PARAMETER_H_ */
