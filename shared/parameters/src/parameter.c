/*
 ** Copyright 2014-2016 The Earlham Institute
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
#include <math.h>
#include <stddef.h>
#include <string.h>

#define ALLOCATE_PARAMETER_TAGS (1)
#include "parameter.h"
#include "memory_allocations.h"
#include "parameter_group.h"
#include "math_utils.h"
#include "string_utils.h"
#include "string_hash_table.h"
#include "parameter_set.h"
#include "service.h"
#include "schema_version.h"
#include "json_tools.h"
#include "json_util.h"
#include "time_util.h"


#include "boolean_parameter.h"
#include "char_parameter.h"
#include "double_parameter.h"
#include "json_parameter.h"
#include "resource_parameter.h"
#include "signed_int_parameter.h"
#include "string_parameter.h"
#include "time_parameter.h"
#include "unsigned_int_parameter.h"


#ifdef _DEBUG
#define PARAMETER_DEBUG	(STM_LEVEL_INFO)
#else
#define PARAMETER_DEBUG	(STM_LEVEL_NONE)
#endif



static const char *S_PARAM_TYPE_NAMES_SS [PT_NUM_TYPES] =
{
		"xsd:boolean",
		"params:signed_integer",
		"params:unsigned_integer",
		"params:negative_integer",
		"xsd:double",
		"params:unsigned_number",
		"xsd:string",
		"params:output_filename",
		"params:input_filename",
		"params:directory",
		"params:character",
		"params:password",
		"params:keyword",
		"params:large_string",
		"params:json",
		"params:tabular",
		"params:fasta",
		"xsd:date",
		"params:json_array"
};




static bool AddParameterNameToJSON (const char *name_s, json_t *root_p, const SchemaVersion * const sv_p);

static bool AddParameterDisplayNameToJSON (const Parameter * const param_p, json_t *root_p, const SchemaVersion * const sv_p);

static bool AddParameterDescriptionToJSON (const Parameter * const param_p, json_t *root_p, const SchemaVersion * const sv_p);


static bool AddParameterTypeToJSON (const ParameterType param_type, json_t *root_p, const SchemaVersion * const sv_p, const bool full_definition_flag);

static bool AddParameterGroupToJSON (const Parameter * const param_p, json_t *json_p, const SchemaVersion * const sv_p);

static bool AddParameterStoreToJSON (const Parameter * const param_p, json_t *root_p, const SchemaVersion * const sv_p);

static bool AddParameterRefreshToJSON (const Parameter * const param_p, json_t *root_p, const SchemaVersion * const sv_p);


static bool AddRemoteParameterDetailsToJSON (const Parameter * const param_p, json_t *root_p, const SchemaVersion * const sv_p);


static bool AddParameterVisibilityToJSON (const Parameter * const param_p, json_t *root_p, const SchemaVersion * const sv_p);

static bool CopyBaseParamaeter (const Parameter *src_p, Parameter *dest_p);


static bool GetParameterTypeFromJSON (const json_t * const json_p, ParameterType *param_type_p);

static bool GetParameterLevelFromJSON (const json_t * const json_p, ParameterLevel *level_p);

static bool InitParameterStoreFromJSON (const json_t *root_p, HashTable *store_p);

static bool SetRemoteParameterDetailsFromJSON (Parameter *param_p, const json_t * json_p);


static const json_t *GetParameterFromConfig (const json_t *service_config_p, const char * const param_name_s);

static bool GetParameterStringFromConfig (const json_t *service_config_p, const char * const param_name_s, const char * const key_s, char **value_ss);



static bool AddCompoundGrassrootsType (json_t *value_p, const ParameterType pt);


static bool GetParameterTypeFromCompoundObject (const json_t *root_p, ParameterType *pt_p);

static bool AddSeparateGrassrootsTypes (json_t *value_p, const ParameterType pt);

static bool GetParameterTypeFromSeparateObjects (const json_t * const json_p, ParameterType *param_type_p);




/******************************************************/


Parameter *CreateParameterFromJSON (const json_t * const root_p, Service *service_p)
{
	Parameter *param_p = NULL;
	const char *name_s = GetJSONString (root_p, PARAM_NAME_S);

	#if SERVER_DEBUG >= STM_LEVEL_FINE
	PrintJSONToLog (STM_LEVEL_FINE, __FILE__, __LINE__, root_p, "InitParameterFromJSON");
	#endif

	if (name_s)
		{
			bool got_type_flag = false;
			ParameterType pt = PT_NUM_TYPES;

			if (service_p)
				{
					if (GetParameterTypeForNamedParameter (service_p, name_s, &pt))
						{
							got_type_flag = true;
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get ParameterType for %s in %s", name_s, GetServiceName (service_p));
						}

				}		/* if (service_p) */
			else
				{
					if (GetParameterTypeFromJSON (root_p, &pt))
						{
							got_type_flag = true;
						}
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, root_p, "Failed to get ParameterType for %s from JSON", name_s);
						}
				}

			if (got_type_flag)
				{
					switch (pt)
						{
							case PT_BOOLEAN:
								{
									BooleanParameter *bool_param_p = AllocateBooleanParameterFromJSON (root_p, service_p);

									if (bool_param_p)
										{
											param_p = & (bool_param_p -> bp_base_param);
										}
								}
							break;

						case PT_CHAR:
							{
								CharParameter *char_param_p = AllocateCharParameterFromJSON (root_p, service_p);

								if (char_param_p)
									{
										param_p = & (char_param_p -> cp_base_param);
									}
							}
							break;

						case PT_JSON:
						case PT_JSON_TABLE:
							{
								JSONParameter *json_param_p = AllocateJSONParameterFromJSON (root_p, service_p);

								if (json_param_p)
									{
										param_p = & (json_param_p -> jp_base_param);
									}
							}
							break;

						case PT_PASSWORD:
						case PT_STRING:
						case PT_KEYWORD:
						case PT_LARGE_STRING:
						case PT_TABLE:
						case PT_FASTA:
							{
								StringParameter *string_param_p = AllocateStringParameterFromJSON (root_p, service_p);

								if (string_param_p)
									{
										param_p = & (string_param_p -> sp_base_param);
									}
							}
							break;

						case PT_FILE_TO_WRITE:
						case PT_FILE_TO_READ:
						case PT_DIRECTORY:
							{
								ResourceParameter *res_param_p = AllocateResourceParameterFromJSON (root_p, service_p);

								if (res_param_p)
									{
										param_p = & (res_param_p -> rp_base_param);
									}
							}
							break;

						case PT_TIME:
							{
								TimeParameter *time_param_p = AllocateTimeParameterFromJSON (root_p, service_p);

								if (time_param_p)
									{
										param_p = & (time_param_p -> tp_base_param);
									}
							}
							break;

						case PT_SIGNED_INT:
						case PT_NEGATIVE_INT:
							{
								SignedIntParameter *int_param_p = AllocateSignedIntParameterFromJSON (root_p, service_p);

								if (int_param_p)
									{
										param_p = & (int_param_p -> sip_base_param);
									}
							}
							break;

						case PT_UNSIGNED_INT:
							{
								UnsignedIntParameter *int_param_p = AllocateUnsignedIntParameterFromJSON (root_p, service_p);

								if (int_param_p)
									{
										param_p = & (int_param_p -> uip_base_param);
									}
							}
							break;

						case PT_SIGNED_REAL:
						case PT_UNSIGNED_REAL:
							{
								DoubleParameter *double_param_p = AllocateDoubleParameterFromJSON (root_p, service_p);

								if (double_param_p)
									{
										param_p = & (double_param_p -> dp_base_param);
									}
							}
							break;

						case PT_NUM_TYPES:
							break;
						}

				}		/* if (got_type_flag) */

		}		/* if (name_s) */

	return param_p;
}


