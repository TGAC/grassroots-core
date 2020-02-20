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
 * unsigned_int_parameter.c
 *
 *  Created on: 31 Jan 2020
 *      Author: billy
 */

#include "unsigned_int_parameter.h"
#include "memory_allocations.h"
#include "service.h"
#include "json_util.h"


/*
 * STATIC DECLARATIONS
 */

static bool SetUnsignedIntParameterValue (uint32 **param_value_pp, const uint32 *new_value_p);

static void ClearUnsignedIntParameter (Parameter *param_p);

static bool AddUnsignedIntParameterDetailsToJSON (const Parameter *param_p, json_t *param_json_p, const bool full_definition_flag);

static bool GetUnsignedIntParameterDetailsFromJSON (Parameter *param_p, const json_t *param_json_p);

static bool SetValueFromJSON (uint32 **value_pp, const json_t *param_json_p, const char *key_s);


static LinkedList *GetUnsignedIntParameterMultiOptions (UnsignedIntParameter *param_p);


static bool SetUnsignedIntParameterCurrentValueFromString (Parameter *param_p, const char *value_s);


static UnsignedIntParameter *GetNewUnsignedIntParameter (const uint32 *current_value_p, const uint32 *default_value_p);


/*
 * API DEFINITIONS
 */


static UnsignedIntParameter *GetNewUnsignedIntParameter (const uint32 *current_value_p, const uint32 *default_value_p)
{
	UnsignedIntParameter *param_p = (UnsignedIntParameter *) AllocMemory (sizeof (UnsignedIntParameter));

	if (param_p)
		{
			param_p -> uip_current_value_p = NULL;
			param_p -> uip_default_value_p = NULL;
			param_p -> uip_min_value_p = NULL;
			param_p -> uip_max_value_p = NULL;


			if (SetUnsignedIntParameterValue (& (param_p -> uip_current_value_p), current_value_p))
				{
					if (SetUnsignedIntParameterValue (& (param_p -> uip_default_value_p), default_value_p))
						{
							return param_p;
						}

					if (param_p -> uip_current_value_p)
						{
							FreeMemory (param_p -> uip_current_value_p);
						}
				}

			FreeMemory (param_p);
		}

	return NULL;
}


UnsignedIntParameter *AllocateUnsignedIntParameter (const struct ServiceData *service_data_p,
																										const char * const name_s, const char * const display_name_s,
																										const char * const description_s,
																										const uint32 *default_value_p, const uint32 *current_value_p,
																										ParameterLevel level)
{
	UnsignedIntParameter *param_p = GetNewUnsignedIntParameter (current_value_p, default_value_p);

	if (param_p)
		{
			if (InitParameter (& (param_p -> uip_base_param), service_data_p, PT_UNSIGNED_INT, name_s, display_name_s, description_s, level,
												 ClearUnsignedIntParameter, AddUnsignedIntParameterDetailsToJSON,
												 NULL, SetUnsignedIntParameterCurrentValueFromString))
				{
					if (service_data_p)
						{
//									GetParameterDefaultValueFromConfig (service_data_p, name_s, param_p -> pa_type, &default_value);
						}

					return param_p;
				}

			FreeParameter (& (param_p -> uip_base_param));
		}

	return NULL;
}



