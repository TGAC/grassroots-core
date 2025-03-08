/*
 * time_array_parameter.h
 *
 *  Created on: 18 Feb 2023
 *      Author: billy
 */

#ifndef CORE_SHARED_SERVICES_INCLUDE_PARAMETERS_TIME_ARRAY_PARAMETER_H_
#define CORE_SHARED_SERVICES_INCLUDE_PARAMETERS_TIME_ARRAY_PARAMETER_H_


#include "parameter.h"
#include "parameter_set.h"
#include "grassroots_service_library.h"

#include "service.h"


typedef struct TimeArrayParameter
{
	Parameter tap_base_param;

	struct tm **tap_current_values_pp;

	struct tm **tap_default_values_pp;

	size_t tap_num_values;

} TimeArrayParameter;



#ifdef __cplusplus
extern "C"
{
#endif

/*
 * Allocate a TimeArrayParameter
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
GRASSROOTS_SERVICE_API TimeArrayParameter *AllocateTimeArrayParameter (const struct ServiceData *service_data_p,  const char * const name_s, const char * const display_name_s, const char * const description_s, struct tm **default_values_pp, struct tm **current_values_pp, const size_t num_values, ParameterLevel level);


GRASSROOTS_SERVICE_API TimeArrayParameter *AllocateTimeArrayParameterFromJSON (const json_t *param_json_p, const struct Service *service_p, const bool concise_flag, const ParameterType *pt_p);



GRASSROOTS_SERVICE_API Parameter *EasyCreateAndAddTimeArrayParameterToParameterSet (const ServiceData *service_data_p, ParameterSet *params_p, ParameterGroup *group_p,
																											const char * const name_s, const char * const display_name_s, const char * const description_s,
																											struct tm **default_values_pp, const size_t num_values, ParameterLevel level);

GRASSROOTS_SERVICE_API Parameter *CreateAndAddTimeArrayParameterToParameterSet (const ServiceData *service_data_p, ParameterSet *params_p, ParameterGroup *group_p,
																											const char * const name_s, const char * const display_name_s, const char * const description_s,
																											struct tm **default_values_pp, struct tm **current_values_pp,
																											const size_t num_values, ParameterLevel level);


GRASSROOTS_SERVICE_API char *GetTimeArrayParameterCurrentValuesAsFlattenedString (const TimeArrayParameter *param_p);

GRASSROOTS_SERVICE_API void FreeTimeArrayParameter (TimeArrayParameter *param_p);

GRASSROOTS_SERVICE_API bool IsTimeArrayParameter (const Parameter *param_p);


GRASSROOTS_SERVICE_API const struct tm **GetTimeArrayParameterCurrentValues (const TimeArrayParameter *param_p);

GRASSROOTS_SERVICE_API const struct tm **GetTimeArrayParameterDefaultValues (const TimeArrayParameter *param_p);

GRASSROOTS_SERVICE_API const struct tm *GetTimeArrayParameterCurrentValueAtIndex (const TimeArrayParameter *param_p, const size_t index);


GRASSROOTS_SERVICE_API bool SetTimeArrayParameterCurrentValues (TimeArrayParameter *param_p, const struct tm **values_pp, const size_t num_values);


GRASSROOTS_SERVICE_API bool SetTimeArrayParameterDefaultValues (TimeArrayParameter *param_p, const struct tm **values_pp, const size_t num_values);


GRASSROOTS_SERVICE_API size_t GetNumberOfTimeArrayCurrentParameterValues (const TimeArrayParameter *param_p);




#ifdef __cplusplus
}
#endif



#endif /* CORE_SHARED_SERVICES_INCLUDE_PARAMETERS_TIME_ARRAY_PARAMETER_H_ */