bool InitParameterFromJSON (Parameter *param_p, const json_t * const root_p, const Service *service_p, const bool full_definition_flag)
{
	bool init_flag = false;
	const char *name_s = GetJSONString (root_p, PARAM_NAME_S);

	#if SERVER_DEBUG >= STM_LEVEL_FINE
	PrintJSONToLog (STM_LEVEL_FINE, __FILE__, __LINE__, root_p, "InitParameterFromJSON");
	#endif

	if (name_s)
		{
			bool got_type_flag = false;
			ParameterType pt = PT_NUM_TYPES;

			if (service_p)
				{
					if (GetParameterTypeForNamedParameter (service_p, name_s, &pt))
						{
							got_type_flag = true;
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get ParameterType for %s in %s", name_s, GetServiceName (service_p));
						}

				}		/* if (service_p) */
			else
				{
					if (GetParameterTypeFromJSON (root_p, &pt))
						{
							got_type_flag = true;
						}
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, root_p, "Failed to get ParameterType for %s from JSON", name_s);
						}
				}

			if (got_type_flag)
				{
					/*
					 * The default, options, display name and bounds are optional
					 */
					const char *description_s = NULL;
					const char *display_name_s = NULL;
					LinkedList *options_p = NULL;
					ParameterLevel level = PL_ALL;
					bool success_flag = false;

					if (GetParameterLevelFromJSON (root_p, &level))
						{

						}


					if (full_definition_flag || (!IsJSONParameterConcise (root_p)))
						{
							description_s = GetJSONString (root_p, PARAM_DESCRIPTION_S);
							display_name_s = GetJSONString (root_p, PARAM_DISPLAY_NAME_S);
						}
					else
						{
							success_flag = true;
						}

					if (success_flag)
						{
							bool flag = true;

							if (GetJSONBoolean (root_p, PARAM_VISIBLE_S, &flag))
								{
									if (!flag)
										{
											param_p -> pa_visible_flag = flag;
										}
								}


							flag = false;
							if (GetJSONBoolean (root_p, PARAM_REFRESH_S, &flag))
								{
									param_p -> pa_refresh_service_flag = flag;
								}


							/* AllocateParameter made a deep copy of the current and default values, so we can deallocate our cached copies */

							if (SetRemoteParameterDetailsFromJSON (param_p, root_p))
								{
									success_flag = InitParameterStoreFromJSON (root_p, param_p -> pa_store_p);
								}
							else
								{
									success_flag = false;
								}

							if (!success_flag)
								{
									FreeParameter (param_p);
									param_p = NULL;
								}
						}

				}		/* if (got_type_flag) */

		}		/* if (name_s) */

	return init_flag;
}


bool InitParameter (Parameter *param_p, const struct ServiceData *service_data_p, ParameterType type, const char * const name_s,
										const char * const display_name_s, const char * const description_s, LinkedList *options_p, ParameterLevel level,
										void (*clear_fn) (Parameter *param_p),
										bool (*add_values_to_json_fn) (const Parameter *param_p, json_t *param_json_p, const bool full_definition_flag),
										bool (*get_values_from_json_fn) (Parameter *param_p, const json_t *param_json_p, const bool full_definition_flag),
										struct Parameter (*clone_fn) (const Parameter *param_p),
										bool (*set_value_from_string_fn) (struct Parameter *param_p, const char *value_s)
)
{
	char *new_name_s = CopyToNewString (name_s, 0, true);

	if (new_name_s)
		{
			bool success_flag = true;
			char *new_description_s = NULL;

			if (description_s)
				{
					new_description_s = CopyToNewString (description_s, 0, true);
					success_flag = (new_description_s != NULL);
				}

			if (success_flag)
				{
					char *new_display_name_s = NULL;

					if (display_name_s)
						{
							new_display_name_s = CopyToNewString (display_name_s, 0, true);
							success_flag = (new_display_name_s != NULL);
						}

					if (success_flag)
						{
							HashTable *store_p = GetHashTableOfStrings (8, 75);

							if (store_p)
								{
									LinkedList *remote_params_p = AllocateLinkedList (FreeRemoteParameterDetailsNode);

									if (remote_params_p)
										{
											param_p -> pa_type = type;
											param_p -> pa_name_s = new_name_s;
											param_p -> pa_display_name_s = new_display_name_s;
											param_p -> pa_description_s = new_description_s;
											param_p -> pa_options_p = options_p;
											param_p -> pa_level = level;
											param_p -> pa_store_p = store_p;
											param_p -> pa_group_p = NULL;

											param_p -> pa_remote_parameter_details_p = remote_params_p;

											param_p -> pa_visible_flag = true;
											param_p -> pa_refresh_service_flag = false;

											param_p -> pa_required_flag = true;

											SetParameterCallbacks (param_p, clear_fn, add_values_to_json_fn, get_values_from_json_fn, clone_fn, set_value_from_string_fn);

											/*
											 * Check for any values that have been overrode in
											 * the service configuration.
											 */
											if (service_data_p)
												{
													//GetParameterDefaultValueFromConfig (service_data_p, name_s, param_p -> pa_type, &default_value);

													GetParameterLevelFromConfig (service_data_p, name_s, & (param_p -> pa_level));

													GetParameterDescriptionFromConfig (service_data_p, name_s, & (param_p -> pa_description_s));
												}
										}

									FreeLinkedList (remote_params_p);
								}		/* if (remote_params_p) */

							FreeHashTable (store_p);
						}		/* if (store_p) */


					if (new_display_name_s)
						{
							FreeCopiedString (new_display_name_s);
						}		/* if (new_description_s) */


					if (new_description_s)
						{
							FreeCopiedString (new_description_s);
						}		/* if (new_description_s) */

				}		/* if (success_flag) */

			FreeCopiedString (new_name_s);
		}		/* if (new_name_s) */

	return false;
}



//Parameter *AllocateParameter (const ServiceData *service_data_p, ParameterType type, const char * const name_s, const char * const display_name_s, const char * const description_s, LinkedList *options_p, SharedType default_value, SharedType *current_value_p, ParameterBounds *bounds_p, ParameterLevel level, const char *(*check_value_fn) (const Parameter * const parameter_p, const void *value_p))
//{
//	char *new_name_s = CopyToNewString (name_s, 0, true);
//
//	if (new_name_s)
//		{
//			bool success_flag = true;
//			char *new_description_s = NULL;
//
//			if (description_s)
//				{
//					new_description_s = CopyToNewString (description_s, 0, true);
//					success_flag = (new_description_s != NULL);
//				}
//
//			if (success_flag)
//				{
//					char *new_display_name_s = NULL;
//
//					if (display_name_s)
//						{
//							new_display_name_s = CopyToNewString (display_name_s, 0, true);
//							success_flag = (new_display_name_s != NULL);
//						}
//
//					if (success_flag)
//						{
//							HashTable *store_p = GetHashTableOfStrings (8, 75);
//
//							if (store_p)
//								{
//									LinkedList *remote_params_p = AllocateLinkedList (FreeRemoteParameterDetailsNode);
//
//									if (remote_params_p)
//										{
//											Parameter *param_p = (Parameter *) AllocMemory (sizeof (Parameter));
//
//											if (param_p)
//												{
//													param_p -> pa_type = type;
//													param_p -> pa_name_s = new_name_s;
//													param_p -> pa_display_name_s = new_display_name_s;
//													param_p -> pa_description_s = new_description_s;
//													param_p -> pa_options_p = options_p;
//													param_p -> pa_check_value_fn = check_value_fn;
//													param_p -> pa_bounds_p = bounds_p;
//													param_p -> pa_level = level;
//													param_p -> pa_store_p = store_p;
//													param_p -> pa_group_p = NULL;
//
//													param_p -> pa_remote_parameter_details_p = remote_params_p;
//
//													param_p -> pa_visible_flag = true;
//													param_p -> pa_refresh_service_flag = false;
//
//													param_p -> pa_required_flag = true;
//
//													/*
//													 * Check for any values that have been overrode in
//													 * the service configuration.
//													 */
//													if (service_data_p)
//														{
//															GetParameterDefaultValueFromConfig (service_data_p, name_s, param_p -> pa_type, &default_value);
//
//															GetParameterLevelFromConfig (service_data_p, name_s, & (param_p -> pa_level));
//
//															GetParameterDescriptionFromConfig (service_data_p, name_s, & (param_p -> pa_description_s));
//														}
//
//													if (SetParameterValueFromSharedType (param_p, current_value_p ? current_value_p : &default_value, true))
//														{
//															if (SetParameterValueFromSharedType (param_p, &default_value, false))
//																{
//																	return param_p;
//																}		/* if (SetParameterValueFromSharedType (param_p, &default_value, false)) */
//															else
//																{
//																	PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to set param current value for %s", param_p -> pa_name_s);
//																}
//														}		/* if (SetParameterValueFromSharedType (param_p, current_value_p ? current_value_p : &default_value, true)) */
//													else
//														{
//															PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to set param default value for %s", param_p -> pa_name_s);
//														}
//
//													FreeMemory (param_p);
//												}		/* if (param_p) */
//
//											FreeLinkedList (remote_params_p);
//										}		/* if (remote_params_p) */
//
//									FreeHashTable (store_p);
//								}		/* if (store_p) */
//
//						}
//
//					if (new_display_name_s)
//						{
//							FreeCopiedString (new_display_name_s);
//						}		/* if (new_description_s) */
//
//
//					if (new_description_s)
//						{
//							FreeCopiedString (new_description_s);
//						}		/* if (new_description_s) */
//
//				}		/* if (success_flag) */
//
//			FreeCopiedString (new_name_s);
//		}		/* if (new_name_s) */
//
//	return NULL;
//}


