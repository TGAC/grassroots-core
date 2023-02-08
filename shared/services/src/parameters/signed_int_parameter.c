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
 * signed_int_parameter.c
 *
 *  Created on: 30 Jan 2020
 *      Author: billy
 */

#include "signed_int_parameter.h"
#include "memory_allocations.h"
#include "service.h"
#include "json_util.h"


/*
 * STATIC DECLARATIONS
 */

static bool SetSignedIntParameterValue (int32 **param_value_pp, const int32 *new_value_p);

static void ClearSignedIntParameter (Parameter *param_p);

static bool AddSignedIntParameterDetailsToJSON (const struct Parameter *param_p, json_t *param_json_p, const bool full_definition_flag);

static bool AddSignedIntValueToJSON (json_t *param_json_p, const char *key_s, const int32 *value_p);


static bool GetSignedIntParameterDetailsFromJSON (Parameter *param_p, const json_t *param_json_p);

static bool SetValueFromJSON (int32 **value_pp, const json_t *param_json_p, const char *key_s);

static bool SetSignedIntParameterCurrentValueFromString (Parameter *param_p, const char *value_s);

static SignedIntParameter *GetNewSignedIntParameter (const int32 *current_value_p, const int32 *default_value_p);

static bool CopySignedIntParameterOptions (const SignedIntParameter *src_p, SignedIntParameter *dest_p);

static Parameter *CloneSignedIntParameter (const Parameter *param_p, const ServiceData *service_data_p);

static LinkedList *GetSignedIntParameterMultiOptions (SignedIntParameter *param_p);


/*
 * API DEFINITIONS
 */

static SignedIntParameter *GetNewSignedIntParameter (const int32 *current_value_p, const int32 *default_value_p)
{
	SignedIntParameter *param_p = (SignedIntParameter *) AllocMemory (sizeof (SignedIntParameter));

	if (param_p)
		{
			param_p -> sip_current_value_p = NULL;
			param_p -> sip_default_value_p = NULL;
			param_p -> sip_min_value_p = NULL;
			param_p -> sip_max_value_p = NULL;


			if (SetSignedIntParameterValue (& (param_p -> sip_current_value_p), current_value_p))
				{
					if (SetSignedIntParameterValue (& (param_p -> sip_default_value_p), default_value_p))
						{
							return param_p;
						}

					if (param_p -> sip_current_value_p)
						{
							FreeMemory (param_p -> sip_current_value_p);
						}
				}

			FreeMemory (param_p);
		}

	return NULL;
}


SignedIntParameter *AllocateSignedIntParameter (const struct ServiceData *service_data_p, const ParameterType pt,
																																			const char * const name_s, const char * const display_name_s,
																																			const char * const description_s,
																																			const int32 *default_value_p, const int32 *current_value_p,
																																			ParameterLevel level)
{
	SignedIntParameter *param_p = GetNewSignedIntParameter (current_value_p, default_value_p);

	if (param_p)
		{
			if (InitParameter (& (param_p -> sip_base_param), service_data_p, pt, name_s, display_name_s, description_s, level,
												 ClearSignedIntParameter, AddSignedIntParameterDetailsToJSON,
												 CloneSignedIntParameter, SetSignedIntParameterCurrentValueFromString))
				{
					if (service_data_p)
						{
//									GetParameterDefaultValueFromConfig (service_data_p, name_s, param_p -> pa_type, &default_value);
						}

					return param_p;
				}

			FreeParameter (& (param_p -> sip_base_param));
		}

	return NULL;
}



