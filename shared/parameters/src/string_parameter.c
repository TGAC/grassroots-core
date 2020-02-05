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
 * char_parameter.c
 *
 *  Created on: 30 Jan 2020
 *      Author: billy
 */

#include <string.h>

#include "string_parameter.h"
#include "memory_allocations.h"

#include "service.h"
#include "json_util.h"


/*
 * STATIC DECLARATIONS
 */

static void ClearStringParameter (Parameter *param_p);

static bool AddStringParameterDetailsToJSON (const Parameter *param_p, json_t *param_json_p);

static bool GetStringParameterDetailsFromJSON (Parameter *param_p, const json_t *param_json_p);

static LinkedList *GetStringParameterMultiOptions (StringParameter *param_p);


/*
 * API DEFINITIONS
 */

StringParameter *AllocateStringParameter (const struct ServiceData *service_data_p, const ParameterType pt, const char * const name_s, const char * const display_name_s, const char * const description_s, LinkedList *options_p, char *default_value_p, char *current_value_s, ParameterLevel level)
{
	StringParameter *param_p = (StringParameter *) AllocMemory (sizeof (StringParameter));

	if (param_p)
		{
			bool success_flag = true;

			if (current_value_s)
				{
					param_p -> sp_current_value_s = EasyCopyToNewString (current_value_s);

					if (! (param_p -> sp_current_value_s))
						{
							success_flag = false;
						}
				}
			else
				{
					param_p -> sp_current_value_s = NULL;
				}


			if (success_flag)
				{
					if (default_value_p)
						{
							param_p -> sp_default_value_s = EasyCopyToNewString (default_value_p);

							if (! (param_p -> sp_default_value_s))
								{
									success_flag = false;
								}
						}
					else
						{
							param_p -> sp_default_value_s = NULL;
						}
				}

			if (success_flag)
				{
					if (InitParameter (& (param_p -> sp_base_param), service_data_p, pt, name_s, display_name_s, description_s, options_p, level,
														 ClearStringParameter, AddStringParameterDetailsToJSON, GetStringParameterDetailsFromJSON,
														 NULL))
						{
							if (service_data_p)
								{
//									GetParameterDefaultValueFromConfig (service_data_p, name_s, param_p -> pa_type, &default_value);
								}

							return param_p;
						}
				}

			if (param_p -> sp_current_value_s)
				{
					FreeCopiedString (param_p -> sp_current_value_s);
				}

			if (param_p -> sp_default_value_s)
				{
					FreeCopiedString (param_p -> sp_default_value_s);
				}

			FreeMemory (param_p);
		}		/* if (param_p) */

	return NULL;
}


const char *GetStringParameterCurrentValue (const StringParameter *param_p)
{
	return param_p -> sp_current_value_s;
}


bool SetStringParameterCurrentValue (StringParameter *param_p, const char *value_p)
{
	return SetStringParameterValue (& (param_p -> sp_current_value_s), value_p);
}


const char *GetStringParameterDefaultValue (const StringParameter *param_p)
{
	return param_p -> sp_default_value_s;
}


bool SetStringParameterDefaultValue (StringParameter *param_p, const char *value_p)
{
	return SetStringParameterValue (& (param_p -> sp_default_value_s), value_p);
}


bool SetStringParameterBounds (StringParameter *param_p, const char *min_value_s, const char *max_value_s)
{
	bool success_flag = false;

	if (SetStringParameterValue (& (param_p -> sp_min_value_s), min_value_s))
		{
			if (SetStringParameterValue (& (param_p -> sp_max_value_s), max_value_s))
				{
					success_flag = true;
				}
		}


	return success_flag;
}


bool IsStringParameterBounded (const StringParameter *param_p)
{
	return ((param_p -> sp_min_value_s) && (param_p -> sp_max_value_s));
}


bool GetStringParameterBounds (const StringParameter *param_p, const char *min_p, const char *max_p)
{
	bool success_flag = false;

	if (IsStringParameterBounded (param_p))
		{
			*min_p = * (param_p -> sp_min_value_s);
			*max_p = * (param_p -> sp_max_value_s);

			success_flag = true;
		}

	return success_flag;
}


bool CreateAndAddStringParameterOption (const StringParameter *param_p, const char *value_s, const char *description_s)
{
	bool success_flag = false;
	LinkedList *options_p = GetStringParameterMultiOptions (param_p);

	if (options_p)
		{
			StringParameterOption *option_p = AllocateStringParameterOption (value_s, description_s);

			if (option_p)
				{
					StringParameterOptionNode *node_p = AllocateStringParameterOptionNode (option_p);

					if (node_p)
						{
							LinkedListAddTail (options_p, & (node_p -> spon_node));
							success_flag = true;
						}
					else
						{
							FreeParameterOption (option_p);
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate option node with value \"%s \" and description \"%s\"", value_s, description_s);
						}
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate option with value \"%s \" and description \"%s\"", value_s, description_s);
				}


		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get options list for parameter \"%s\"", param_p -> sp_base_param.pa_name_s);
		}


	return success_flag;
}



StringParameterOption *AllocateStringParameterOption (const char *value_s, const char *description_s)
{
	bool success_flag = true;
	char *new_value_s  = NULL;

	if (CloneValidString (value_s, &new_value_s))
		{
			char *new_description_s  = NULL;

			if (CloneValidString (description_s, &new_description_s))
				{
					StringParameterOption *option_p = (StringParameterOption *) AllocMemory (sizeof (StringParameterOption));

					if (option_p)
						{
							option_p -> spo_value_s = new_value_s;
							option_p -> spo_description_s = new_description_s;

							return option_p;
						}
				}

			if (new_description_s)
				{
					FreeCopiedString (new_description_s);
				}
		}

	if (new_value_s)
		{
			FreeCopiedString (new_value_s);
		}

	return NULL;
}