void ClearParameter (Parameter *param_p)
{
#if PARAMETER_DEBUG >= STM_LEVEL_FINER
	PrintLog (STM_LEVEL_FINER, __FILE__, __LINE__, "Clearing parameter %s", param_p -> pa_name_s);
#endif


	if (param_p -> pa_description_s)
		{
			FreeCopiedString (param_p -> pa_description_s);
		}

	if (param_p -> pa_name_s)
		{
			FreeCopiedString (param_p -> pa_name_s);
		}

	if (param_p -> pa_display_name_s)
		{
			FreeCopiedString (param_p -> pa_display_name_s);
		}

	if (param_p -> pa_options_p)
		{
			FreeLinkedList (param_p -> pa_options_p);
		}


	FreeHashTable (param_p -> pa_store_p);

	FreeLinkedList (param_p -> pa_remote_parameter_details_p);
}


void FreeParameter (Parameter *param_p)
{
	ClearParameter (param_p);
	FreeMemory (param_p);
}





Parameter *CloneParameter (const Parameter * const src_p)
{
	Parameter *dest_p = src_p -> pa_clone_fn (src_p);

	if (dest_p)
		{
			if (CopyBaseParamaeter (src_p, dest_p))
				{
					return dest_p;
				}

			FreeParameter (dest_p);
		}

	return NULL;
}


static bool CopyBaseParamaeter (const Parameter *src_p, Parameter *dest_p)
{
	bool success_flag = true;
	LinkedList *dest_options_p = NULL;

	if (src_p -> pa_options_p)
		{
			dest_options_p = CloneProgramOptionsList (src_p -> pa_options_p);

			if (!dest_options_p)
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to to clone parameter options for\"%s\"", src_p -> pa_name_s);
					success_flag = false;
				}
		}


	if (success_flag)
		{
			if (CopyRemoteParameterDetails (src_p, dest_p))
				{
					HashTable *dest_store_p = NULL;

					if (GetHashTableSize (src_p -> pa_store_p) > 0)
						{
							dest_store_p = CopyHashTable (src_p -> pa_store_p, true);
						}
					else
						{
							dest_store_p = GetHashTableOfStrings (8, 75);
						}


					if (dest_store_p)
						{
							char *dest_name_s = NULL;

							if (CloneValidString (src_p -> pa_name_s, &dest_name_s))
								{
									char *dest_display_name_s = NULL;

									if (CloneValidString (src_p -> pa_display_name_s, &dest_display_name_s))
										{
											char *dest_description_s = NULL;

											if (CloneValidString (src_p -> pa_description_s, &dest_description_s))
												{
													dest_p -> pa_type = src_p -> pa_type;
													dest_p -> pa_name_s = dest_name_s;
													dest_p -> pa_display_name_s = dest_display_name_s;
													dest_p -> pa_description_s = dest_description_s;
													dest_p -> pa_options_p = dest_options_p;
													dest_p -> pa_level = src_p -> pa_level;
													dest_p -> pa_visible_flag = src_p -> pa_visible_flag;
													dest_p -> pa_refresh_service_flag = src_p -> pa_refresh_service_flag;
													dest_p -> pa_required_flag = src_p -> pa_required_flag;

													dest_p -> pa_clear_fn = src_p -> pa_clear_fn;
													dest_p -> pa_add_values_to_json_fn = src_p -> pa_add_values_to_json_fn;
													dest_p -> pa_get_values_from_json_fn = src_p -> pa_get_values_from_json_fn;
													dest_p -> pa_clone_fn = src_p -> pa_clone_fn;


													dest_p -> pa_group_p = src_p -> pa_group_p;
													dest_p -> pa_store_p = src_p -> pa_store_p;

													return true;
												}		/* if (CloneValidString (src_p -> pa_description_s, &dest_description_s)) */

											FreeCopiedString (dest_display_name_s);
										}		/* if (CloneValidString (src_p -> pa_display_name_s, &dest_display_name_s)) */

									FreeCopiedString (dest_name_s);
								}		/* if (CloneValidString (src_p -> pa_name_s, &dest_name_s)) */

						}		/* if (dest_store_p) */

				}		/* if (CopyRemoteParameterDetails (src_p, dest_p)) */

		}		/* if (success_flag) */

	return false;
}


ParameterNode *AllocateParameterNode (Parameter *param_p)
{
	ParameterNode *node_p = (ParameterNode *) AllocMemory (sizeof (ParameterNode));

	if (node_p)
		{
			node_p -> pn_node.ln_prev_p = NULL;
			node_p -> pn_node.ln_next_p = NULL;

			node_p -> pn_parameter_p = param_p;
		}		/* if (node_p) */

	return node_p;
}


void FreeParameterNode (ListItem *node_p)
{
	ParameterNode *param_node_p = (ParameterNode *) node_p;

	if (param_node_p -> pn_parameter_p)
		{
			FreeParameter (param_node_p -> pn_parameter_p);
		}

	FreeMemory (param_node_p);
}


bool CopyRemoteParameterDetails (const Parameter * const src_p, Parameter *dest_p)
{
	bool success_flag = true;

	if (src_p -> pa_remote_parameter_details_p -> ll_size)
		{
			const RemoteParameterDetailsNode *src_node_p = (const RemoteParameterDetailsNode *) (src_p -> pa_remote_parameter_details_p -> ll_head_p);

			while (src_node_p && success_flag)
				{
					const RemoteParameterDetails *src_details_p = src_node_p -> rpdn_details_p;

					if (AddRemoteDetailsToParameter (dest_p, src_details_p -> rpd_server_uri_s, src_details_p -> rpd_name_s))
						{
							src_node_p = (const RemoteParameterDetailsNode *) (src_node_p -> rpdn_node.ln_next_p);
						}
					else
						{
							success_flag = false;
						}
				}
		}
	else
		{
			ClearLinkedList (dest_p -> pa_remote_parameter_details_p);
		}

	return success_flag;
}


const char *CheckForSignedReal (const Parameter * const UNUSED_PARAM (parameter_p), const void *value_p)
{
	const char *error_s = NULL;

	if (value_p)
		{
			if (CompareDoubles (* ((double64 *) value_p), 0.0) < 0)
				{
					error_s = "The value must be non-negative";
				}
		}
	else
		{
			error_s = "The value is required";
		}

	return error_s;
}


const char *CheckForNotNull (const Parameter * const UNUSED_PARAM (parameter_p), const void *value_p)
{
	if (!value_p)
		{
			return "The value is required";
		}
	else
		{
			return NULL;
		}
}



bool CompareParameterLevels (const ParameterLevel param_level, const ParameterLevel threshold)
{
	bool show_flag = false;

	switch (param_level)
	{
		case PL_ALL:
			show_flag = true;
			break;

		case PL_SIMPLE:
			if (threshold != PL_ADVANCED)
				{
					show_flag = true;
				}
			break;

		case PL_ADVANCED:
			if (threshold != PL_SIMPLE)
				{
					show_flag = true;
				}
			break;
	}		/* switch (param_level) */

	return show_flag;
}


bool AddParameterKeyJSONValuePair (Parameter * const parameter_p, const char *key_s, const json_t *value_p)
{
	bool success_flag = false;
	char *value_s = json_dumps (value_p, JSON_INDENT (2));

	if (value_s)
		{
			success_flag = PutInHashTable (parameter_p -> pa_store_p, key_s, value_s);
			free (value_s);
		}
	else
		{
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, value_p, "json_dumps failed for \"%s\"", key_s);
		}

	return success_flag;
}


bool AddParameterKeyStringValuePair (Parameter * const parameter_p, const char *key_s, const char *value_s)
{
	bool success_flag = PutInHashTable (parameter_p -> pa_store_p, key_s, value_s);

	if (!success_flag)
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "PutInHashTable failed for \"%s\": \"%s\"", key_s, value_s);
		}

	return success_flag;
}


void RemoveParameterKeyValuePair (Parameter * const parameter_p, const char *key_s)
{
	RemoveFromHashTable (parameter_p -> pa_store_p, key_s);
}


const char *GetParameterKeyValue (const Parameter * const parameter_p, const char *key_s)
{
	return ((const char *) GetFromHashTable (parameter_p -> pa_store_p, key_s));
}



