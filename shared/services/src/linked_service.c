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
/*
 * linked_service.c
 *
 *  Created on: 8 Sep 2016
 *      Author: tyrrells
 */

#include <string.h>

#include "../../../shared/services/include/linked_service.h"
#include "memory_allocations.h"
#include "streams.h"

#include "json_util.h"
#include "string_utils.h"
#include "../../../shared/services/include/service.h"
#include "schema_version.h"
#include "user_details.h"
#include "../../../shared/services/include/service_job.h"
//#include "resource.h"


#ifdef _DEBUG
	#define LINKED_SERVICE_DEBUG	(STM_LEVEL_FINEST)
#else
	#define LINKED_SERVICE_DEBUG	(STM_LEVEL_NONE)
#endif



static bool CopyInputData (const char *input_service_s, char **copied_input_service_ss, const char *input_key_s, char **input_key_ss);

static bool AddMappedParameterToLinkedServiceList (LinkedList *params_p, MappedParameter *mapped_param_p);


LinkedService *AllocateLinkedService (const char *linked_service_s, const char *input_key_s, const json_t *mapped_params_json_p, const char * const function_s, const json_t *config_p, GrassrootsServer *grassroots_p)
{
	char *linked_service_copy_s = NULL;
	char *input_key_copy_s = NULL;

	if (CopyInputData (linked_service_s, &linked_service_copy_s, input_key_s, &input_key_copy_s))
		{
			LinkedList *list_p = AllocateLinkedList (FreeMappedParameterNode);

			if (list_p)
				{
					bool continue_flag = true;
					const size_t size = json_array_size (mapped_params_json_p);
					size_t i;

					for (i = 0; i < size; ++ i)
						{
							const json_t *mapped_param_json_p = json_array_get (mapped_params_json_p, i);
							MappedParameter *mapped_param_p = CreateMappedParameterFromJSON (mapped_param_json_p);

							if (mapped_param_p)
								{
									if (!AddMappedParameterToLinkedServiceList (list_p, mapped_param_p))
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add mapped param \"%s\":\"%s\" for \"%s\"", mapped_param_p -> mp_input_param_s, mapped_param_p -> mp_output_param_s, linked_service_s);
											i = size; 		/* force exit from loop */
											continue_flag = false;
										}		/* if (!AddMappedParameterToLinkedService (linked_service_p, mapped_param_p)) */

								}		/* if (mapped_param_p) */
							else
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, mapped_param_json_p, "Failed to create mapped parameter");
									i = size; 		/* force exit from loop */
									continue_flag = false;
								}

						}		/* for (i = 0; i < size; ++ i) */


					if (continue_flag)
						{
							LinkedService *linked_service_p = (LinkedService *) AllocMemory (sizeof (LinkedService));

							if (linked_service_p)
								{
									linked_service_p -> ls_output_service_s = linked_service_copy_s;
									linked_service_p -> ls_input_key_s = input_key_copy_s;
									linked_service_p -> ls_mapped_params_p = list_p;
									linked_service_p -> ls_generate_fn_s = function_s;
									linked_service_p -> ls_config_p = config_p;
									linked_service_p -> ls_grassroots_p = grassroots_p;

									return linked_service_p;
								}
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate LinkedService for \"%s\"", linked_service_s);
								}
						}

					FreeLinkedList (list_p);
				}		/* if (list_p) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate list of mapped params for \"%s\"", linked_service_s);
				}

			FreeCopiedString (linked_service_copy_s);

			if (input_key_copy_s)
				{
					FreeCopiedString (input_key_copy_s);
				}

		}		/* if (CopyInputData (input_service_s, &input_service_copy_s, input_key_s, input_key_copy_s)) */

	return NULL;
}


void FreeLinkedService (LinkedService *linked_service_p)
{
	if (linked_service_p -> ls_output_service_s)
		{
			FreeCopiedString (linked_service_p -> ls_output_service_s);
		}

	if (linked_service_p -> ls_mapped_params_p)
		{
			FreeLinkedList (linked_service_p -> ls_mapped_params_p);
		}

	if (linked_service_p -> ls_input_key_s)
		{
			FreeCopiedString (linked_service_p -> ls_input_key_s);
		}

	FreeMemory (linked_service_p);
}


LinkedServiceNode *AllocateLinkedServiceNode (LinkedService *linked_service_p)
{
	LinkedServiceNode *node_p = (LinkedServiceNode *) AllocMemory (sizeof (LinkedServiceNode));

	if (node_p)
		{
			node_p -> lsn_node.ln_prev_p = NULL;
			node_p -> lsn_node.ln_next_p = NULL;

			node_p -> lsn_linked_service_p = linked_service_p;
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate LinkedServiceNode for \"%s\"", linked_service_p -> ls_output_service_s);
		}

	return node_p;
}



void FreeLinkedServiceNode (ListItem *node_p)
{
	LinkedServiceNode *ls_node_p = (LinkedServiceNode *) node_p;

	FreeLinkedService (ls_node_p -> lsn_linked_service_p);
	FreeMemory (node_p);
}