UnsignedIntParameter *AllocateUnsignedIntParameterFromJSON (const json_t *param_json_p, const struct Service *service_p)
{
	UnsignedIntParameter *param_p = NULL;
	uint32 *current_value_p = NULL;
	uint32 *default_value_p = NULL;
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
			param_p = GetNewUnsignedIntParameter (current_value_p, default_value_p);

			if (param_p)
				{
					if (InitParameterFromJSON (& (param_p -> uip_base_param), param_json_p, service_p, full_definition_flag))
						{
							SetParameterCallbacks (& (param_p -> uip_base_param), ClearUnsignedIntParameter, AddUnsignedIntParameterDetailsToJSON,
																		 NULL, SetUnsignedIntParameterCurrentValueFromString);

							return param_p;
						}
					else
						{
							ClearUnsignedIntParameter (& (param_p -> uip_base_param));
							FreeMemory (param_p);
							param_p = NULL;
						}
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

	return param_p;
}


Parameter *EasyCreateAndAddUnsignedIntParameterToParameterSet (const ServiceData *service_data_p, ParameterSet *params_p, ParameterGroup *group_p,
																								const char * const name_s, const char * const display_name_s, const char * const description_s,
																								const uint32 *default_value_p, uint8 level)
{
	return CreateAndAddUnsignedIntParameterToParameterSet (service_data_p, params_p, group_p, name_s, display_name_s, description_s, default_value_p, NULL, level);
}


Parameter *CreateAndAddUnsignedIntParameterToParameterSet (const ServiceData *service_data_p, ParameterSet *params_p, ParameterGroup *group_p,
																								const char * const name_s, const char * const display_name_s, const char * const description_s,
																								const uint32 *default_value_p, const uint32 *current_value_p, uint8 level)
{
	UnsignedIntParameter *int_param_p = AllocateUnsignedIntParameter (service_data_p, name_s, display_name_s, description_s, default_value_p, current_value_p, level);

	if (int_param_p)
		{
			Parameter *base_param_p = & (int_param_p -> uip_base_param);

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



const uint32 *GetUnsignedIntParameterCurrentValue (const UnsignedIntParameter *param_p)
{
	return param_p -> uip_current_value_p;
}


bool SetUnsignedIntParameterCurrentValue (UnsignedIntParameter *param_p, const uint32 *value_p)
{
	return SetUnsignedIntParameterValue (& (param_p -> uip_current_value_p), value_p);
}


const uint32 *GetUnsignedIntParameterDefaultValue (const UnsignedIntParameter *param_p)
{
	return param_p -> uip_default_value_p;
}


bool SetUnsignedIntParameterDefaultValue (UnsignedIntParameter *param_p, const uint32 *value_p)
{
	return SetUnsignedIntParameterValue (& (param_p -> uip_default_value_p), value_p);
}


bool SetUnsignedIntParameterBounds (UnsignedIntParameter *param_p, const uint32 min_value, const uint32 max_value)
{
	if (! (param_p -> uip_min_value_p))
		{
			param_p -> uip_min_value_p = (uint32 *) AllocMemory (sizeof (uint32));

			if (! (param_p -> uip_min_value_p))
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate min value");
					return false;
				}
		}

	if (! (param_p -> uip_max_value_p))
		{
			param_p -> uip_max_value_p = (uint32 *) AllocMemory (sizeof (uint32));

			if (! (param_p -> uip_max_value_p))
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate max value");
					return false;
				}
		}

	* (param_p -> uip_min_value_p) = min_value;
	* (param_p -> uip_max_value_p) = max_value;

	return true;
}


bool IsUnsignedIntParameterBounded (const UnsignedIntParameter *param_p)
{
	return ((param_p -> uip_min_value_p) && (param_p -> uip_max_value_p));
}


bool GetUnsignedIntParameterBounds (const UnsignedIntParameter *param_p, uint32 *min_p, uint32 *max_p)
{
	bool success_flag = false;

	if (IsUnsignedIntParameterBounded (param_p))
		{
			*min_p = * (param_p -> uip_min_value_p);
			*max_p = * (param_p -> uip_max_value_p);

			success_flag = true;
		}

	return success_flag;
}


bool IsUnsignedIntParameter (const Parameter *param_p)
{
	bool unsigned_int_param_flag = false;

	switch (param_p -> pa_type)
		{
			case PT_UNSIGNED_INT:
				unsigned_int_param_flag = true;
				break;

			default:
				break;
		}

	return unsigned_int_param_flag;
}



bool GetCurrentUnsignedIntParameterValueFromParameterSet (const ParameterSet * const params_p, const char * const name_s, const uint32 **value_pp)
{
	bool success_flag = false;
	Parameter *param_p = GetParameterFromParameterSetByName (params_p, name_s);

	if (param_p)
		{
			if (IsUnsignedIntParameter (param_p))
				{
					*value_pp = GetUnsignedIntParameterCurrentValue ((const UnsignedIntParameter *) param_p);
					success_flag = true;
				}
		}

	return success_flag;
}




bool CreateAndAddUnsignedIntParameterOption (UnsignedIntParameter *param_p, const uint32 value, const char *description_s)
{
	bool success_flag = false;
	LinkedList *options_p = GetUnsignedIntParameterMultiOptions (param_p);

	if (options_p)
		{
			UnsignedIntParameterOption *option_p = AllocateUnsignedIntParameterOption (value, description_s);

			if (option_p)
				{
					UnsignedIntParameterOptionNode *node_p = AllocateUnsignedIntParameterOptionNode (option_p);

					if (node_p)
						{
							LinkedListAddTail (options_p, & (node_p -> uipon_node));
							success_flag = true;
						}
					else
						{
							FreeUnsignedIntParameterOption (option_p);
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate option node with value " UINT32_FMT " and description \"%s\"", value, description_s);
						}
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate option with value " UINT32_FMT "and description \"%s\"", value, description_s);
				}
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get options list for parameter \"%s\"", param_p -> uip_base_param.pa_name_s);
		}


	return success_flag;
}