json_t *GetRunnableParameterAsJSON (const Parameter *param_p, const SchemaVersion * const sv_p, const bool full_definition_flag)
{
	json_t *root_p = json_object ();

	if (root_p)
		{
			if (AddParameterNameToJSON (param_p -> pa_name_s, root_p, sv_p))
				{
					if (AddParameterValuesToJSON (param_p, root_p, full_definition_flag))
						{
							return root_p;
						}
				}

			json_decref (root_p);
		}

	return NULL;
}


json_t *GetParameterAsJSON (const Parameter * const param_p, const SchemaVersion * const sv_p, const bool full_definition_flag)
{
	json_t *root_p = GetRunnableParameterAsJSON (param_p, sv_p, full_definition_flag);

	if (root_p)
		{
			bool success_flag = false;

			if (AddParameterStoreToJSON (param_p, root_p, sv_p))
				{
					if (AddRemoteParameterDetailsToJSON (param_p, root_p, sv_p))
						{
							if (AddParameterGroupToJSON (param_p, root_p, sv_p))
								{
									if (full_definition_flag)
										{
											if (AddParameterTypeToJSON (param_p -> pa_type, root_p, sv_p, full_definition_flag))
												{
													if (AddParameterLevelToJSON (param_p -> pa_level, root_p, sv_p))
														{
															if (AddParameterDescriptionToJSON (param_p, root_p, sv_p))
																{
																	if (AddParameterDisplayNameToJSON (param_p, root_p, sv_p))
																		{
																			if (AddParameterOptionsToJSON (param_p, root_p, sv_p))
																				{
																					if (AddParameterVisibilityToJSON (param_p, root_p, sv_p))
																						{
																							if (AddParameterRefreshToJSON (param_p, root_p, sv_p))
																								{
																									success_flag = true;
																								}		/* if (AddParameterRefreshToJSON (param_p, root_p, sv_p)) */
																							else
																								{
																									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed: AddParameterRefreshToJSON for \"%s\"", param_p -> pa_name_s);
																								}

																						}		/* if (AddParameterVisibilityToJSON (param_p, root_p, sv_p)) */
																					else
																						{
																							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed: AddParameterVisibilityToJSON for \"%s\"", param_p -> pa_name_s);
																						}

																				}		/* if (AddParameterOptionsToJSON (param_p, root_p)) */
																			else
																				{
																					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed: AddParameterOptionsToJSON for \"%s\"", param_p -> pa_name_s);
																				}

																		}		/* if (AddParameterDisplayNameToJSON (param_p, root_p)) */
																	else
																		{
																			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed: AddParameterDisplayNameToJSON for \"%s\"", param_p -> pa_name_s);
																		}

																}		/* if (AddParameterDescriptionToJSON (param_p, root_p)) */
															else
																{
																	PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed: AddParameterDescriptionToJSON for \"%s\"", param_p -> pa_name_s);
																}
														}		/* if (AddParameterLevelToJSON (param_p, root_p)) */
													else
														{
															PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed: AddParameterLevelToJSON for \"%s\"", param_p -> pa_name_s);
														}

												}		/* if (AddParameterTypeToJSON (param_p -> pa_type, root_p, sv_p, full_definition_flag)) */
											else
												{
													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed: AddParameterTypeToJSON for \"%s\"", param_p -> pa_name_s);
												}

										}		/* if (full_definition_flag) */
									else
										{
											success_flag = true;
										}
								}		/* if (AddParameterGroupToJSON (param_p, root_p, sv_p)) */
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed: AddParameterGroupToJSON for \"%s\"", param_p -> pa_name_s);
								}

						}		/* if (AddParameterRemoteDetailsToJSON (param_p, root_p)) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed: AddParameterRemoteDetailsToJSON for \"%s\"", param_p -> pa_name_s);
						}

				}		/* if (AddParameterStoreToJSON (param_p, root_p)) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed: AddParameterStoreToJSON for \"%s\"", param_p -> pa_name_s);
				}

			if (!success_flag)
				{
					json_decref (root_p);
					root_p = NULL;
				}

		}		/* if (root_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate JSON for parameter");
		}

	return root_p;
}


static bool AddParameterNameToJSON (const char *name_s, json_t *root_p, const SchemaVersion * const UNUSED_PARAM (sv_p))
{
	bool success_flag = (json_object_set_new (root_p, PARAM_NAME_S, json_string (name_s)) == 0);

#if SERVER_DEBUG >= STM_LEVEL_FINER
	PrintJSONToLog (root_p, "AddParameterNameToJSON - root_p :: ", STM_LEVEL_FINER, __FILE__, __LINE__);
#endif

	return success_flag;
}



static bool AddParameterDisplayNameToJSON (const Parameter * const param_p, json_t *root_p, const SchemaVersion * const UNUSED_PARAM (sv_p))
{
	bool success_flag = true;

	if (param_p -> pa_display_name_s)
		{
			success_flag = (json_object_set_new (root_p, PARAM_DISPLAY_NAME_S, json_string (param_p -> pa_display_name_s)) == 0);
		}

#if SERVER_DEBUG >= STM_LEVEL_FINER
	PrintJSONToLog (root_p, "AddParameterDisplayNameToJSON - root_p :: ", STM_LEVEL_FINER, __FILE__, __LINE__);
#endif

	return success_flag;
}


static bool AddParameterDescriptionToJSON (const Parameter * const param_p, json_t *root_p, const SchemaVersion * const UNUSED_PARAM (sv_p))
{
	bool success_flag = true;

	if (param_p -> pa_description_s)
		{
			success_flag = (json_object_set_new (root_p, PARAM_DESCRIPTION_S, json_string (param_p -> pa_description_s)) == 0);
		}

#if SERVER_DEBUG >= STM_LEVEL_FINER
	PrintJSONToLog (root_p, "AddParameterDescriptionToJSON - root_p :: ", STM_LEVEL_FINER, __FILE__, __LINE__);
#endif

	return success_flag;
}


static bool AddRemoteParameterDetailsToJSON (const Parameter * const param_p, json_t *root_p, const SchemaVersion * const UNUSED_PARAM (sv_p))
{
	bool success_flag = true;

	if (param_p -> pa_remote_parameter_details_p -> ll_size > 0)
		{
			json_t *remote_details_list_json_p = json_array ();

			if (remote_details_list_json_p)
				{
					RemoteParameterDetailsNode *node_p = (RemoteParameterDetailsNode *) (param_p -> pa_remote_parameter_details_p -> ll_head_p);

					while (node_p && success_flag)
						{
							json_t *remote_details_json_p = GetRemoteParameterDetailsAsJSON (node_p -> rpdn_details_p);

							if (remote_details_json_p)
								{
									if (json_array_append_new (remote_details_list_json_p, remote_details_json_p) == 0)
										{
											node_p = (RemoteParameterDetailsNode *) (node_p -> rpdn_node.ln_next_p);
										}
									else
										{
											json_decref (remote_details_json_p);
											success_flag = false;
										}

								}		/* if (remote_details_json_p) */
							else
								{
									success_flag = false;
								}

						}		/* while (node_p && success_flag) */

					if (!success_flag)
						{
							json_decref (remote_details_list_json_p);
						}

				}		/* if (remote_details_list_json_p) */
			else
				{
					success_flag = false;
				}

		}		/* if (param_p -> pa_remote_parameter_details_p -> ll_size > 0) */


#if SERVER_DEBUG >= STM_LEVEL_FINER
	PrintJSONToLog (root_p, "AddRemoteParameterDetailsToJSON - root_p :: ", STM_LEVEL_FINER, __FILE__, __LINE__);
#endif

	return success_flag;
}


const char *GetParameterLevelAsString (const ParameterLevel level)
{
	const char *level_s = NULL;

	switch (level)
		{
			case PL_SIMPLE:
				level_s = PARAM_LEVEL_TEXT_SIMPLE_S;
				break;

			case PL_ADVANCED:
				level_s = PARAM_LEVEL_TEXT_ADVANCED_S;
				break;

			case PL_ALL:
				level_s = PARAM_LEVEL_TEXT_ALL_S;
				break;

			default:
				PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Unknown ParameterLevel %d", level);
				break;
		}		/* switch (param_p -> pa_level) */

	return level_s;
}



bool GetParameterLevelFromString (const char *level_s, ParameterLevel *level_p)
{
	bool match_flag = true;

	if (strcmp (level_s, PARAM_LEVEL_TEXT_SIMPLE_S) == 0)
		{
			*level_p = PL_SIMPLE;
		}
	else if (strcmp (level_s, PARAM_LEVEL_TEXT_ADVANCED_S) == 0)
		{
			*level_p = PL_ADVANCED;
		}
	else if (strcmp (level_s, PARAM_LEVEL_TEXT_ALL_S) == 0)
		{
			*level_p = PL_ALL;
		}
	else
		{
			match_flag = false;
		}

	return match_flag;
}