bool AddLinkedServiceToRequestJSON (json_t *request_p, LinkedService *linked_service_p, ParameterSet *output_params_p)
{
	bool success_flag = false;
	json_t *linked_services_json_p = json_object_get (request_p, LINKED_SERVICES_S);

	if (!linked_services_json_p)
		{
			linked_services_json_p = json_object ();

			if (linked_services_json_p)
				{
					if (json_object_set_new (request_p, LINKED_SERVICES_S, linked_services_json_p) != 0)
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, request_p, "Failed to add empty linked services array for \"%s\"", LINKED_SERVICES_S);
							json_decref (linked_services_json_p);
							linked_services_json_p = NULL;
						}
				}

		}

	if (linked_services_json_p)
		{
			json_t *services_p = json_object_get (linked_services_json_p, SERVICES_NAME_S);

			if (!services_p)
				{
					services_p = json_array ();

					if (services_p)
						{
							if (json_object_set_new (linked_services_json_p, SERVICES_NAME_S, services_p) != 0)
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, request_p, "Failed to add empty service object for linked services array for \"%s\"", LINKED_SERVICES_S);
									json_decref (services_p);
									services_p = NULL;
								}
						}
				}

			if (services_p)
				{
					Service *service_p = GetServiceByName (linked_service_p -> ls_grassroots_p, linked_service_p -> ls_output_service_s, NULL);

					if (service_p)
						{
							json_t *run_service_p = GetInterestedServiceJSON (service_p, NULL, output_params_p, false);

							if (run_service_p)
								{
									const bool sync_flag = (service_p -> se_synchronous) == SY_SYNCHRONOUS ? true : false;

									if (SetJSONBoolean (run_service_p, OPERATION_SYNCHRONOUS_S, sync_flag))
										{
											if (json_array_append_new (services_p, run_service_p) == 0)
												{
													success_flag = true;
												}
											else
												{
													success_flag = false;
													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, run_service_p, "Failed to append linked service");
												}
										}
									else
										{
											success_flag = false;
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, run_service_p, "Failed to append \"%s\": true", OPERATION_SYNCHRONOUS_S);
										}

									if (!success_flag)
										{
											json_decref (run_service_p);
										}
								}

							FreeService (service_p);
						}		/* if (service_p) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to find Service with name \"%s\"", linked_service_p -> ls_output_service_s);
						}

				}		/* if (services_p) */

		}		/* if (linked_services_json_p) */

	return success_flag;
}


MappedParameter *GetMappedParameterByInputParamName (const LinkedService *linked_service_p, const char * const name_s)
{
	MappedParameterNode *param_node_p = (MappedParameterNode *) (linked_service_p -> ls_mapped_params_p -> ll_head_p);

	while (param_node_p)
		{
			MappedParameter *mapped_param_p = param_node_p -> mpn_mapped_param_p;

			if (strcmp (mapped_param_p -> mp_input_param_s, name_s) == 0)
				{
					return mapped_param_p;
				}

			param_node_p = (MappedParameterNode *) param_node_p -> mpn_node.ln_next_p;
		}

	return NULL;
}



bool ProcessLinkedService (LinkedService *linked_service_p, ServiceJob *job_p)
{
	bool success_flag = false;
	Service *service_p = GetServiceFromServiceJob (job_p);

	/* Does the input service have a custom process function? */
	if (service_p && (service_p -> se_process_linked_services_fn))
		{
			success_flag = service_p -> se_process_linked_services_fn (service_p, job_p, linked_service_p);
		}

	return success_flag;
}


LinkedService *CreateLinkedServiceFromJSON (Service *service_p, const json_t *linked_service_json_p, GrassrootsServer *grassroots_p)
{
	const char *linked_service_s = GetJSONString (linked_service_json_p, SERVICE_NAME_S);

	if (linked_service_s)
		{
			const char *function_s = GetJSONString (linked_service_json_p, LINKED_SERVICE_FUNCTION_S);
			const json_t *value_p = json_object_get (linked_service_json_p, PARAM_SET_PARAMS_S);
			const char *input_root_s = NULL;
			json_t *mapped_params_json_p = NULL;

			if (value_p)
				{
					if (json_is_object (value_p))
						{
							input_root_s = GetJSONString (value_p, MAPPED_PARAMS_ROOT_S);
							mapped_params_json_p = json_object_get (value_p, MAPPED_PARAMS_LIST_S);
						}		/* if (json_is_object (value_p)) */
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, value_p, "Fragment is not a JSON object");
						}
				}		/* if (value_p) */

			if (mapped_params_json_p || function_s)
				{
					LinkedService *linked_service_p = AllocateLinkedService (linked_service_s, input_root_s, mapped_params_json_p, function_s, json_object_get (linked_service_json_p, LINKED_SERVICE_CONFIG_S), grassroots_p);

					if (linked_service_p)
						{
							return linked_service_p;
						}
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, linked_service_json_p, "Failed to create Linked Service for %s", linked_service_s);
						}

				}		/* if (mapped_params_json_p || function_s) */

		}		/* if (linked_service_s) */
	else
		{
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, linked_service_json_p, "Failed to get %s", SERVICE_NAME_S);
		}

	return NULL;
}




