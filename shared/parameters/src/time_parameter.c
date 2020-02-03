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
 * time_parameter.c
 *
 *  Created on: 31 Jan 2020
 *      Author: billy
 */

#include "time_parameter.h"
#include "memory_allocations.h"
#include "service.h"
#include "json_util.h"
#include "time_util.h"

/*
 * STATIC DECLARATIONS
 */

static bool SetTimeParameterValue (struct tm **param_value_pp, const struct tm *new_value_p);

static void ClearTimeParameter (Parameter *param_p);

static bool AddTimeParameterDetailsToJSON (const Parameter *param_p, json_t *param_json_p);

static bool GetTimeParameterDetailsFromJSON (Parameter *param_p, const json_t *param_json_p);

static bool SetTimeValue (const json_t *param_json_p, const char *key_s, struct tm **time_pp);


/*
 * API DEFINITIONS
 */

TimeParameter *AllocateTimeParameter (const struct ServiceData *service_data_p, const char * const name_s, const char * const display_name_s, const char * const description_s, LinkedList *options_p, struct tm *default_value_p, struct tm *current_value_p, ParameterLevel level)
{
	TimeParameter *param_p = (TimeParameter *) AllocMemory (sizeof (TimeParameter));

	if (param_p)
		{
			bool success_flag = true;

			if (current_value_p)
				{
					param_p -> tp_current_value_p = AllocateTime ();

					if (param_p -> tp_current_value_p)
						{
							CopyTime (current_value_p, param_p -> tp_current_value_p);
						}
					else
						{
							success_flag = false;
						}
				}
			else
				{
					param_p -> tp_current_value_p = NULL;
				}

			if (success_flag)
				{
					if (default_value_p)
						{
							param_p -> tp_default_value_p = AllocateTime ();

							if (param_p -> tp_default_value_p)
								{
									CopyTime (default_value_p, param_p -> tp_default_value_p);
								}
							else
								{
									success_flag = false;
								}
						}
					else
						{
							param_p -> tp_default_value_p = NULL;
						}
				}

			if (success_flag)
				{
					if (InitParameter (& (param_p -> tp_base_param), service_data_p, PT_TIME, name_s, display_name_s, description_s, options_p, level,
														 ClearTimeParameter, AddTimeParameterDetailsToJSON, GetTimeParameterDetailsFromJSON,
														 NULL))
						{
							if (service_data_p)
								{
//									GetParameterDefaultValueFromConfig (service_data_p, name_s, param_p -> pa_type, &default_value);
								}

							param_p -> tp_min_value_p = NULL;
							param_p -> tp_max_value_p = NULL;

							return param_p;
						}
				}

			if (param_p -> tp_current_value_p)
				{
					FreeTime (param_p -> tp_current_value_p);
				}

			if (param_p -> tp_default_value_p)
				{
					FreeTime (param_p -> tp_default_value_p);
				}

			FreeMemory (param_p);
		}		/* if (param_p) */

	return NULL;
}


const struct tm *GetTimeParameterCurrentValue (const TimeParameter *param_p)
{
	return param_p -> tp_current_value_p;
}


bool SetTimeParameterCurrentValue (TimeParameter *param_p, const struct tm *value_p)
{
	return SetTimeParameterValue (& (param_p -> tp_current_value_p), value_p);
}


const struct tm *GetTimeParameterDefaultValue (const TimeParameter *param_p)
{
	return param_p -> tp_default_value_p;
}


bool SetTimeParameterDefaultValue (TimeParameter *param_p, const struct tm *value_p)
{
	return SetTimeParameterValue (& (param_p -> tp_default_value_p), value_p);
}


bool SetTimeParameterBounds (TimeParameter *param_p, const struct tm *min_value_p, const struct tm *max_value_p)
{
	bool success_flag = false;

	if (SetTimeParameterValue (& (param_p -> tp_min_value_p), min_value_p, max_value_p))
		{
			if (SetTimeParameterValue (& (param_p -> tp_min_value_p), min_value_p, max_value_p))
				{
					success_flag = true;
				}
		}

	return success_flag;
}