bool AddParameterLevelToJSON (const ParameterLevel level, json_t *root_p, const SchemaVersion * const UNUSED_PARAM (sv_p))
{
	bool success_flag = false;
	const char *level_s = GetParameterLevelAsString (level);

	if (level_s)
		{
			if (SetJSONString (root_p, PARAM_LEVEL_S, level_s))
				{
					success_flag = true;
				}
		}

#if SERVER_DEBUG >= STM_LEVEL_FINER
	PrintJSONToLog (root_p, "AddParameterLevelToJSON - root_p :: ", STM_LEVEL_FINER, __FILE__, __LINE__);
#endif

	return success_flag;
}


static bool AddParameterStoreToJSON (const Parameter * const param_p, json_t *root_p, const SchemaVersion * const UNUSED_PARAM (sv_p))
{
	bool success_flag = true;
	uint32 i = GetHashTableSize (param_p -> pa_store_p);

	if (i > 0)
		{
			void **keys_pp = GetKeysIndexFromHashTable (param_p -> pa_store_p);

			if (keys_pp)
				{
					json_t *store_json_p = json_object ();

					if (store_json_p)
						{
							void **key_pp = keys_pp;

							while (success_flag && (i > 0))
								{
									const char *key_s = *key_pp;
									const char *value_s = GetParameterKeyValue (param_p, key_s);

									/*
									 * Is the value a valid JSON object?
									 */
									json_error_t err;
									json_t *value_p = json_loads (value_s, 0, &err);

									if (value_p)
										{
											if (json_object_set_new (store_json_p, key_s, value_p) != 0)
												{
													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, store_json_p, "Failed to add JSON for key \"%s\" from \"%s\"", key_s, value_s);
													success_flag = false;
													json_decref (value_p);
												}
										}
									else if (!SetJSONString (store_json_p, key_s, value_s))
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, store_json_p, "Failed to add string from key \"%s\" from \"%s\"", key_s, value_s);
											success_flag = false;
										}

									if (success_flag)
										{
											++ key_pp;
											-- i;
										}
								}

							if (success_flag)
								{
									if (json_object_set_new (root_p, PARAM_STORE_S, store_json_p) == 0)
										{
											success_flag = true;
										}
								}

							if (!success_flag)
								{
									json_object_clear (store_json_p);
									json_decref (store_json_p);
								}

						}		/* if (store_json_p) */

					FreeKeysIndex (keys_pp);
				}
		}

	return success_flag;
}


static bool AddParameterTypeToJSON (const ParameterType param_type, json_t *root_p, const SchemaVersion * const sv_p, const bool full_definition_flag)
{
	bool success_flag = false;

	if (full_definition_flag)
		{
			/* Set the parameter type */
			switch (param_type)
				{
					case PT_BOOLEAN:
						success_flag = (json_object_set_new (root_p, PARAM_TYPE_S, json_string (PA_TYPE_BOOLEAN_S)) == 0);
						break;

					case PT_CHAR:
						success_flag = (json_object_set_new (root_p, PARAM_TYPE_S, json_string (PA_TYPE_STRING_S)) == 0);
						break;

					case PT_SIGNED_INT:
					case PT_NEGATIVE_INT:
					case PT_UNSIGNED_INT:
						success_flag = (json_object_set_new (root_p, PARAM_TYPE_S, json_string (PA_TYPE_INTEGER_S)) == 0);
						break;

					case PT_SIGNED_REAL:
					case PT_UNSIGNED_REAL:
						success_flag = (json_object_set_new (root_p, PARAM_TYPE_S, json_string (PA_TYPE_NUMBER_S)) == 0);
						break;

					case PT_STRING:
					case PT_TABLE:
					case PT_LARGE_STRING:
					case PT_PASSWORD:
					case PT_FILE_TO_WRITE:
					case PT_DIRECTORY:
					case PT_KEYWORD:
					case PT_FASTA:
						success_flag = (json_object_set_new (root_p, PARAM_TYPE_S, json_string (PA_TYPE_STRING_S)) == 0);
						break;

					case PT_FILE_TO_READ:
						success_flag = (json_object_set_new (root_p, PARAM_TYPE_S, json_string (PA_TYPE_STRING_S)) == 0);
						break;

					case PT_JSON:
					case PT_JSON_TABLE:
						success_flag = (json_object_set_new (root_p, PARAM_TYPE_S, json_string ("json")) == 0);
						break;

					case PT_TIME:
						success_flag = (json_object_set_new (root_p, PARAM_TYPE_S, json_string (PA_TYPE_STRING_S)) == 0);
						break;

					case PT_NUM_TYPES:
						PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, root_p, "Parameter has invalid type");
						break;
				}		/* switch (param_p -> pa_type) */

		}		/* if (full_definition_flag) */
	else
		{
			success_flag = true;
		}

	if (success_flag)
		{
			if ((sv_p -> sv_major == 0) && (sv_p -> sv_minor == 1))
				{
					success_flag = AddSeparateGrassrootsTypes (root_p, param_type);
				}
			else if ((sv_p -> sv_major == 0) && (sv_p -> sv_minor <= 9))
				{
					success_flag = AddCompoundGrassrootsType (root_p, param_type);
				}
			else
				{
					const char *param_type_s = * (S_PARAM_TYPE_NAMES_SS + param_type);

					success_flag = (json_object_set_new (root_p, PARAM_GRASSROOTS_TYPE_INFO_TEXT_S, json_string (param_type_s)) == 0);
				}

			if (!success_flag)
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, root_p, "Failed to add grassroots type for %d", param_type);
				}		/* if (!AddCompoundGrassrootsType (root_p, param_p -> pa_type)) */

		}		/* if (success_flag) */
	else
		{
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, root_p, "Failed to add type for %d", param_type);
		}

#if SERVER_DEBUG >= STM_LEVEL_FINER
	PrintJSONToLog (root_p, "AddParameterTypeToJSON - root_p :: ", STM_LEVEL_FINER, __FILE__, __LINE__);
#endif

	return success_flag;
}


static bool AddCompoundGrassrootsType (json_t *value_p, const ParameterType pt)
{
	bool success_flag = false;
	json_t *grassroots_p = json_object ();

	if (grassroots_p)
		{
			if (json_object_set_new (grassroots_p, PARAM_COMPOUND_VALUE_S, json_integer (pt)) == 0)
				{
					const char *type_s = GetGrassrootsTypeAsString (pt);

					if (type_s)
						{
							if (json_object_set_new (grassroots_p, PARAM_COMPOUND_TEXT_S, json_string (type_s)) == 0)
								{
									if (json_object_set_new (value_p, PARAM_GRASSROOTS_S, grassroots_p) == 0)
										{
											success_flag = true;
										}
									else
										{
											PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to add compound %s: %s to the json definition", PARAM_COMPOUND_TEXT_S, type_s);
										}

								}
							else
								{
									PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to add compound %s: %s to the json definition", PARAM_COMPOUND_TEXT_S, type_s);
								}
						}
					else
						{
							PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to get human-readable type string for %d", pt);
						}
				}
			else
				{
					PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to add compound %s: %d to the json definition", PARAM_GRASSROOTS_TYPE_INFO_S, pt);
				}

			if (!success_flag)
				{
					json_decref (grassroots_p);
				}
		}
	else
		{
			PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to allocate json object for comppond grassroots type");
		}

	return success_flag;
}


static bool GetParameterTypeFromCompoundObject (const json_t *root_p, ParameterType *pt_p)
{
	bool success_flag = false;

	const json_t *grassroots_p = json_object_get (root_p, PARAM_GRASSROOTS_S);

	if (grassroots_p)
		{
			const json_t *value_p = json_object_get (grassroots_p, PARAM_COMPOUND_VALUE_S);

			if (value_p)
				{
					if (json_is_integer (value_p))
						{
							json_int_t subtype = json_integer_value (value_p);

							if ((subtype >= 0) && (subtype < PT_NUM_TYPES))
								{
									*pt_p = subtype;
									success_flag = true;
								}
							else
								{
									PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to get grassroots type value from " JSON_INTEGER_FORMAT, subtype);
								}
						}
					else
						{
							PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "json value for %s is %d not a int ", PARAM_COMPOUND_VALUE_S, json_typeof (value_p));
						}
				}
			else if ((value_p = json_object_get (value_p, PARAM_COMPOUND_TEXT_S)) != NULL)
				{
					if (json_is_string (value_p))
						{
							const char *value_s = json_string_value (value_p);

							if (value_s)
								{
									if (GetGrassrootsTypeFromString (value_s, pt_p))
										{
											success_flag = true;
										}
									else
										{
											PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to get grassroots type from \"%s\"", value_s);
										}
								}
							else
								{
									PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to get string from json value for %s", PARAM_COMPOUND_TEXT_S);
								}
						}
					else
						{
							PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "json value for %s is %d not a string ", PARAM_COMPOUND_TEXT_S, json_typeof (value_p));
						}
				}

		}		/* if (grassroots_p) */


	return success_flag;
}