UnsignedIntParameterOption *AllocateUnsignedIntParameterOption (const uint32 value, const char *description_s)
{
	char *new_description_s  = NULL;

	if (CloneValidString (description_s, &new_description_s))
		{
			UnsignedIntParameterOption *option_p = (UnsignedIntParameterOption *) AllocMemory (sizeof (UnsignedIntParameterOption));

			if (option_p)
				{
					option_p -> uipo_value = value;
					option_p -> uipo_description_s = new_description_s;

					return option_p;
				}

			if (new_description_s)
				{
					FreeCopiedString (new_description_s);
				}
		}

	return NULL;
}


void FreeUnsignedIntParameterOption (UnsignedIntParameterOption *option_p)
{
	if (option_p -> uipo_description_s)
		{
			FreeCopiedString (option_p -> uipo_description_s);
		}

	FreeMemory (option_p);
}


UnsignedIntParameterOptionNode *AllocateUnsignedIntParameterOptionNode (UnsignedIntParameterOption *option_p)
{
	UnsignedIntParameterOptionNode *node_p = (UnsignedIntParameterOptionNode *) AllocMemory (sizeof (UnsignedIntParameterOptionNode));

	if (node_p)
		{
			InitListItem (& (node_p -> uipon_node));

			node_p -> uipon_option_p = option_p;
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate UnsignedIntParameterOptionNode with value " UINT32_FMT " and description \"%s\"", option_p -> uipo_value, option_p -> uipo_description_s);
		}

	return node_p;
}


void FreeUnsignedIntParameterOptionNode (ListItem *item_p)
{
	UnsignedIntParameterOptionNode *node_p = (UnsignedIntParameterOptionNode *) item_p;

	FreeUnsignedIntParameterOption (node_p -> uipon_option_p);
	FreeMemory (node_p);
}

/*
 * STATIC DEFINITIONS
 */

