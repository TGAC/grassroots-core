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
 * boolean_parameter.c
 *
 *  Created on: 30 Jan 2020
 *      Author: billy
 */


#include "boolean_parameter.h"
#include "memory_allocations.h"
#include "service.h"
#include "json_util.h"

#include "parameter.h"


/*
 * STATIC DECLARATIONS
 */

static bool SetBooleanParameterValue (bool **param_value_pp, const bool *new_value_p);

static void ClearBooleanParameter (Parameter *param_p);

static bool AddBooleanParameterDetailsToJSON (const Parameter *param_p, json_t *param_json_p, const bool full_definition_flag);

static bool GetBooleanParameterDetailsFromJSON (Parameter *param_p, const json_t *param_json_p);

static bool SetValueFromJSON (bool **value_pp, const json_t *param_json_p, const char *key_s);

static bool SetBooleanParameterCurrentValueFromString (Parameter *param_p, const char *value_s);


/*
 * API DEFINITIONS
 */

BooleanParameter *AllocateBooleanParameterFromJSON (const json_t *param_json_p, const Service *service_p)
{
	BooleanParameter *param_p = NULL;
	bool *current_value_p = NULL;

	if (SetValueFromJSON (&current_value_p, param_json_p, PARAM_CURRENT_VALUE_S))
		{
			bool *default_value_p = NULL;
			bool success_flag = true;
			bool full_definition_flag = ! (IsJSONParameterConcise (param_json_p));

			if (full_definition_flag)
				{
					if (!SetValueFromJSON (&default_value_p, param_json_p, PARAM_DEFAULT_VALUE_S))
						{
							success_flag = false;
						}
				}

			if (success_flag)
				{
					param_p = (BooleanParameter *) AllocMemory (sizeof (BooleanParameter));

					if (param_p)
						{
							if (InitParameterFromJSON (& (param_p -> bp_base_param), param_json_p, service_p, full_definition_flag))
								{
									SetParameterCallbacks (& (param_p -> bp_base_param), ClearBooleanParameter, AddBooleanParameterDetailsToJSON,
																				 GetBooleanParameterDetailsFromJSON, NULL, SetBooleanParameterCurrentValueFromString);

									param_p -> bp_current_value_p = current_value_p;
									param_p -> bp_default_value_p = default_value_p;

									return param_p;
								}

							FreeMemory (param_p);
						}


					if (default_value_p)
						{
							FreeMemory (default_value_p);
						}
				}		/* if (SetValueFromJSON (&default_value_p, param_json_p, PARAM_DEFAULT_VALUE_S)) */

			if (current_value_p)
				{
					FreeMemory (current_value_p);
				}
		}		/* if (SetValueFromJSON (&current_value_p, param_json_p, PARAM_CURRENT_VALUE_S)) */

	return NULL;
}


BooleanParameter *AllocateBooleanParameter (const struct ServiceData *service_data_p, const char * const name_s, const char * const display_name_s, const char * const description_s, LinkedList *options_p, bool *default_value_p, bool *current_value_p, ParameterLevel level)
{
	BooleanParameter *param_p = (BooleanParameter *) AllocMemory (sizeof (BooleanParameter));

	if (param_p)
		{
			bool success_flag = true;

			if (current_value_p)
				{
					param_p -> bp_current_value_p = (bool *) AllocMemory (sizeof (bool));

					if (param_p -> bp_current_value_p)
						{
							* (param_p -> bp_current_value_p) = *current_value_p;
						}
					else
						{
							success_flag = false;
						}
				}
			else
				{
					param_p -> bp_current_value_p = NULL;
				}


			if (success_flag)
				{
					if (default_value_p)
						{
							param_p -> bp_default_value_p = (bool *) AllocMemory (sizeof (bool));

							if (param_p -> bp_default_value_p)
								{
									* (param_p -> bp_default_value_p) = *default_value_p;
								}
							else
								{
									success_flag = false;
								}
						}
					else
						{
							param_p -> bp_default_value_p = NULL;
						}
				}

			if (success_flag)
				{
					if (InitParameter (& (param_p -> bp_base_param), service_data_p, PT_BOOLEAN, name_s, display_name_s, description_s, options_p, level,
														 ClearBooleanParameter, AddBooleanParameterDetailsToJSON, GetBooleanParameterDetailsFromJSON,
														 NULL, SetBooleanParameterCurrentValueFromString))
						{
							if (service_data_p)
								{
//									GetParameterDefaultValueFromConfig (service_data_p, name_s, param_p -> pa_type, &default_value);
								}

							return param_p;
						}
				}

			if (param_p -> bp_current_value_p)
				{
					FreeMemory (param_p -> bp_current_value_p);
				}

			if (param_p -> bp_default_value_p)
				{
					FreeMemory (param_p -> bp_default_value_p);
				}

			FreeMemory (param_p);
		}		/* if (param_p) */

	return NULL;
}


const bool *GetBooleanParameterCurrentValue (const BooleanParameter *param_p)
{
	return param_p -> bp_current_value_p;
}


bool SetBooleanParameterCurrentValue (BooleanParameter *param_p, const bool *value_p)
{
	return SetBooleanParameterValue (& (param_p -> bp_current_value_p), value_p);
}


const bool *GetBooleanParameterDefaultValue (const BooleanParameter *param_p)
{
	return param_p -> bp_default_value_p;
}


bool SetBooleanParameterDefaultValue (BooleanParameter *param_p, const bool *value_p)
{
	return SetBooleanParameterValue (& (param_p -> bp_default_value_p), value_p);
}


bool IsBooleanParameter (Parameter *param_p)
{
	bool boolean_param_flag = false;

	switch (param_p -> pa_type)
		{
			case PT_BOOLEAN:
				boolean_param_flag = true;
				break;

			default:
				break;
		}

	return boolean_param_flag;
}