static bool AddSeparateGrassrootsTypes (json_t *value_p, const ParameterType pt)
{
	bool success_flag = false;

	if (json_object_set_new (value_p, PARAM_GRASSROOTS_TYPE_INFO_S, json_integer (pt)) == 0)
		{
			const char *type_s = GetGrassrootsTypeAsString (pt);

			if (type_s)
				{
					if (json_object_set_new (value_p, PARAM_GRASSROOTS_TYPE_INFO_TEXT_S, json_string (type_s)) != 0)
						{
							PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to add %s: %s to the json defintiion", PARAM_GRASSROOTS_TYPE_INFO_TEXT_S, type_s);
						}
				}
			else
				{
					PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to get human-readable type string for %d", pt);
				}
		}
	else
		{
			PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to add %s: %d to the json defintiion", PARAM_GRASSROOTS_TYPE_INFO_S, pt);
			success_flag = false;
		}


	return success_flag;
}


static bool GetParameterTypeFromSeparateObjects (const json_t * const json_p, ParameterType *param_type_p)
{
	bool success_flag = false;
	json_t *value_p = json_object_get (json_p, PARAM_GRASSROOTS_TYPE_INFO_S);

	if (value_p)
		{
			if (json_is_integer (value_p))
				{
					json_int_t subtype = json_integer_value (value_p);

					if ((subtype >= 0) && (subtype < PT_NUM_TYPES))
						{
							*param_type_p = subtype;
							success_flag = true;
						}
					else
						{
							PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to get grassroots type value from " JSON_INTEGER_FORMAT, subtype);
						}
				}
			else
				{
					PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "json value for %s is %d not a int ", PARAM_GRASSROOTS_TYPE_INFO_S, json_typeof (value_p));
				}
		}
	else if ((value_p = json_object_get (json_p, PARAM_GRASSROOTS_TYPE_INFO_TEXT_S)) != NULL)
		{
			if (json_is_string (value_p))
				{
					const char *value_s = json_string_value (value_p);

					if (value_s)
						{
							if (GetGrassrootsTypeFromString (value_s, param_type_p))
								{
									success_flag = true;
								}
							else
								{
									PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to get grassroots type from \"%s\"", value_s);
								}
						}
					else
						{
							PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to get string from json value for %s", PARAM_GRASSROOTS_TYPE_INFO_TEXT_S);
						}
				}
			else
				{
					PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "json value for %s is %d not a string ", PARAM_GRASSROOTS_TYPE_INFO_TEXT_S, json_typeof (value_p));
				}
		}

	return success_flag;
}


bool AddParameterValuesToJSON (const struct Parameter *param_p, json_t *param_json_p, const bool full_definition_flag)
{
	return (param_p -> pa_add_values_to_json_fn (param_p, param_json_p, full_definition_flag));
}

static bool AddParameterVisibilityToJSON (const Parameter * const param_p, json_t *root_p, const SchemaVersion * const sv_p)
{
	bool success_flag = true;

	if (! (param_p -> pa_visible_flag))
		{
			success_flag = SetJSONBoolean (root_p, PARAM_VISIBLE_S, false);
		}

	return success_flag;
}


static bool AddParameterRefreshToJSON (const Parameter * const param_p, json_t *root_p, const SchemaVersion * const sv_p)
{
	bool success_flag = true;

	if (param_p -> pa_refresh_service_flag)
		{
			success_flag = SetJSONBoolean (root_p, PARAM_REFRESH_S, true);
		}

	return success_flag;
}


const char *GetGrassrootsTypeAsString (const ParameterType param_type)
{
	const char *res_s = NULL;

	if ((param_type >= 0) && (param_type < PT_NUM_TYPES))
		{
			return * (S_PARAM_TYPE_NAMES_SS + param_type);
		}
	else
		{
			PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "param type %d is out of range", param_type);
		}

	return res_s;
}


bool GetGrassrootsTypeFromString (const char *param_type_s, ParameterType *param_type_p)
{
	const char **type_ss = S_PARAM_TYPE_NAMES_SS;
	ParameterType i;

	for (i = 0; i < PT_NUM_TYPES; ++ i, ++ type_ss)
		{
			if (strcmp (*type_ss, param_type_s) == 0)
				{
					*param_type_p = i;
					return true;
				}
		}

	return false;
}



bool AddColumnParameterHint (const char *name_s, const ParameterType param_type, json_t *array_p)
{
	json_t *hint_p = json_object ();

	if (hint_p)
		{
			if (SetJSONString (hint_p, PARAM_NAME_S, name_s))
				{
					const char *type_s = GetGrassrootsTypeAsString (param_type);

					if (type_s)
						{
							if (SetJSONString (hint_p, PARAM_TYPE_S, type_s))
								{
									if (json_array_append_new (array_p, hint_p) == 0)
										{
											return true;
										}
									else
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, hint_p, "json_array_append_new failed");
										}
								}
							else
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, hint_p, "SetJSONString failed for \"%s\": \"%s\"", PARAM_TYPE_S, name_s);
								}
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "GetGrassrootsTypeAsString failed for \"%s\": %d", name_s, param_type);
						}
				}
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, hint_p, "SetJSONString failed for \"%s\": \"%s\"", PARAM_NAME_S, name_s);
				}

			json_decref (hint_p);
		}

	return false;
}




