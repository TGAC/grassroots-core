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

static bool AddStringParameterDetailsToJSON (const Parameter *param_p, json_t *param_json_p, const bool full_definition_flag);

static bool GetStringParameterDetailsFromJSON (StringParameter *param_p, const json_t *param_json_p);

static bool SetStringParameterCurrentValueFromString (Parameter *param_p, const char *value_s);

static bool SetStringParameterValue (char **param_value_pp, const char *new_value_p);

static StringParameter *GetNewStringParameter (const char *current_value_s, const char *default_value_s);

static Parameter *CloneStringParameter (const Parameter *param_p, const ServiceData *service_data_p);

static bool SetStringJSONValue (json_t *param_json_p, const char *key_s, const char *value_s);

/*
 * API DEFINITIONS
 */




StringParameter *AllocateStringParameter (const struct ServiceData *service_data_p, const ParameterType pt,
																					const char * const name_s, const char * const display_name_s,
																					const char * const description_s,
																					const char *default_value_s, const char *current_value_s,
																					ParameterLevel level)
{
	StringParameter *param_p = GetNewStringParameter (current_value_s, default_value_s);

	if (param_p)
		{
			if (InitParameter (& (param_p -> sp_base_param), service_data_p, pt, name_s, display_name_s, description_s, level,
												 ClearStringParameter, AddStringParameterDetailsToJSON,
												 CloneStringParameter, SetStringParameterCurrentValueFromString))
				{
					if (service_data_p)
						{
//									GetParameterDefaultValueFromConfig (service_data_p, name_s, param_p -> pa_type, &default_value);
						}

					return param_p;
				}

			ClearParameter (& (param_p -> sp_base_param));
		}

	return NULL;
}


StringParameter *AllocateStringParameterFromJSON (const json_t *param_json_p, const struct Service *service_p, const bool concise_flag, const ParameterType *pt_p)
{
	StringParameter *param_p = NULL;
	const char *current_value_s = GetJSONString (param_json_p, PARAM_CURRENT_VALUE_S);
	const char *default_value_s = NULL;

	if (!concise_flag)
		{
			default_value_s = GetJSONString (param_json_p, PARAM_DEFAULT_VALUE_S);
		}

	param_p = GetNewStringParameter (current_value_s, default_value_s);

	if (param_p)
		{
			Parameter *base_param_p = & (param_p -> sp_base_param);

			if (InitParameterFromJSON (& (param_p -> sp_base_param), param_json_p, service_p, concise_flag, pt_p))
				{
					if (GetStringParameterDetailsFromJSON (param_p, param_json_p))
						{
							SetParameterCallbacks (& (param_p -> sp_base_param), ClearStringParameter, AddStringParameterDetailsToJSON,
																		 CloneStringParameter, SetStringParameterCurrentValueFromString);

							return param_p;
						}
					else
						{
							FreeParameter (base_param_p);
						}
				}
			else
				{
					ClearStringParameter (& (param_p -> sp_base_param));
					FreeMemory (param_p);
				}
		}

	return NULL;
}



Parameter *EasyCreateAndAddStringParameterToParameterSet (const ServiceData *service_data_p, ParameterSet *params_p, ParameterGroup *group_p, ParameterType type,
																											const char * const name_s, const char * const display_name_s, const char * const description_s,
																											const char *default_value_s, ParameterLevel level)
{
	return CreateAndAddStringParameterToParameterSet (service_data_p, params_p, group_p, type,
																										name_s, display_name_s, description_s,
																										default_value_s, default_value_s, level);
}

Parameter *CreateAndAddStringParameterToParameterSet (const ServiceData *service_data_p, ParameterSet *params_p, ParameterGroup *group_p, ParameterType type,
																											const char * const name_s, const char * const display_name_s, const char * const description_s,
																											const char *default_value_s, const char *current_value_s, ParameterLevel level)
{
	Parameter *base_param_p = NULL;
	StringParameter *string_param_p = AllocateStringParameter (service_data_p, type, name_s, display_name_s, description_s, default_value_s, current_value_s, level);

	if (string_param_p)
		{
			base_param_p = & (string_param_p -> sp_base_param);

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

			if (!AddParameterToParameterSet (params_p, base_param_p))
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add param \"%s\" to set \"%s\"", name_s, params_p -> ps_name_s);
					FreeParameter (base_param_p);
					base_param_p = NULL;
				}

		}		/* if (param_p) */

	return base_param_p;
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