bool IsTimeParameterBounded (const TimeParameter *param_p)
{
	return ((param_p -> tp_min_value_p) && (param_p -> tp_max_value_p));
}


bool GetTimeParameterBounds (const TimeParameter *param_p, struct tm *min_p, struct tm *max_p)
{
	bool success_flag = false;

	if (IsTimeParameterBounded (param_p))
		{
			CopyTime (param_p -> tp_min_value_p, min_p);
			CopyTime (param_p -> tp_max_value_p, max_p);

			success_flag = true;
		}

	return success_flag;
}


/*
 * STATIC DEFINITIONS
 */

static bool SetTimeParameterValue (struct tm **param_value_pp, const struct tm *new_value_p)
{
	bool success_flag = true;

	if (new_value_p)
		{
			if (! (*param_value_pp))
				{
					*param_value_pp = AllocateTime ();

					if (! (*param_value_pp))
						{
							success_flag = false;
						}
				}

			if (success_flag)
				{
					CopyTime (new_value_p, *param_value_pp);
				}
		}
	else
		{
			if (*param_value_pp)
				{
					FreeTime (*param_value_pp);
					*param_value_pp = NULL;
				}
		}

	return success_flag;
}



static void ClearTimeParameter (Parameter *param_p)
{
	TimeParameter *time_param_p = (TimeParameter *) param_p;

	if (time_param_p -> tp_current_value_p)
		{
			FreeTime (time_param_p -> tp_current_value_p);
			time_param_p -> tp_current_value_p = NULL;
		}

	if (time_param_p -> tp_default_value_p)
		{
			FreeTime (time_param_p -> tp_default_value_p);
			time_param_p -> tp_current_value_p = NULL;
		}
}


static bool AddTimeParameterDetailsToJSON (const Parameter *param_p, json_t *param_json_p)
{
	TimeParameter *time_param_p = (TimeParameter *) param_p;
	bool success_flag = true;

	if (time_param_p -> tp_current_value_p)
		{
			char *time_s = GetTimeAsString (time_param_p -> tp_current_value_p, true);

			if (time_s)
				{
					success_flag = SetJSONString (param_json_p, PARAM_CURRENT_VALUE_S, time_s);
					FreeCopiedString (time_s);
				}
			else
				{
					success_flag = false;
				}
		}

	if (success_flag)
		{
			if (time_param_p -> tp_default_value_p)
				{
					char *time_s = GetTimeAsString (time_param_p -> tp_default_value_p, true);

					if (time_s)
						{
							success_flag = SetJSONString (param_json_p, PARAM_DEFAULT_VALUE_S, time_s);
							FreeCopiedString (time_s);
						}
					else
						{
							success_flag = false;
						}
				}
		}

	return success_flag;
}


static bool GetTimeParameterDetailsFromJSON (Parameter *param_p, const json_t *param_json_p)
{
	TimeParameter *time_param_p = (TimeParameter *) param_p;
	bool success_flag = false;


	if (SetTimeValueFromJSON (param_json_p, PARAM_CURRENT_VALUE_S, & (time_param_p -> tp_current_value_p)))
		{
			if (SetTimeValueFromJSON (param_json_p, PARAM_DEFAULT_VALUE_S, & (time_param_p -> tp_default_value_p)))
				{
					success_flag = true;
				}

		}

	return success_flag;
}



static bool SetTimeValueFromJSON (const json_t *param_json_p, const char *key_s, struct tm **time_pp)
{
	bool success_flag = false;
	const char *value_s = GetJSONString (param_json_p, key_s);

	if (value_s)
		{
			if (*time_pp)
				{
					if (SetTimeFromString (*time_pp, value_s))
						{
							success_flag = true;
						}
				}
			else
				{
					*time_pp = GetTimeFromString (value_s);

					if (*time_pp)
						{
							success_flag = true;
						}
				}
		}		/* if (value_s) */
	else
		{
			if (*time_pp)
				{
					FreeTime (*time_pp);
					*time_pp = NULL;
				}
		}

	return success_flag;
}