bool GetValueFromJSON (const json_t * const root_p, const char *key_s, const ParameterType param_type, SharedType *value_p)
{
	bool success_flag = false;

	/* Get the parameter's value */
	json_t *json_value_p = json_object_get (root_p, key_s);

	if (json_value_p)
		{
			bool value_as_text_flag = false;

#if PARAMETER_DEBUG >= STM_LEVEL_FINE
			PrintJSONToLog (STM_LEVEL_FINE, __FILE__, __LINE__, json_value_p, key_s);
#endif

			/*
			 * Is the value set from a string? If so, we'll need to do
			 * a type conversion
			 */
			GetJSONBoolean (root_p, PARAM_VALUE_SET_FROM_TEXT_S, &value_as_text_flag);

			if (value_as_text_flag)
				{
					if (json_is_string (json_value_p))
						{
							const char *value_s = json_string_value (json_value_p);

							if (SetSharedTypeFromString (value_p, param_type, value_s))
								{
									success_flag = true;
								}

						}

				}		/* if (value_as_text_flag) */
			else
				{
					switch (param_type)
						{
							case PT_BOOLEAN:
								if (json_is_boolean (json_value_p))
									{
										value_p -> st_boolean_value = (json_is_true (json_value_p)) ? true : false;
										success_flag = true;
									}
								break;

							case PT_CHAR:
								if (json_is_string (json_value_p))
									{
										const char *value_s = json_string_value (json_value_p);

										if (value_s && (strlen (value_s) == 1))
											{
												value_p -> st_char_value = *value_s;
												success_flag = true;
											}
									}
								break;

							case PT_SIGNED_INT:
							case PT_NEGATIVE_INT:
								if (json_is_integer (json_value_p))
									{
										value_p -> st_long_value = (int32) json_integer_value (json_value_p);
										success_flag = true;
									}
								break;

							case PT_UNSIGNED_INT:
								if (json_is_integer (json_value_p))
									{
										value_p -> st_ulong_value = (uint32) json_integer_value (json_value_p);
										success_flag = true;
									}
								break;

							case PT_SIGNED_REAL:
							case PT_UNSIGNED_REAL:
								if (json_is_real (json_value_p))
									{
										value_p -> st_data_value = (double64) json_real_value (json_value_p);
										success_flag = true;
									}
								else if (json_is_integer (json_value_p))
									{
										value_p -> st_data_value = (double64) json_integer_value (json_value_p);
										success_flag = true;
									}
								break;


							case PT_DIRECTORY:
							case PT_FILE_TO_READ:
							case PT_FILE_TO_WRITE:
								{
									json_t *protocol_p = json_object_get (json_value_p, RESOURCE_PROTOCOL_S);

									if ((protocol_p) && (json_is_string (protocol_p)))
										{
											json_t *res_value_p = json_object_get (json_value_p, RESOURCE_VALUE_S);
											if (res_value_p && json_is_string (res_value_p))
												{
													const char *protocol_s = json_string_value (protocol_p);
													const char *value_s = json_string_value (res_value_p);

													value_p -> st_resource_value_p = AllocateResource (protocol_s, value_s, NULL);

													success_flag = (value_p -> st_resource_value_p != NULL);
												}
										}
								}
								break;

							case PT_TABLE:
							case PT_STRING:
							case PT_LARGE_STRING:
							case PT_PASSWORD:
							case PT_KEYWORD:
							case PT_FASTA:
								if (json_is_string (json_value_p))
									{
										const char * const src_s = json_string_value (json_value_p);

										if (strlen (src_s) > 0)
											{
												char *value_s = EasyCopyToNewString (src_s);

												if (value_s)
													{
														if (value_p -> st_string_value_s)
															{
																FreeCopiedString (value_p -> st_string_value_s);
															}

														value_p -> st_string_value_s = value_s;
														success_flag = true;
													}
											}
										else
											{
												if (value_p -> st_string_value_s)
													{
														FreeCopiedString (value_p -> st_string_value_s);
														value_p -> st_string_value_s = NULL;
													}

												success_flag = true;
											}
									}
								break;

							case PT_JSON:
							case PT_JSON_TABLE:
								{
									if (json_value_p)
										{
											json_t *copied_value_p = json_deep_copy (json_value_p);

											if (copied_value_p)
												{
													if (value_p -> st_json_p)
														{
															WipeJSON (value_p -> st_json_p);
														}

													value_p -> st_json_p = copied_value_p;
													success_flag = true;
												}
										}
									else
										{
											if (value_p -> st_json_p)
												{
													WipeJSON (value_p -> st_json_p);
													value_p -> st_json_p = NULL;
												}
										}
								}
								break;

							case PT_TIME:
								if (json_is_string (json_value_p))
									{
										const char * const time_s = json_string_value (json_value_p);

										if (value_p -> st_time_p)
											{
												success_flag = SetTimeFromString (value_p -> st_time_p, time_s);
											}
										else
											{
												value_p -> st_time_p = GetTimeFromString (time_s);
												success_flag = (value_p -> st_time_p != NULL);
											}
									}

								break;

							case PT_NUM_TYPES:
								PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, root_p, "Parameter has invalid type for key \"%s\"", key_s);
								break;

						}

				}

		}		/* if (json_value_p) */
	else
		{
			/* json params are allowed to be NULL */
			if ((param_type == PT_JSON) || (param_type == PT_JSON_TABLE))
				{
					success_flag = true;
				}
		}

	return success_flag;
}




static bool InitParameterStoreFromJSON (const json_t *root_p, HashTable *store_p)
{
	bool success_flag = true;
	json_t *store_json_p = json_object_get (root_p, PARAM_STORE_S);

	if (store_json_p)
		{
			const char *key_s;
			const json_t *value_p;

			json_object_foreach (store_json_p, key_s, value_p)
			{
				char *value_s = NULL;
				bool alloc_flag = false;

				if (json_is_string (value_p))
					{
						value_s = (char *) json_string_value (value_p);
					}
				else
					{
						value_s = json_dumps (value_p, JSON_INDENT (2));

						if (value_s)
							{
								alloc_flag = true;
							}
						else
							{
								PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, value_p, "Failed to decode \"%s\" to parameter store\n");
								success_flag = false;
							}
					}

				if (success_flag)
					{
						if (!PutInHashTable (store_p, key_s, value_s))
							{
								PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add \"%s\"=\"%s\" to parameter store\n", key_s, value_s);
								success_flag = false;
							}
					}

			}		/* json_object_foreach (store_json_p, key_s, value_p) */

		}		/* if (store_json_p) */

	return success_flag;
}


static bool GetParameterLevelFromJSON (const json_t * const json_p, ParameterLevel *level_p)
{
	bool success_flag = false;
	const char *level_s = GetJSONString (json_p, PARAM_LEVEL_S);

	if (level_s)
		{
			if (strcmp (level_s, PARAM_LEVEL_TEXT_ALL_S) == 0)
				{
					*level_p = PL_ALL;
				}
			else if (strcmp (level_s, PARAM_LEVEL_TEXT_SIMPLE_S) == 0)
				{
					*level_p = PL_SIMPLE;
				}
			else if (strcmp (level_s, PARAM_LEVEL_TEXT_ADVANCED_S) == 0)
				{
					*level_p = PL_ADVANCED;
				}

			success_flag = true;
		}


	return success_flag;
}


static bool GetParameterTypeFromJSON (const json_t * const json_p, ParameterType *param_type_p)
{
	bool success_flag = false;

	const char *value_s = GetJSONString (json_p, PARAM_GRASSROOTS_TYPE_INFO_TEXT_S);

	if (value_s)
		{
			if (GetGrassrootsTypeFromString (value_s, param_type_p))
				{
					success_flag = true;
				}
		}		/* if (value_s) */
	else
		{
			/* Backward compatibility */

			if (GetParameterTypeFromCompoundObject (json_p, param_type_p))
				{
					success_flag = true;
				}
			else
				{
					success_flag = GetParameterTypeFromSeparateObjects (json_p, param_type_p);
				}
		}

	return success_flag;
}




bool IsJSONParameterConcise (const json_t * const json_p)
{
	bool concise_flag = false;
	json_t *value_p = json_object_get (json_p, PARAM_CONCISE_DEFINITION_S);

	concise_flag = (value_p && json_is_true (value_p));

	return concise_flag;
}


const char *GetUIName (const Parameter * const parameter_p)
{
	if (parameter_p -> pa_display_name_s)
		{
			return (parameter_p -> pa_display_name_s);			
		}
	else
		{
			return (parameter_p -> pa_name_s);
		}

}


void SetParameterCallbacks (Parameter *param_p, void (*clear_fn) (Parameter *param_p),
														bool (*add_values_to_json_fn) (const Parameter *param_p, json_t *param_json_p, const bool full_definition_flag),
														bool (*get_values_from_json_fn) (Parameter *param_p, const json_t *param_json_p, const bool full_definition_flag),
														Parameter (*clone_fn) (const Parameter *param_p),
														bool (*set_value_from_string_fn) (struct Parameter *param_p, const char *value_s)
)
{
	param_p -> pa_clear_fn = clear_fn;
	param_p -> pa_clone_fn = clone_fn;
	param_p -> pa_add_values_to_json_fn = add_values_to_json_fn;
	param_p -> pa_get_values_from_json_fn = get_values_from_json_fn;
	param_p -> pa_set_value_from_string_fn = set_value_from_string_fn;
}