bool GetStringParameterBounds (const StringParameter *param_p, const char **min_pp, const char **max_pp)
{
	bool success_flag = false;

	if (IsStringParameterBounded (param_p))
		{
			*min_pp = param_p -> sp_min_value_s;
			*max_pp = param_p -> sp_max_value_s;

			success_flag = true;
		}

	return success_flag;
}


bool CreateAndAddStringParameterOption (Parameter *param_p, const char *value_s, const char *description_s)
{
	bool success_flag = false;
	LinkedList *options_p = GetParameterStringMultiOptions (param_p);

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
							FreeStringParameterOption (option_p);
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
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get options list for parameter \"%s\"", param_p -> pa_name_s);
		}


	return success_flag;
}



StringParameterOption *AllocateStringParameterOption (const char *value_s, const char *description_s)
{
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
	FreeMemory (node_p);
}


bool IsStringParameter (const Parameter *param_p)
{
	bool string_param_flag = false;

	switch (param_p -> pa_type)
		{
			case PT_PASSWORD:
			case PT_STRING:
			case PT_KEYWORD:
			case PT_LARGE_STRING:
			case PT_TABLE:
			case PT_FASTA:
				string_param_flag = true;
				break;

			default:
				break;
		}

	return string_param_flag;
}


bool GetCurrentStringParameterValueFromParameterSet (const ParameterSet * const params_p, const char * const name_s, const char **value_pp)
{
	bool success_flag = false;
	Parameter *param_p = GetParameterFromParameterSetByName (params_p, name_s);

	if (param_p)
		{
			if (IsStringParameter (param_p))
				{
					const char *value_s = GetStringParameterCurrentValue ((const StringParameter *) param_p);

					*value_pp = value_s;
					success_flag = true;
				}
		}

	return success_flag;
}


bool GetCurrentStringParameterValueFromParameterGroup (const ParameterGroup * const params_p, const char * const name_s, const char **value_pp)
{
	bool success_flag = false;
	Parameter *param_p = GetParameterFromParameterGroupByName (params_p, name_s);

	if (param_p)
		{
			if (IsStringParameter (param_p))
				{
					const char *value_s = GetStringParameterCurrentValue ((const StringParameter *) param_p);

					*value_pp = value_s;
					success_flag = true;
				}
		}

	return success_flag;

}


bool SetStringParameterCurrentValueFromJSON (StringParameter *param_p, const json_t *value_p)
{
	bool success_flag = false;

	if ((value_p == NULL) || (json_is_null (value_p)))
		{
			success_flag = SetStringParameterValue (& (param_p -> sp_current_value_s), NULL);
		}
	else if (json_is_string (value_p))
		{
			success_flag = SetStringParameterValue (& (param_p -> sp_current_value_s), json_string_value (value_p));
		}

	return success_flag;
}