SignedIntParameter *AllocateSignedIntParameterFromJSON (const json_t *param_json_p, const struct Service *service_p, const bool concise_flag)
{
	SignedIntParameter *param_p = NULL;
	int32 *current_value_p = NULL;

	if (SetValueFromJSON (&current_value_p, param_json_p, PARAM_CURRENT_VALUE_S))
		{
			int32 *default_value_p = NULL;
			bool success_flag = true;

			if (!concise_flag)
				{
					if (!SetValueFromJSON (&default_value_p, param_json_p, PARAM_DEFAULT_VALUE_S))
						{
							success_flag = false;
						}
				}

			if (success_flag)
				{
					param_p = GetNewSignedIntParameter (current_value_p, default_value_p);

					if (param_p)
						{
							if (InitParameterFromJSON (& (param_p -> sip_base_param), param_json_p, service_p, concise_flag, NULL))
								{
									if (GetSignedIntParameterDetailsFromJSON (& (param_p -> sip_base_param), param_json_p))
										{
											SetParameterCallbacks (& (param_p -> sip_base_param), ClearSignedIntParameter, AddSignedIntParameterDetailsToJSON,
																						 NULL,
																						 SetSignedIntParameterCurrentValueFromString);
										}
									else
										{
											FreeParameter (& (param_p -> sip_base_param));
											param_p = NULL;
										}
								}
							else
								{
									ClearSignedIntParameter (& (param_p -> sip_base_param));
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
		}		/* if (SetValueFromJSON (&current_value_p, param_json_p, PARAM_CURRENT_VALUE_S)) */

	return param_p;
}


Parameter *EasyCreateAndAddSignedIntParameterToParameterSet (const ServiceData *service_data_p, ParameterSet *params_p, ParameterGroup *group_p, ParameterType type,
																								const char * const name_s, const char * const display_name_s, const char * const description_s,
																								const int32 *default_value_p, uint8 level)
{
	return CreateAndAddSignedIntParameterToParameterSet (service_data_p, params_p, group_p, type, name_s, display_name_s, description_s, default_value_p, default_value_p, level);
}


Parameter *CreateAndAddSignedIntParameterToParameterSet (const ServiceData *service_data_p, ParameterSet *params_p, ParameterGroup *group_p,  ParameterType type,
																								const char * const name_s, const char * const display_name_s, const char * const description_s,
																								const int32 *default_value_p, const int32 *current_value_p, uint8 level)
{
	SignedIntParameter *int_param_p = AllocateSignedIntParameter (service_data_p, type, name_s, display_name_s, description_s, default_value_p, current_value_p, level);

	if (int_param_p)
		{
			Parameter *base_param_p = & (int_param_p -> sip_base_param);

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




const int32 *GetSignedIntParameterCurrentValue (const SignedIntParameter *param_p)
{
	return param_p -> sip_current_value_p;
}


bool SetSignedIntParameterCurrentValue (SignedIntParameter *param_p, const int32 *value_p)
{
	return SetSignedIntParameterValue (& (param_p -> sip_current_value_p), value_p);
}


const int32 *GetSignedIntParameterDefaultValue (const SignedIntParameter *param_p)
{
	return param_p -> sip_default_value_p;
}


bool SetSignedIntParameterDefaultValue (SignedIntParameter *param_p, const int32 *value_p)
{
	return SetSignedIntParameterValue (& (param_p -> sip_default_value_p), value_p);
}


bool SetSignedIntParameterBounds (SignedIntParameter *param_p, const int32 min_value, const int32 max_value)
{
	if (! (param_p -> sip_min_value_p))
		{
			param_p -> sip_min_value_p = (int32 *) AllocMemory (sizeof (int32));

			if (! (param_p -> sip_min_value_p))
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate min value");
					return false;
				}
		}

	if (! (param_p -> sip_max_value_p))
		{
			param_p -> sip_max_value_p = (int32 *) AllocMemory (sizeof (int32));

			if (! (param_p -> sip_max_value_p))
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate max value");
					return false;
				}
		}

	* (param_p -> sip_min_value_p) = min_value;
	* (param_p -> sip_max_value_p) = max_value;

	return true;
}


bool IsSignedIntParameterBounded (const SignedIntParameter *param_p)
{
	return ((param_p -> sip_min_value_p) && (param_p -> sip_max_value_p));
}



void GetSignedIntParameterBounds (const SignedIntParameter *param_p, const int32 **min_pp, const int32 **max_pp)
{
	*min_pp = param_p -> sip_min_value_p;
	*max_pp = param_p -> sip_max_value_p;
}


bool IsSignedIntParameter (const Parameter *param_p)
{
	bool signed_int_param_flag = false;

	switch (param_p -> pa_type)
		{
			case PT_SIGNED_INT:
			case PT_NEGATIVE_INT:
				signed_int_param_flag = true;
				break;

			default:
				break;
		}

	return signed_int_param_flag;
}


bool GetCurrentSignedIntParameterValueFromParameterSet (const ParameterSet * const params_p, const char * const name_s, const int32 **value_pp)
{
	bool success_flag = false;
	Parameter *param_p = GetParameterFromParameterSetByName (params_p, name_s);

	if (param_p)
		{
			if (IsSignedIntParameter (param_p))
				{
					*value_pp = GetSignedIntParameterCurrentValue ((const SignedIntParameter *) param_p);
					success_flag = true;
				}
		}

	return success_flag;
}


bool CreateAndAddSignedIntParameterOption (SignedIntParameter *param_p, const int32 value, const char *description_s)
{
	bool success_flag = false;
	LinkedList *options_p = GetSignedIntParameterMultiOptions (param_p);

	if (options_p)
		{
			SignedIntParameterOption *option_p = AllocateSignedIntParameterOption (value, description_s);

			if (option_p)
				{
					SignedIntParameterOptionNode *node_p = AllocateSignedIntParameterOptionNode (option_p);

					if (node_p)
						{
							LinkedListAddTail (options_p, & (node_p -> sipon_node));
							success_flag = true;
						}
					else
						{
							FreeSignedIntParameterOption (option_p);
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
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get options list for parameter \"%s\"", param_p -> sip_base_param.pa_name_s);
		}


	return success_flag;
}



SignedIntParameterOption *AllocateSignedIntParameterOption (const int32 value, const char *description_s)
{
	char *new_description_s  = NULL;

	if (CloneValidString (description_s, &new_description_s))
		{
			SignedIntParameterOption *option_p = (SignedIntParameterOption *) AllocMemory (sizeof (SignedIntParameterOption));

			if (option_p)
				{
					option_p -> sipo_value = value;
					option_p -> sipo_description_s = new_description_s;

					return option_p;
				}

			if (new_description_s)
				{
					FreeCopiedString (new_description_s);
				}
		}

	return NULL;
}


void FreeSignedIntParameterOption (SignedIntParameterOption *option_p)
{
	if (option_p -> sipo_description_s)
		{
			FreeCopiedString (option_p -> sipo_description_s);
		}

	FreeMemory (option_p);
}


SignedIntParameterOptionNode *AllocateSignedIntParameterOptionNode (SignedIntParameterOption *option_p)
{
	SignedIntParameterOptionNode *node_p = (SignedIntParameterOptionNode *) AllocMemory (sizeof (SignedIntParameterOptionNode));

	if (node_p)
		{
			InitListItem (& (node_p -> sipon_node));

			node_p -> sipon_option_p = option_p;
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate SignedIntParameterOptionNode with value " UINT32_FMT " and description \"%s\"", option_p -> sipo_value, option_p -> sipo_description_s);
		}

	return node_p;
}


void FreeSignedIntParameterOptionNode (ListItem *item_p)
{
	SignedIntParameterOptionNode *node_p = (SignedIntParameterOptionNode *) item_p;

	FreeSignedIntParameterOption (node_p -> sipon_option_p);
	FreeMemory (node_p);
}


bool SetSignedIntParameterCurrentValueFromJSON (SignedIntParameter *param_p, const json_t *value_p)
{
	bool success_flag = false;

	if (value_p)
		{
			if (json_is_integer (value_p))
				{
					int32 i = json_integer_value (value_p);
					success_flag = SetSignedIntParameterValue (& (param_p -> sip_current_value_p), &i);
				}
			else if (json_is_null (value_p))
				{
					success_flag = SetSignedIntParameterValue (& (param_p -> sip_current_value_p), NULL);
				}
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, value_p, "JSON value is of the wrong type, %d not integer", value_p -> type);
				}
		}
	else
		{
			success_flag = SetSignedIntParameterValue (& (param_p -> sip_current_value_p), NULL);
		}

	return success_flag;

}

/*
 * STATIC DEFINITIONS
 */

static bool SetSignedIntParameterValue (int32 **param_value_pp, const int32 *new_value_p)
{
	bool success_flag = true;

	if (new_value_p)
		{
			if (! (*param_value_pp))
				{
					*param_value_pp = (int32 *) AllocMemory (sizeof (int32));

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



static void ClearSignedIntParameter (Parameter *param_p)
{
	SignedIntParameter *int_param_p = (SignedIntParameter *) param_p;

	if (int_param_p -> sip_current_value_p)
		{
			FreeMemory (int_param_p -> sip_current_value_p);
			int_param_p -> sip_current_value_p = NULL;
		}

	if (int_param_p -> sip_default_value_p)
		{
			FreeMemory (int_param_p -> sip_default_value_p);
			int_param_p -> sip_current_value_p = NULL;
		}

	if (int_param_p -> sip_min_value_p)
		{
			FreeMemory (int_param_p -> sip_min_value_p);
			int_param_p -> sip_min_value_p = NULL;
		}

	if (int_param_p -> sip_max_value_p)
		{
			FreeMemory (int_param_p -> sip_max_value_p);
			int_param_p -> sip_max_value_p = NULL;
		}
}


static bool AddSignedIntParameterDetailsToJSON (const Parameter *param_p, json_t *param_json_p, const bool full_definition_flag)
{
	SignedIntParameter *int_param_p = (SignedIntParameter *) param_p;
	bool success_flag = false;

	if (AddSignedIntValueToJSON (param_json_p, PARAM_CURRENT_VALUE_S, int_param_p -> sip_current_value_p))
		{
 			if (full_definition_flag)
				{
 					if (AddSignedIntValueToJSON (param_json_p, PARAM_DEFAULT_VALUE_S, int_param_p -> sip_default_value_p))
						{
							if (AddSignedIntValueToJSON (param_json_p, PARAM_MIN_S, int_param_p -> sip_min_value_p))
								{
									if (AddSignedIntValueToJSON (param_json_p, PARAM_MAX_S, int_param_p -> sip_max_value_p))
										{
											LinkedList *options_p = param_p -> pa_options_p;

											if ((options_p != NULL) && (options_p -> ll_size > 0))
												{
													json_t *json_options_p = json_array ();

													if (json_options_p)
														{
															SignedIntParameterOptionNode *node_p = (SignedIntParameterOptionNode *) (options_p -> ll_head_p);

															while (node_p)
																{
																	SignedIntParameterOption *option_p = node_p -> sipon_option_p;
																	json_t *value_p = json_integer (option_p -> sipo_value);

																	if (value_p)
																		{
																			json_t *item_p = json_object ();

																			success_flag = false;

																			if (item_p)
																				{
																					bool res_flag = true;

																					if (option_p -> sipo_description_s)
																						{
																							if (!SetJSONString (item_p, SHARED_TYPE_DESCRIPTION_S, option_p -> sipo_description_s))
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

																	node_p = (SignedIntParameterOptionNode *) (node_p -> sipon_node.ln_next_p);
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




static bool AddSignedIntValueToJSON (json_t *param_json_p, const char *key_s, const int32 *value_p)
{
	bool success_flag = false;

	if (value_p)
		{
			int i = (int) *value_p;
			success_flag = SetJSONInteger  (param_json_p, key_s, i);
		}
	else
		{
			success_flag = AddNullParameterValueToJSON (param_json_p, key_s);
		}

	return success_flag;
}




static bool GetSignedIntParameterDetailsFromJSON (Parameter *param_p, const json_t *param_json_p)
{
	SignedIntParameter *int_param_p = (SignedIntParameter *) param_p;
	bool success_flag = false;

	if (SetValueFromJSON (& (int_param_p -> sip_min_value_p), param_json_p, PARAM_MIN_S))
		{
			if (SetValueFromJSON (& (int_param_p -> sip_max_value_p), param_json_p, PARAM_MAX_S))
				{
					success_flag = true;
				}
		}

	return success_flag;
}


static bool SetValueFromJSON (int32 **value_pp, const json_t *param_json_p, const char *key_s)
{
	bool success_flag = false;
	const json_t *value_p = json_object_get (param_json_p, key_s);

	if (value_p)
		{
			if (json_is_integer (value_p))
				{
					int32 i = json_integer_value (value_p);
					success_flag = SetSignedIntParameterValue (value_pp, &i);
				}
			else if (json_is_null (value_p))
				{
					success_flag = SetSignedIntParameterValue (value_pp, NULL);
				}
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, value_p, "JSON value is of the wrong type, %d not integer", value_p -> type);
				}
		}
	else
		{
			success_flag = SetSignedIntParameterValue (value_pp, NULL);
		}

	return success_flag;
}


static bool SetSignedIntParameterCurrentValueFromString (Parameter *param_p, const char *value_s)
{
	bool success_flag = false;
	SignedIntParameter *int_param_p = (SignedIntParameter *) param_p;
	int32 *value_p = NULL;
	int32 value = 0;

	if (value_s)
		{
			if (sscanf (value_s, UINT32_FMT, &value) > 0)
				{
					value_p = &value;
				}
		}

	success_flag = SetSignedIntParameterCurrentValue (int_param_p, value_p);

	return success_flag;
}



static Parameter *CloneSignedIntParameter (const Parameter *param_p, const ServiceData *service_data_p)
{
	const SignedIntParameter *src_p = (const SignedIntParameter *) param_p;
	const int32 *default_value_p = GetSignedIntParameterDefaultValue (src_p);
	const int32 *current_value_p = GetSignedIntParameterCurrentValue (src_p);
	SignedIntParameter *dest_param_p = AllocateSignedIntParameter (service_data_p, param_p -> pa_type, param_p -> pa_name_s, param_p -> pa_display_name_s, param_p -> pa_description_s, default_value_p, current_value_p, param_p -> pa_level);

	if (dest_param_p)
		{
			bool success_flag = true;
			const int32 *min_value_p = NULL;
			const int32 *max_value_p = NULL;

			GetSignedIntParameterBounds (src_p, &min_value_p, &max_value_p);

			if (min_value_p && max_value_p)
				{
					if (!SetSignedIntParameterBounds (dest_param_p, *min_value_p, *max_value_p))
						{
							success_flag = false;
						}
				}

			if (success_flag)
				{
					if (CopySignedIntParameterOptions (src_p, dest_param_p))
						{
							return (& (dest_param_p -> sip_base_param));
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "CopySignedIntParameterOptions failed for copying \"%s\"", dest_param_p -> sip_base_param.pa_name_s);
						}

				}		/* if (SetStringParameterBounds (dest_param_p, min_value_s, max_value_s))  */
			else
				{
					if (min_value_p && max_value_p)
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "CopySignedIntParameterOptions failed for copying \"%s\", min \"" INT32_FMT "\", max \"" INT32_FMT "\"", dest_param_p -> sip_base_param.pa_name_s, *min_value_p, *max_value_p);
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "CopySignedIntParameterOptions failed for copying \"%s\"", dest_param_p -> sip_base_param.pa_name_s);
						}
				}

			FreeParameter (& (dest_param_p -> sip_base_param));
		}		/* if (dest_param_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "AllocateSignedIntParameter failed for copying \"%s\"", dest_param_p -> sip_base_param.pa_name_s);
		}


	return NULL;
}


static bool CopySignedIntParameterOptions (const SignedIntParameter *src_p, SignedIntParameter *dest_p)
{
	bool success_flag = true;
	const LinkedList *src_options_p = src_p -> sip_base_param.pa_options_p;

	if (src_options_p && (src_options_p -> ll_size > 0))
		{
			const SignedIntParameterOptionNode *src_node_p = (const SignedIntParameterOptionNode *) (src_options_p -> ll_head_p);

			while (src_node_p && success_flag)
				{
					const SignedIntParameterOption *option_p = src_node_p -> sipon_option_p;

					if (CreateAndAddSignedIntParameterOption (dest_p, option_p -> sipo_value, option_p -> sipo_description_s))
						{
							src_node_p = (SignedIntParameterOptionNode *) (src_node_p -> sipon_node.ln_next_p);
						}
					else
						{
							success_flag = false;
						}
				}
		}

	return success_flag;
}


static LinkedList *GetSignedIntParameterMultiOptions (SignedIntParameter *param_p)
{
	Parameter *base_param_p = & (param_p -> sip_base_param);

	if (! (base_param_p -> pa_options_p))
		{
			base_param_p -> pa_options_p = AllocateLinkedList (FreeSignedIntParameterOptionNode);

			if (! (base_param_p -> pa_options_p))
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate options list for parameter \"%s\"", base_param_p -> pa_name_s);
				}
		}

	return (base_param_p -> pa_options_p);
}