char *GetParameterValueAsString (const Parameter * const param_p, bool *alloc_flag_p)
{
	char *value_s = NULL;

	switch (param_p -> pa_type)
		{
			case PT_BOOLEAN:
				{
					BooleanParameter *boolean_param_p = (BooleanParameter *) param_p;
					const bool *value_p = GetBooleanParameterCurrentValue (boolean_param_p);

					if (value_p)
						{
							const char *src_s = (*value_p == true) ? "true" : "false";

							value_s = EasyCopyToNewString (src_s);
							*alloc_flag_p = true;
						}
				}
				break;

			case PT_CHAR:
				{
					CharParameter *char_param_p = (CharParameter *) param_p;
					const char *value_p = GetCharParameterCurrentValue (char_param_p);

					if (value_p)
						{
							char buffer_s [2];

							*buffer_s = *value_p;
							* (buffer_s + 1) = '\0';

							value_s = EasyCopyToNewString (buffer_s);
							*alloc_flag_p = true;
						}
				}
				break;

			case PT_SIGNED_INT:
			case PT_NEGATIVE_INT:
				{
					SignedIntParameter *int_param_p = (SignedIntParameter *) param_p;
					const int32 *value_p = GetSignedIntParameterCurrentValue (int_param_p);

					if (value_p)
						{
							value_s = ConvertIntegerToString (*value_p);
							*alloc_flag_p = true;
						}
				}
				break;

			case PT_UNSIGNED_INT:
				{
					UnsignedIntParameter *int_param_p = (UnsignedIntParameter *) param_p;
					const uint32 *value_p = GetUnsignedIntParameterCurrentValue (int_param_p);

					if (value_p)
						{
							value_s = ConvertUnsignedIntegerToString (*value_p);
							*alloc_flag_p = true;
						}
				}
				break;

			case PT_SIGNED_REAL:
			case PT_UNSIGNED_REAL:
				{
					DoubleParameter *double_param_p = (DoubleParameter *) param_p;
					const double64 *value_p = GetDoubleParameterCurrentValue (double_param_p);

					if (value_p)
						{
							value_s = ConvertDoubleToString (*value_p);
							*alloc_flag_p = true;
						}
				}
				break;

			case PT_DIRECTORY:
			case PT_FILE_TO_READ:
			case PT_FILE_TO_WRITE:
				{
					ResourceParameter *resource_param_p = (ResourceParameter *) param_p;
					const Resource *resource_p = GetResourceParameterCurrentValue (resource_param_p);

					if (resource_p)
						{
							json_t *resource_json_p = GetResourceAsJSON (resource_p);

							if (resource_json_p)
								{
									char *resource_s = json_dumps (resource_json_p, JSON_INDENT (2));

									if (resource_s)
										{
											value_s = EasyCopyToNewString (resource_s);
											*alloc_flag_p = true;

											free (resource_s);
										}

									json_decref (resource_json_p);
								}		/* if (resource_json_p) */
						}
				}

				break;

			case PT_TABLE:
			case PT_LARGE_STRING:
			case PT_STRING:
			case PT_PASSWORD:
			case PT_KEYWORD:
			case PT_FASTA:
				{
					StringParameter *string_param_p = (StringParameter *) param_p;
					const char *string_value_s = GetStringParameterCurrentValue (string_param_p);

					if (string_value_s)
						{
							value_s = EasyCopyToNewString (string_value_s);
							*alloc_flag_p = true;
						}
				}
				break;

			case PT_JSON:
			case PT_JSON_TABLE:
				{
					JSONParameter *json_param_p = (JSONParameter *) param_p;
					const json_t *value_p = GetJSONParameterCurrentValue (json_param_p);

					if (value_p)
						{
							char *dump_s = json_dumps (value_p, 0);

							if (dump_s)
								{
									value_s = EasyCopyToNewString (dump_s);
									*alloc_flag_p = true;

									free (dump_s);
								}
						}
				}
				break;


			case PT_TIME:
				{
					TimeParameter *time_param_p = (TimeParameter *) param_p;
					const struct tm *value_p = GetTimeParameterCurrentValue (time_param_p);

					if (value_p)
						{
							value_s = GetTimeAsString (value_p , true);
							*alloc_flag_p = true;
						}
				}
				break;

			case PT_NUM_TYPES:
				PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Parameter \"%s\" has invalid type", param_p -> pa_name_s);
				break;
		}		/* switch (param_p -> pa_type) */


	return value_s;
}



bool SetParameterValueFromString (Parameter * const param_p, const char *value_s)
{
	bool success_flag = param_p -> pa_set_value_from_string_fn (param_p, value_s);

	if (!success_flag)
		{
			PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to set parameter \"%s\" with type " UINT32_FMT " to \"%s\"", param_p -> pa_name_s, param_p -> pa_type, value_s);
		}

	return success_flag;
}



bool SetSharedTypeFromString (SharedType * const value_p, const ParameterType pt, const char *value_s)
{
	bool success_flag = false;

	switch (pt)
		{
			case PT_DIRECTORY:
			case PT_FILE_TO_READ:
			case PT_FILE_TO_WRITE:
				{
					Resource *resource_p = ParseStringToResource (value_s);

					if (resource_p)
						{
							success_flag = SetSharedTypeResourceValue (value_p, resource_p);
							FreeResource (resource_p);
						}
				}
				break;

			case PT_TABLE:
			case PT_LARGE_STRING:
			case PT_STRING:
			case PT_PASSWORD:
			case PT_KEYWORD:
			case PT_FASTA:
				success_flag = SetSharedTypeStringValue (value_p, value_s);
				break;


			case PT_CHAR:
				success_flag = SetSharedTypeCharValue (value_p, *value_s);
				break;

			case PT_JSON:
			case PT_JSON_TABLE:
				break;

			case PT_TIME:
				break;

			case PT_NUM_TYPES:
				break;
			//default:
			//	break;
		}		/* switch (param_p -> pa_type) */


	return success_flag;
}

bool AddRemoteDetailsToParameter (Parameter *param_p, const char * const uri_s, const char * const name_s)
{
	bool success_flag = false;
	RemoteParameterDetailsNode *node_p = AllocateRemoteParameterDetailsNodeByParts (uri_s, name_s);

	if (node_p)
		{
			LinkedListAddTail (param_p -> pa_remote_parameter_details_p, (ListItem *) node_p);
			success_flag = true;
		}

	return success_flag;
}



/*************************************************/
/************** STATIC FUNCTIONS *****************/
/*************************************************/


static bool AddParameterGroupToJSON (const Parameter * const param_p, json_t *json_p, const SchemaVersion * const sv_p)
{
	bool success_flag = true;

	if (param_p -> pa_group_p)
		{
			const char *group_name_s = param_p -> pa_group_p -> pg_name_s;

			if (group_name_s)
				{
					if (json_object_set_new (json_p, PARAM_GROUP_S, json_string (group_name_s)) != 0)
						{
							success_flag = false;
							PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to set group name to \"%s\" for param \"%s\"", group_name_s, param_p -> pa_name_s);
						}
				}

		}		/* if (param_p -> pa_group_p) */

	return success_flag;
}





static bool SetRemoteParameterDetailsFromJSON (Parameter *param_p, const json_t * json_p)
{
	bool success_flag = true;
	const json_t *remote_p = json_object_get (json_p, PARAM_REMOTE_S);

	if (remote_p)
		{
			if (json_is_array (remote_p))
				{

				}
		}

	return success_flag;
}



bool GetParameterGroupVisibility (const ServiceData *service_data_p, const char *group_name_s, bool *visibility_p)
{
	bool found_flag = false;

	if (service_data_p -> sd_config_p)
		{
			const json_t *groups_config_p = json_object_get (service_data_p -> sd_config_p, PARAM_SET_GROUPS_S);

			if (groups_config_p)
				{
					const json_t *group_config_p = json_object_get (groups_config_p, group_name_s);

					if (group_config_p)
						{
							found_flag = GetJSONBoolean (group_config_p, PARAM_GROUP_VISIBLE_S, visibility_p);
						}
				}
		}

	return found_flag;
}




bool GetParameterLevelFromConfig (const ServiceData *service_data_p, const char *param_name_s, ParameterLevel *level_p)
{
	bool found_flag = false;

	const json_t *param_config_p = GetParameterFromConfig (service_data_p -> sd_config_p, param_name_s);

	if (param_config_p)
		{
			found_flag = GetParameterLevelFromJSON (param_config_p, level_p);
		}

	return found_flag;
}



bool GetParameterDescriptionFromConfig (const ServiceData *service_data_p, const char *param_name_s, char **description_ss)
{
	return GetParameterStringFromConfig (service_data_p -> sd_config_p, param_name_s, PARAM_DESCRIPTION_S, description_ss);
}


bool GetParameterDisplayNameFromConfig (const ServiceData *service_data_p, const char *param_name_s, char **display_name_ss)
{
	return GetParameterStringFromConfig (service_data_p -> sd_config_p, param_name_s, PARAM_DISPLAY_NAME_S, display_name_ss);
}




static const json_t *GetParameterFromConfig (const json_t *service_config_p, const char * const param_name_s)
{
	if (service_config_p)
		{
			const json_t *params_config_p = json_object_get (service_config_p, PARAM_SET_PARAMS_S);

			if (params_config_p)
				{
					const json_t *param_config_p = json_object_get (params_config_p, param_name_s);

					if (param_config_p)
						{
							return param_config_p;
						}
				}
		}

	return NULL;
}


static bool GetParameterStringFromConfig (const json_t *service_config_p, const char * const param_name_s, const char * const key_s, char **value_ss)
{
	bool found_flag = false;

	const json_t *param_config_p = GetParameterFromConfig (service_config_p, param_name_s);

	if (param_config_p)
		{
			const char *value_s = GetJSONString (param_config_p, key_s);

			if (value_s)
				{
					char *copied_value_s = EasyCopyToNewString (value_s);

					if (copied_value_s)
						{
							if (*value_ss)
								{
									FreeCopiedString (*value_ss);
								}

							*value_ss = copied_value_s;

							found_flag = true;
						}		/* if (copied_value_s) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to make copy of \"%s\" for param \"%s\":\"%s\"", value_s, param_name_s, key_s);
						}
				}
		}

	return found_flag;
}