/*
 * STATIC DEFINITIONS
 */


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

	if (SetStringJSONValue (param_json_p, PARAM_CURRENT_VALUE_S, string_param_p -> sp_current_value_s))
		{
			if (full_definition_flag)
				{
					if (SetStringJSONValue (param_json_p, PARAM_DEFAULT_VALUE_S, string_param_p -> sp_default_value_s))
						{
							if (SetStringJSONValue (param_json_p, PARAM_MIN_S, string_param_p -> sp_min_value_s))
								{
									if (SetStringJSONValue (param_json_p, PARAM_MAX_S, string_param_p -> sp_max_value_s))
										{
											if (AddStringParameterOptionsToJSON (param_p, param_json_p))
												{
													success_flag = true;
												}

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



bool AddStringParameterOptionsToJSON (const Parameter *param_p, json_t *param_json_p)
{
	bool success_flag = false;
	LinkedList *options_p = param_p -> pa_options_p;

	if ((options_p != NULL) && (options_p -> ll_size > 0))
		{
			json_t *json_options_p = json_array ();

			if (json_options_p)
				{
					StringParameterOptionNode *node_p = (StringParameterOptionNode *) (options_p -> ll_head_p);

					while (node_p)
						{
							StringParameterOption *option_p = node_p -> spon_option_p;
							json_t *value_p = json_string (option_p -> spo_value_s);

							if (value_p)
								{
									json_t *item_p = json_object ();

									success_flag = false;

									if (item_p)
										{
											bool res_flag = true;

											if (option_p -> spo_description_s)
												{
													if (!SetJSONString (item_p, SHARED_TYPE_DESCRIPTION_S, option_p -> spo_description_s))
														{
															res_flag = false;
														}
												}

											if (res_flag)
												{
													if (json_object_set_new (item_p, SHARED_TYPE_VALUE_S, value_p) == 0)
														{
															success_flag = (json_array_append_new (json_options_p, item_p) == 0);
														}
												}

											if (!success_flag)
												{
													json_object_clear (item_p);
													json_decref (item_p);
												}
										}
								}

							node_p = (StringParameterOptionNode *) (node_p -> spon_node.ln_next_p);
						}		/* while (node_p) */

					if (success_flag)
						{
							if (json_object_set_new (param_json_p, PARAM_OPTIONS_S, json_options_p) == 0)
								{
									success_flag = true;
								}
							else
								{
									json_decref (json_options_p);
								}
						}

				}		/* if (json_options_p) */

		}
	else
		{
			success_flag = true;
		}

	return success_flag;
}



/**
 * Get the configured default value for a given Parameter.
 *
 * @param service_data_p The ServiceData for the Service that the given ParameterGroup belongs to.
 * @param param_name_s The name of the Parameter to check.
 * @param pt The ParameterType of the given Parameter.
 * @param value_p Pointer to where the value for configured default value for the given Parameter
 * will be stored.
 * @return <code>true</code> if the default value was set successfully, <code>false</code> otherwise.
 * @memberof Parameter
 */
bool GetStringParameterDefaultValueFromConfig (StringParameter *param_p, const ServiceData *service_data_p )
{
	bool found_flag = false;

	const json_t *param_config_p = GetParameterFromConfig (service_data_p -> sd_config_p, param_p -> sp_base_param.pa_name_s);

	if (param_config_p)
		{
			const char *default_value_s = GetJSONString (param_config_p, PARAM_DEFAULT_VALUE_S);

			if (default_value_s)
				{
					if (SetStringParameterDefaultValue (param_p, default_value_s))
						{
							found_flag = true;
						}
				}
		}

	return found_flag;
}


static StringParameter *GetNewStringParameter (const char *current_value_s, const char *default_value_s)
{
	StringParameter *param_p = (StringParameter *) AllocMemory (sizeof (StringParameter));

	if (param_p)
		{
			param_p -> sp_current_value_s = NULL;
			param_p -> sp_default_value_s = NULL;
			param_p -> sp_min_value_s = NULL;
			param_p -> sp_max_value_s = NULL;

			NullifyParameter (& (param_p -> sp_base_param));


			if (SetStringParameterValue (& (param_p -> sp_current_value_s), current_value_s))
				{
					if (SetStringParameterValue (& (param_p -> sp_default_value_s), default_value_s))
						{
							return param_p;
						}

					if (param_p -> sp_current_value_s)
						{
							FreeCopiedString (param_p -> sp_current_value_s);
						}
				}

			FreeMemory (param_p);
		}

	return NULL;
}




static bool GetStringParameterDetailsFromJSON (StringParameter *param_p, const json_t *param_json_p)
{
	const char *current_value_s = GetJSONString (param_json_p, PARAM_CURRENT_VALUE_S);
	const char *default_value_s = GetJSONString (param_json_p, PARAM_DEFAULT_VALUE_S);

	if (SetStringParameterCurrentValue (param_p, current_value_s))
		{
			if (SetStringParameterDefaultValue (param_p, default_value_s))
				{
					if (GetStringParameterOptionsFromJSON (& (param_p -> sp_base_param), param_json_p))
						{
							return true;
						}
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, param_json_p, "Failed to set param \"%s\" options from JSON", param_p -> sp_base_param.pa_name_s);
						}
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to set param \"%s\" default value \"%s\"", param_p -> sp_base_param.pa_name_s, default_value_s);
				}
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to set param \"%s\" current value \"%s\"", param_p -> sp_base_param.pa_name_s, current_value_s);
		}

	return false;
}




bool GetStringParameterOptionsFromJSON (Parameter *param_p, const json_t * const json_p)
{
	bool success_flag = true;
	json_t *options_json_p = json_object_get (json_p, PARAM_OPTIONS_S);

	if (options_json_p)
		{
			success_flag = false;

			if (json_is_array (options_json_p))
				{
					const size_t num_options = json_array_size (options_json_p);
					size_t i = 0;

					success_flag = true;

					while (success_flag && (i < num_options))
						{
							json_t *json_value_p = json_array_get (options_json_p, i);

							if (json_value_p)
								{
									const char *value_s = GetJSONString (json_value_p, SHARED_TYPE_VALUE_S);

									if (value_s)
										{
											const char *desc_s = GetJSONString (json_value_p, SHARED_TYPE_DESCRIPTION_S);

											if (!CreateAndAddStringParameterOption (param_p, value_s, desc_s))
												{
													success_flag = false;
												}
										}
								}

							if (success_flag)
								{
									++ i;
								}

						}		/* while (success_flag && (i < num_options)) */

				}
		}

	return success_flag;
}


static Parameter *CloneStringParameter (const Parameter *param_p, const ServiceData *service_data_p)
{
	const StringParameter *src_p = (const StringParameter *) param_p;
	const char *default_value_s = GetStringParameterDefaultValue (src_p);
	const char *current_value_s = GetStringParameterCurrentValue (src_p);
	const char *min_value_s = NULL;
	const char *max_value_s = NULL;

	if (GetStringParameterBounds (src_p, &min_value_s, &max_value_s))
		{
			StringParameter *dest_param_p = AllocateStringParameter (service_data_p, param_p -> pa_type, param_p -> pa_name_s, param_p -> pa_display_name_s, param_p -> pa_description_s, default_value_s, current_value_s, param_p -> pa_level);

			if (dest_param_p)
				{
					if (SetStringParameterBounds (dest_param_p, min_value_s, max_value_s))
						{
							if (CopyStringParameterOptions (param_p, & (dest_param_p -> sp_base_param), true))
								{
									return (& (dest_param_p -> sp_base_param));
								}
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "CopyStringParameterOptions failed for copying \"%s\"", dest_param_p -> sp_base_param.pa_name_s);
								}

						}		/* if (SetStringParameterBounds (dest_param_p, min_value_s, max_value_s))  */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "SetStringParameterBounds failed for copying \"%s\", min \"%s\", max \"%s\"", dest_param_p -> sp_base_param.pa_name_s, min_value_s ? min_value_s : "NULL", max_value_s ? max_value_s : "NULL");
						}

					FreeParameter (& (dest_param_p -> sp_base_param));
				}		/* if (dest_param_p) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "AllocateStringParameter failed for copying \"%s\"", dest_param_p -> sp_base_param.pa_name_s);
				}

		}		/* if (GetStringParameterBounds (src_p, &min_value_s, &max_value_s)) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "GetStringParameterBounds failed for copying \"%s\"", param_p -> pa_name_s);
		}

	return NULL;
}


static bool SetStringParameterCurrentValueFromString (Parameter *param_p, const char *value_s)
{
	StringParameter *string_param_p = (StringParameter *) param_p;

	return SetStringParameterCurrentValue (string_param_p, value_s);
}



bool CopyStringParameterOptions (const Parameter *src_p, Parameter *dest_p, const bool clear_existing_dest_options_flag)
{
	bool success_flag = true;
	const LinkedList *src_options_p = src_p -> pa_options_p;

	if (clear_existing_dest_options_flag)
		{
			ClearLinkedList (dest_p -> pa_options_p);
		}

	if (src_options_p && (src_options_p -> ll_size > 0))
		{
			StringParameterOptionNode *src_node_p = (StringParameterOptionNode *) (src_options_p -> ll_head_p);



			while (src_node_p && success_flag)
				{
					const StringParameterOption *option_p = src_node_p -> spon_option_p;

					if (CreateAndAddStringParameterOption (dest_p, option_p -> spo_value_s, option_p -> spo_description_s))
						{
							src_node_p = (StringParameterOptionNode *) (src_node_p -> spon_node.ln_next_p);
						}
					else
						{
							success_flag = false;
						}
				}
		}

	return success_flag;
}


static bool SetStringJSONValue (json_t *param_json_p, const char *key_s, const char *value_s)
{
	bool success_flag = false;

	if (value_s)
		{
			success_flag = SetJSONString (param_json_p, key_s, value_s);
		}
	else
		{
			success_flag = SetJSONNull (param_json_p, key_s);
		}

	return success_flag;
}