void FreeStringParameterOption (StringParameterOption *option_p)
{
	if (option_p -> spo_value_s)
		{
			FreeCopiedString (option_p -> spo_value_s);
		}

	if (option_p -> spo_description_s)
		{
			FreeCopiedString (option_p -> spo_description_s);
		}

	FreeMemory (option_p);
}


StringParameterOptionNode *AllocateStringParameterOptionNode (StringParameterOption *option_p)
{
	StringParameterOptionNode *node_p = (StringParameterOptionNode *) AllocMemory (sizeof (StringParameterOptionNode));

	if (node_p)
		{
			InitListItem (& (node_p -> spon_node));

			node_p -> spon_option_p = option_p;
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate StringParameterOptionNode with value \"%s \" and description \"%s\"", option_p -> spo_value_s, option_p -> spo_description_s);
		}

	return node_p;
}


void FreeStringParameterOptionNode (ListItem *item_p)
{
	StringParameterOptionNode *node_p = (StringParameterOptionNode *) item_p;

	FreeStringParameterOption (node_p -> spon_option_p);
}


/*
 * STATIC DEFINITIONS
 */

static bool SetStringParameterValue (char **param_value_pp, const char *new_value_p);

static bool SetStringParameterValue (char **param_value_ss, const char *new_value_s)
{
	bool success_flag = false;

	if (new_value_s)
		{
			char *copied_value_s = EasyCopyToNewString (new_value_s);

			if (copied_value_s)
				{
					if (*param_value_ss)
						{
							FreeCopiedString (*param_value_ss);
						}

					*param_value_ss = copied_value_s;
					success_flag = true;
				}
		}
	else
		{
			if (*param_value_ss)
				{
					FreeCopiedString (*param_value_ss);
					*param_value_ss = NULL;
				}

			success_flag = true;
		}

	return success_flag;
}



static void ClearStringParameter (Parameter *param_p)
{
	StringParameter *string_param_p = (StringParameter *) param_p;

	if (string_param_p -> sp_current_value_s)
		{
			FreeCopiedString (string_param_p -> sp_current_value_s);
			string_param_p -> sp_current_value_s = NULL;
		}

	if (string_param_p -> sp_default_value_s)
		{
			FreeCopiedString (string_param_p -> sp_default_value_s);
			string_param_p -> sp_default_value_s = NULL;
		}

	if (string_param_p -> sp_min_value_s)
		{
			FreeCopiedString (string_param_p -> sp_min_value_s);
			string_param_p -> sp_min_value_s = NULL;
		}

	if (string_param_p -> sp_max_value_s)
		{
			FreeCopiedString (string_param_p -> sp_max_value_s);
			string_param_p -> sp_max_value_s = NULL;
		}

}


static bool AddStringParameterDetailsToJSON (const Parameter *param_p, json_t *param_json_p, const bool full_definition_flag)
{
	StringParameter *string_param_p = (StringParameter *) param_p;
	bool success_flag = false;

	if ((string_param_p -> sp_current_value_s == NULL ) || (SetJSONInteger (param_json_p, PARAM_CURRENT_VALUE_S, * (string_param_p -> sp_current_value_s))))
		{
			if (full_definition_flag)
				{
					if ((string_param_p -> sp_default_value_s == NULL ) || (SetJSONInteger (param_json_p, PARAM_DEFAULT_VALUE_S, * (string_param_p -> sp_default_value_s))))
						{
							if ((string_param_p -> sp_min_value_s == NULL ) || (SetJSONInteger (param_json_p, PARAM_MIN_S, * (string_param_p -> sp_min_value_s))))
								{
									if ((string_param_p -> sp_max_value_s == NULL ) || (SetJSONInteger (param_json_p, PARAM_MAX_S, * (string_param_p -> sp_max_value_s))))
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


static bool GetStringParameterDetailsFromJSON (Parameter *param_p, const json_t *param_json_p)
{
	StringParameter *string_param_p = (StringParameter *) param_p;
	bool success_flag = true;
	const char *value_s = GetJSONString (param_json_p, PARAM_CURRENT_VALUE_S);

	if (value_s)
		{
			success_flag = SetStringParameterCurrentValue (string_param_p, value_s);
		}
	else
		{
			success_flag = SetStringParameterCurrentValue (string_param_p, NULL);
		}

	if (success_flag)
		{
			value_s = GetJSONString (param_json_p, PARAM_DEFAULT_VALUE_S);

			if (value_s)
				{
					success_flag = SetStringParameterCurrentValue (string_param_p, value_s);
				}
		}
	else
		{
			success_flag = SetStringParameterDefaultValue (string_param_p, NULL);
		}


	return success_flag;
}


static LinkedList *GetStringParameterMultiOptions (StringParameter *param_p)
{
	Parameter *base_param_p = & (param_p -> sp_base_param);

	if (! (base_param_p -> pa_options_p))
		{
			base_param_p -> pa_options_p = AllocateLinkedList (FreeStringParameterOptionNode);

			if (! (base_param_p -> pa_options_p))
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate options list for parameter \"%s\"", base_param_p -> pa_name_s);
				}
		}

	return (base_param_p -> pa_options_p);
}