static bool SetUnsignedIntParameterValue (uint32 **param_value_pp, const uint32 *new_value_p)
{
	bool success_flag = true;

	if (new_value_p)
		{
			if (! (*param_value_pp))
				{
					*param_value_pp = (uint32 *) AllocMemory (sizeof (uint32));

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



static void ClearUnsignedIntParameter (Parameter *param_p)
{
	UnsignedIntParameter *int_param_p = (UnsignedIntParameter *) param_p;

	if (int_param_p -> uip_current_value_p)
		{
			FreeMemory (int_param_p -> uip_current_value_p);
			int_param_p -> uip_current_value_p = NULL;
		}

	if (int_param_p -> uip_default_value_p)
		{
			FreeMemory (int_param_p -> uip_default_value_p);
			int_param_p -> uip_current_value_p = NULL;
		}

	if (int_param_p -> uip_min_value_p)
		{
			FreeMemory (int_param_p -> uip_min_value_p);
			int_param_p -> uip_min_value_p = NULL;
		}

	if (int_param_p -> uip_max_value_p)
		{
			FreeMemory (int_param_p -> uip_max_value_p);
			int_param_p -> uip_max_value_p = NULL;
		}

}


static bool AddUnsignedIntParameterDetailsToJSON (const Parameter *param_p, json_t *param_json_p, const bool full_definition_flag)
{
	UnsignedIntParameter *int_param_p = (UnsignedIntParameter *) param_p;
	bool success_flag = false;

	if ((int_param_p -> uip_current_value_p == NULL ) || (SetJSONInteger (param_json_p, PARAM_CURRENT_VALUE_S, * (int_param_p -> uip_current_value_p))))
		{
			if (full_definition_flag)
				{
					if ((int_param_p -> uip_default_value_p == NULL ) || (SetJSONInteger (param_json_p, PARAM_DEFAULT_VALUE_S, * (int_param_p -> uip_default_value_p))))
						{
							if ((int_param_p -> uip_min_value_p == NULL ) || (SetJSONInteger (param_json_p, PARAM_MIN_S, * (int_param_p -> uip_min_value_p))))
								{
									if ((int_param_p -> uip_max_value_p == NULL ) || (SetJSONInteger (param_json_p, PARAM_MAX_S, * (int_param_p -> uip_max_value_p))))
										{
											success_flag = true;
										}
								}
						}

				}
			else
				{
					success_flag = true;
				}
		}

	return success_flag;
}



static bool GetUnsignedIntParameterDetailsFromJSON (Parameter *param_p, const json_t *param_json_p)
{
	UnsignedIntParameter *int_param_p = (UnsignedIntParameter *) param_p;
	bool success_flag = false;

	if (SetValueFromJSON (& (int_param_p -> uip_current_value_p), param_json_p, PARAM_CURRENT_VALUE_S))
		{
			if (SetValueFromJSON (& (int_param_p -> uip_default_value_p), param_json_p, PARAM_DEFAULT_VALUE_S))
				{
					if (SetValueFromJSON (& (int_param_p -> uip_min_value_p), param_json_p, PARAM_MIN_S))
						{
							if (SetValueFromJSON (& (int_param_p -> uip_max_value_p), param_json_p, PARAM_MAX_S))
								{
									success_flag = true;
								}
						}
				}
		}

	return success_flag;
}



static bool SetValueFromJSON (uint32 **value_pp, const json_t *param_json_p, const char *key_s)
{
	bool success_flag = false;
	const json_t *value_p = json_object_get (param_json_p, key_s);

	if (value_p)
		{
			if (json_is_integer (value_p))
				{
					json_int_t i = json_integer_value (value_p);

					if (i >= 0)
						{
							uint32 u = (uint32) i;
							success_flag = SetUnsignedIntParameterValue (value_pp, &u);
						}
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, value_p, "JSON value is less than zero, %d not an unisgned integer", value_p -> type);
						}
				}
			else if (json_is_null (value_p))
				{
					success_flag = SetUnsignedIntParameterValue (value_pp, NULL);
				}
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, value_p, "JSON value is of the wrong type, %d not integer", value_p -> type);
				}
		}
	else
		{
			success_flag = SetUnsignedIntParameterValue (value_pp, NULL);
		}

	return success_flag;
}


static bool SetUnsignedIntParameterCurrentValueFromString (Parameter *param_p, const char *value_s)
{
	bool success_flag = false;
	UnsignedIntParameter *int_param_p = (UnsignedIntParameter *) param_p;
	uint32 *value_p = NULL;
	uint32 value = 0;

	if (value_s)
		{
			if (sscanf (value_s, UINT32_FMT, &value) > 0)
				{
					value_p = &value;
				}
		}

	success_flag = SetUnsignedIntParameterCurrentValue (int_param_p, value_p);

	return success_flag;
}


static LinkedList *GetUnsignedIntParameterMultiOptions (UnsignedIntParameter *param_p)
{
	Parameter *base_param_p = & (param_p -> uip_base_param);

	if (! (base_param_p -> pa_options_p))
		{
			base_param_p -> pa_options_p = AllocateLinkedList (FreeUnsignedIntParameterOptionNode);

			if (! (base_param_p -> pa_options_p))
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate options list for parameter \"%s\"", base_param_p -> pa_name_s);
				}
		}

	return (base_param_p -> pa_options_p);
}