bool GetCurrentBooleanParameterValueFromParameterSet (const ParameterSet * const params_p, const char * const name_s, bool *value_p)
{
	bool success_flag = false;
	Parameter *param_p = GetParameterFromParameterSetByName (params_p, name_s);

	if (param_p)
		{
			if (IsBooleanParameter (param_p))
				{
					const bool *current_value_p = GetBooleanParameterCurrentValue ((const BooleanParameter *) param_p);

					*value_p = *current_value_p;
					success_flag = true;
				}
		}

	return success_flag;
}



Parameter *EasyCreateAndAddBooleanParameterToParameterSet (const ServiceData *service_data_p, ParameterSet *params_p, ParameterGroup *group_p,
																								const char * const name_s, const char * const display_name_s, const char * const description_s,
																								bool *default_value_p, uint8 level)
{
	return CreateAndAddBooleanParameterToParameterSet (service_data_p, params_p, group_p, name_s, display_name_s, description_s, NULL, default_value_p, NULL, level);
}


Parameter *CreateAndAddBooleanParameterToParameterSet (const ServiceData *service_data_p, ParameterSet *params_p, ParameterGroup *group_p,
																								const char * const name_s, const char * const display_name_s, const char * const description_s, LinkedList *options_p,
																								bool *default_value_p, bool *current_value_p, uint8 level)
{
	BooleanParameter *bool_param_p = AllocateBooleanParameter (service_data_p, name_s, display_name_s, description_s, options_p, default_value_p, current_value_p, level);

	if (bool_param_p)
		{
			Parameter *base_param_p = & (bool_param_p -> bp_base_param);

			if (group_p)
				{
					/*
					 * If the parameter fails to get added to the group, it's
					 * not a terminal error so still carry on
					 */
					if (!AddParameterToParameterGroup (group_p, base_param_p))
						{
							PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to add param \"%s\" to group \"%s\"", name_s, group_p -> pg_name_s);
						}
				}

			if (AddParameterToParameterSet (params_p, base_param_p))
				{
					return base_param_p;
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add param \"%s\" to set \"%s\"", name_s, params_p -> ps_name_s);
					FreeParameter (base_param_p);
				}

		}		/* if (bool_param_p) */

	return NULL;
}


/*
 * STATIC DEFINITIONS
 */

static bool SetBooleanParameterValue (bool **param_value_pp, const bool *new_value_p)
{
	bool success_flag = true;

	if (new_value_p)
		{
			if (! (*param_value_pp))
				{
					*param_value_pp = (bool *) AllocMemory (sizeof (bool));

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



static void ClearBooleanParameter (Parameter *param_p)
{
	BooleanParameter *boolean_param_p = (BooleanParameter *) param_p;

	if (boolean_param_p -> bp_current_value_p)
		{
			FreeMemory (boolean_param_p -> bp_current_value_p);
			boolean_param_p -> bp_current_value_p = NULL;
		}

	if (boolean_param_p -> bp_default_value_p)
		{
			FreeMemory (boolean_param_p -> bp_default_value_p);
			boolean_param_p -> bp_current_value_p = NULL;
		}
}


static bool AddBooleanParameterDetailsToJSON (const Parameter *param_p, json_t *param_json_p, const bool full_definition_flag)
{
	BooleanParameter *boolean_param_p = (BooleanParameter *) param_p;
	bool success_flag = false;

	if ((boolean_param_p -> bp_current_value_p == NULL ) || (SetJSONBoolean (param_json_p, PARAM_CURRENT_VALUE_S, * (boolean_param_p -> bp_current_value_p))))
		{
			if (full_definition_flag)
				{
					if ((boolean_param_p -> bp_default_value_p == NULL ) || (SetJSONBoolean (param_json_p, PARAM_DEFAULT_VALUE_S, * (boolean_param_p -> bp_default_value_p))))
						{
							success_flag = true;
						}

				}
			else
				{
					success_flag = true;
				}
		}


	return success_flag;
}


static bool GetBooleanParameterDetailsFromJSON (Parameter *param_p, const json_t *param_json_p)
{
	BooleanParameter *boolean_param_p = (BooleanParameter *) param_p;
	bool success_flag = true;

	if (SetValueFromJSON (& (boolean_param_p -> bp_current_value_p), param_json_p, PARAM_CURRENT_VALUE_S))
		{
			if (!IsJSONParameterConcise (param_json_p))
				{
					if (SetValueFromJSON (& (boolean_param_p -> bp_default_value_p), param_json_p, PARAM_DEFAULT_VALUE_S))
						{
							success_flag = true;
						}
				}
		}

	return success_flag;
}


static bool SetBooleanParameterCurrentValueFromString (Parameter *param_p, const char *value_s)
{
	BooleanParameter *bool_param_p = (BooleanParameter *) param_p;
	bool success_flag = false;
	bool *b_p = NULL;
	bool b;

	if (value_s)
		{
			if (Stricmp (value_s, "true") == 0)
				{
					b = true;
					b_p = &b;
				}
			else if (Stricmp (value_s, "false") == 0)
				{
					b = false;
					b_p = &b;
				}
		}

	success_flag = SetBooleanParameterCurrentValue (bool_param_p, b_p);

	return success_flag;
}



static bool SetValueFromJSON (bool **value_pp, const json_t *param_json_p, const char *key_s)
{
	bool success_flag = false;
	bool b;

	if (GetJSONBoolean (param_json_p, key_s, &b))
		{
			success_flag = SetBooleanParameterValue (value_pp, &b);
		}
	else
		{
			success_flag = SetBooleanParameterValue (value_pp, NULL);
		}

	return success_flag;
}