bool CreateAndAddMappedParameterToLinkedService (LinkedService *linked_service_p, const char *input_s, const char *output_s, bool required_flag, bool multi_flag)
{
	MappedParameter *mapped_param_p = AllocateMappedParameter (input_s, output_s, required_flag, multi_flag);

	if (mapped_param_p)
		{
			if (AddMappedParameterToLinkedService (linked_service_p, mapped_param_p))
				{
					return true;
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add MappedParameter for \"%s\"=\"%s\" to \"%s\"", input_s, output_s, linked_service_p -> ls_output_service_s);
				}

			FreeMappedParameter (mapped_param_p);
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate MappedParameter for \"%s\"=\"%s\"", input_s, output_s);
		}

	return false;
}


bool AddMappedParameterToLinkedService (LinkedService *linked_service_p, MappedParameter *mapped_param_p)
{
	return AddMappedParameterToLinkedServiceList (linked_service_p -> ls_mapped_params_p, mapped_param_p);
}



json_t *GetLinkedServiceAsJSON (LinkedService *linked_service_p)
{
	json_t *res_p = NULL;
	json_error_t json_err;

	res_p = json_pack_ex (&json_err, 0, "{s:s,s:b}", JOB_SERVICE_S, linked_service_p -> ls_output_service_s, SERVICE_RUN_S, true);

	if (res_p)
		{
			const SchemaVersion *sv_p = GetSchemaVersion (linked_service_p -> ls_grassroots_p);
			Service *service_p = GetServiceByName (linked_service_p -> ls_grassroots_p, linked_service_p -> ls_output_service_s, NULL);

			if (service_p)
				{
					DataResource *resource_p = NULL;
					UserDetails *user_p = NULL;
					ParameterSet *params_p = GetServiceParameters (service_p, resource_p, user_p);

					if (params_p)
						{
							json_t *params_json_p = GetParameterSetAsJSON (params_p, sv_p, false);

							if (params_json_p)
								{
									if (!json_object_set_new (res_p, PARAM_SET_KEY_S, params_json_p) == 0)
										{
											json_decref (params_json_p);
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add parameters JSON to InterestedServiceJSON");
										}
								}
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "GetParameterSetAsJSON failed");
								}

							ReleaseServiceParameters (service_p, params_p);
						}		/* if (params_p) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "GetServiceParameters failed for \"%s\"", GetServiceName (service_p));
						}

					FreeService (service_p);
				}		/* if (service_p) */
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "GetInterestedServiceJSON failed, %s", json_err.text);
		}

	return res_p;
}



bool RunCustomLinkedServiceGenerator (struct LinkedService *linked_service_p, json_t *data_p, struct ServiceJob *job_p)
{
	bool success_flag = false;

	if (linked_service_p -> ls_generate_fn_s)
		{
			Service *calling_service_p = job_p -> sj_service_p;
			void *symbol_p = GetSymbolFromPlugin (calling_service_p -> se_plugin_p, linked_service_p -> ls_generate_fn_s);

			if (symbol_p)
				{
					bool (*generate_fn) (struct LinkedService *linked_service_p, json_t *data_p, struct ServiceJob *job_p) = (bool (*) (struct LinkedService *linked_service_p, json_t *data_p, struct ServiceJob *job_p)) symbol_p;

					success_flag = generate_fn (linked_service_p, data_p, job_p);
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to find function \"%s\" in service \"%s\"", linked_service_p -> ls_generate_fn_s, GetServiceName (calling_service_p));
				}
		}

	return success_flag;
}



static bool AddMappedParameterToLinkedServiceList (LinkedList *params_p, MappedParameter *mapped_param_p)
{
	bool success_flag = false;
	MappedParameterNode *node_p = AllocateMappedParameterNode (mapped_param_p);

	if (node_p)
		{
			LinkedListAddTail (params_p, (ListItem *) node_p);
			success_flag  = true;
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate memory of MappedParameterNode for \"%s\"=\"%s\"", mapped_param_p -> mp_input_param_s, mapped_param_p -> mp_output_param_s);
		}

	return success_flag;
}


static bool CopyInputData (const char *input_service_s, char **copied_input_service_ss, const char *input_key_s, char **input_key_ss)
{
	char *input_service_copy_s = EasyCopyToNewString (input_service_s);

	if (input_service_copy_s)
		{
			char *input_key_copy_s = NULL;

			if (CloneValidString (input_key_s, input_key_ss))
				{
					*copied_input_service_ss = input_service_copy_s;

					return true;
				}

			FreeCopiedString (input_service_copy_s);
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to copy input service name \"%s\"", input_service_s);
		}

	return false;
}


