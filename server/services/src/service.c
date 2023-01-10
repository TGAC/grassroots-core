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

/* Allocate the global constants in service.h */
#define ALLOCATE_PATH_TAGS (1)

#include <string.h>

#include "service.h"
#include "plugin.h"
#include "string_linked_list.h"
#include "string_utils.h"
#include "filesystem_utils.h"
#include "service_matcher.h"
#include "json_tools.h"
#include "json_util.h"
#include "streams.h"
#include "service_job.h"
#include "paired_service.h"
#include "linked_service.h"
#include "servers_manager.h"
#include "string_utils.h"
#include "service_job.h"
#include "provider.h"
#include "uuid_util.h"

#ifdef _DEBUG
#define SERVICE_DEBUG	(STM_LEVEL_INFO)
#else
#define SERVICE_DEBUG	(STM_LEVEL_NONE)
#endif


/*
 * STATIC FUNCTION DECLARATIONS
 */

static bool AddServiceNameToJSON (Service * const service_p, json_t *root_p);

static bool AddServiceDescriptionToJSON (Service * const service_p, json_t *root_p);

static bool AddServiceParameterSetToJSON (Service * const service_p, json_t *root_p, const SchemaVersion * const sv_p,  const bool full_definition_flag, Resource *resource_p, UserDetails *user_p);

static bool AddOperationInformationURIToJSON (Service * const service_p, json_t *root_p);

static bool AddServiceUUIDToJSON (Service * const service_p, json_t *root_p);

static uint32 GetMatchingServices (GrassrootsServer *grassroots_p, ServiceMatcher *matcher_p, UserDetails *user_p, LinkedList *services_list_p, bool multiple_match_flag);

static uint32 GetMatchingReferrableServices (GrassrootsServer *grassroots_p, ServiceMatcher *matcher_p, UserDetails *user_p, const json_t *reference_config_p, LinkedList *services_list_p, bool multiple_match_flag);

static uint32 FindMatchingServices (GrassrootsServer *grassroots_p, ServiceMatcher *matcher_p, UserDetails *user_p, const json_t *reference_config_p, LinkedList *services_list_p, bool multiple_match_flag, bool native_services_flag);

static const char *GetPluginNameFromJSON (const json_t * const root_p);

static uint32 AddMatchingServicesFromServicesArray (ServicesArray *services_p, LinkedList *matching_services_p, ServiceMatcher *matcher_p, bool multiple_match_flag);

static void GenerateServiceUUID (Service *service_p);

static json_t *GetServiceProcessRequest (const char * const service_name_s, const char *request_key_s, const bool run_flag, json_t *param_set_json_p, const SchemaVersion *sv_p, const ParameterLevel level);

static ServicesArray *GetServiceFromConfigJSON (const json_t *service_config_p, const char * const plugin_name_s, Service *(*get_service_fn) (json_t *config_p, size_t i, GrassrootsServer *grassroots_p), GrassrootsServer *grassroots_p);

static uint32 AddLinkedServices (Service *service_p, GrassrootsServer *grassroots_p);

static int CompareServicesByName (const void *v0_p, const void *v1_p);



/*
 * FUNCTION DEFINITIONS
 */

bool InitialiseService (Service * const service_p,
												const char *(*get_service_name_fn) (const Service *service_p),
												const char *(*get_service_description_fn) (const Service *service_p),
												const char *(*get_service_alias_fn) (const Service *service_p),
												const char *(*get_service_info_uri_fn) (const Service *service_p),
												ServiceJobSet *(*run_fn) (Service *service_p, ParameterSet *param_set_p, UserDetails *user_p, ProvidersStateTable *providers_p),
												ParameterSet *(*match_fn) (Service *service_p, Resource *resource_p, Handler *handler_p),
												ParameterSet *(*get_parameters_fn) (Service *service_p, Resource *resource_p, UserDetails *user_p),
												bool (*get_parameter_type_fn) (const struct Service *service_p, const char *param_name_s, ParameterType *pt_p),
												void (*release_parameters_fn) (Service *service_p, ParameterSet *params_p),
												bool (*close_fn) (struct Service *service_p),
												void (*customise_service_job_fn) (Service *service_p, ServiceJob *job_p),
												bool specific_flag,
												Synchronicity synchronous,
												ServiceData *data_p,
												ServiceMetadata *(*get_metadata_fn) (struct Service *service_p),
												json_t *(*get_indexing_data_fn) (struct Service *service_p),
												GrassrootsServer *grassroots_p)
{
	bool success_flag = false;

	if (get_metadata_fn)
		{
			service_p -> se_get_service_name_fn = get_service_name_fn;
			service_p -> se_get_service_description_fn = get_service_description_fn;
			service_p -> se_get_service_alias_fn = get_service_alias_fn;
			service_p -> se_get_service_info_uri_fn = get_service_info_uri_fn;
			service_p -> se_run_fn = run_fn;
			service_p -> se_match_fn = match_fn;
			service_p -> se_get_params_fn = get_parameters_fn;
			service_p -> se_get_parameter_type_fn = get_parameter_type_fn;
			service_p -> se_release_params_fn = release_parameters_fn;
			service_p -> se_close_fn = close_fn;
			service_p -> se_customise_service_job_fn = customise_service_job_fn;

			service_p -> se_data_p = data_p;

			service_p -> se_is_specific_service_flag = specific_flag;
			service_p -> se_synchronous = synchronous;


			service_p -> se_running_flag = false;

			uuid_clear (service_p -> se_id);

			service_p -> se_plugin_p = NULL;
			service_p -> se_has_permissions_fn = NULL;

			service_p -> se_deserialise_job_json_fn = NULL;
			service_p -> se_serialise_job_json_fn = NULL;
			service_p -> se_process_linked_services_fn = NULL;

			service_p -> se_jobs_p = NULL;

			service_p -> se_sync_data_p = NULL;

			service_p -> se_release_service_fn = NULL;

			service_p -> se_get_indexing_data_fn = get_indexing_data_fn;

			service_p -> se_custom_parameter_decoder_fn = NULL;

			InitLinkedList (& (service_p -> se_paired_services));
			SetLinkedListFreeNodeFunction (& (service_p -> se_paired_services), FreePairedServiceNode);

			if (service_p -> se_data_p)
				{
					const char *service_name_s = service_p -> se_get_service_name_fn (service_p);

					service_p -> se_data_p -> sd_service_p = service_p;

					if (service_name_s)
						{
							service_p -> se_data_p -> sd_config_p = GetGlobalServiceConfig (grassroots_p, service_name_s, & (service_p -> se_data_p -> sd_config_flag));
						}
				}


			/*
			 * Add the LinkedServices
			 */
			InitLinkedList (& (service_p -> se_linked_services));
			SetLinkedListFreeNodeFunction (& (service_p -> se_linked_services), FreeLinkedServiceNode);
			AddLinkedServices (service_p, grassroots_p);

			service_p -> se_get_metadata_fn = get_metadata_fn;
			service_p -> se_metadata_p = service_p -> se_get_metadata_fn (service_p);

			if (service_p -> se_metadata_p)
				{
					if (synchronous == SY_ASYNCHRONOUS_ATTACHED)
						{
							if ((service_p -> se_sync_data_p = AllocateSyncData ()) != NULL)
								{
									success_flag = true;
								}
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get SyncData for \"%s\"", GetServiceName (service_p));
								}

						}
					else
						{
							success_flag = true;
						}

				}		/* if (service_p -> se_metadata_p) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get metadata for \"%s\"", GetServiceName (service_p));
				}

		}		/* if (get_metadata_fn) */

	return success_flag;
}


void FreeService (Service *service_p)
{
#if SERVICE_DEBUG >= STM_LEVEL_FINEST
	PrintLog (STM_LEVEL_FINEST, __FILE__, __LINE__, "FreeService %s at %.16X", GetServiceName (service_p), service_p);
#endif

	Plugin *plugin_p = service_p -> se_plugin_p;

	if (service_p -> se_jobs_p)
		{
			FreeServiceJobSet (service_p -> se_jobs_p);
			service_p -> se_jobs_p = NULL;
		}

	ClearLinkedList (& (service_p -> se_paired_services));

	ClearLinkedList (& (service_p -> se_linked_services));

	if (service_p -> se_data_p)
		{
			ReleaseServiceData (service_p -> se_data_p);
		}

	CloseService (service_p);

	if (plugin_p)
		{
			DecrementPluginOpenCount (plugin_p);
		}

	if (service_p -> se_metadata_p)
		{
			FreeServiceMetadata (service_p -> se_metadata_p);
		}

	if (service_p -> se_sync_data_p)
		{
			FreeSyncData (service_p -> se_sync_data_p);
		}

	FreeMemory (service_p);
}


bool CloseService (Service *service_p)
{
#if SERVICE_DEBUG >= STM_LEVEL_FINEST
	PrintLog (STM_LEVEL_FINEST, __FILE__, __LINE__, "Closing %s at %.16X", GetServiceName (service_p), service_p);
#endif

	return service_p -> se_close_fn (service_p);
}


void ReleaseService (Service *service_p)
{
	if (service_p -> se_release_service_fn)
		{
			service_p -> se_release_service_fn (service_p);
		}
}



void SetServiceReleaseFunction (Service *service_p, void (*release_fn) (Service *service_p))
{
	service_p -> se_release_service_fn = release_fn;
}


void ReleaseServiceData (ServiceData *data_p)
{
	if ((data_p -> sd_config_flag) && (data_p -> sd_config_p))
		{
			json_decref (data_p -> sd_config_p);
		}
}


bool IsServiceLive (Service *service_p)
{
	bool is_live_flag = false;

	if (service_p -> se_jobs_p)
		{
			int32 num_jobs = GetNumberOfLiveJobs (service_p);

			is_live_flag = (num_jobs != 0);
		}

	return is_live_flag;
}


//json_t *GetServiceResults (Service *service_p, const uuid_t service_id)
//{
//	json_t *results_p = NULL;
//
//	if (service_p -> se_get_results_fn)
//		{
//			results_p = service_p -> se_get_results_fn (service_p, service_id);
//		}
//
//	return results_p;
//}


ServiceNode *AllocateServiceNode (Service *service_p)
{
	ServiceNode *node_p = (ServiceNode *) AllocMemory (sizeof (ServiceNode));

	if (node_p)
		{
			node_p -> sn_service_p = service_p;

			return node_p;
		}

	return NULL;
}


void FreeServiceNode (ListItem * const node_p)
{
	ServiceNode *service_node_p = (ServiceNode *) node_p;
	Service *service_p = service_node_p -> sn_service_p;

	if (service_p)
		{
			bool delete_flag = true;

			/* If the service is runnning in an async task or similar, keep it */
			if (service_p -> se_synchronous == SY_ASYNCHRONOUS_ATTACHED)
				{
					delete_flag = !IsServiceRunning (service_p);
				}

#if SERVICE_DEBUG >= STM_LEVEL_FINER
			PrintLog (STM_LEVEL_FINER, __FILE__, __LINE__, "Service \"%s\": delete flag = %d", GetServiceName (service_p), delete_flag ? 1 : 0);
#endif


			if (delete_flag)
				{
					FreeService (service_p);
				}
		}

	FreeMemory (service_node_p);
}




/**
 * Load any json stubs for external services that are used to configure generic services, 
 * e.g. web services
 */
void AddReferenceServices (GrassrootsServer *grassroots_p, LinkedList *services_p, const char * const references_path_s, const char * const services_path_s, const char *operation_name_s, UserDetails *user_p)
{
	const char *root_path_s = GetServerRootDirectory (grassroots_p);
	char *full_references_path_s = MakeFilename (root_path_s, references_path_s);

	if (full_references_path_s)
		{
			char *path_and_pattern_s = MakeFilename (full_references_path_s, "*.json");

			if (path_and_pattern_s)
				{
					LinkedList *matching_filenames_p = GetMatchingFiles (path_and_pattern_s, true);

					if (matching_filenames_p)
						{
							StringListNode *reference_file_node_p = (StringListNode *) (matching_filenames_p -> ll_head_p);
							PluginNameServiceMatcher *matcher_p = NULL;				

							if (operation_name_s)
								{
									matcher_p = (PluginNameServiceMatcher *) AllocatePluginOperationNameServiceMatcher (NULL, operation_name_s);
								}
							else
								{
									matcher_p = (PluginNameServiceMatcher *) AllocatePluginNameServiceMatcher (NULL);											
								}

							if (matcher_p)
								{

									while (reference_file_node_p)
										{
											json_t *reference_config_p = LoadJSONFile (reference_file_node_p -> sln_string_s);

											if (reference_config_p)
												{
													uint32 num_added_services = 0;
													char *json_s = json_dumps (reference_config_p, JSON_INDENT (2));

													json_t *services_json_p = json_object_get (reference_config_p, SERVICES_NAME_S);

													if (services_json_p)
														{
															const char * const plugin_name_s = GetPluginNameFromJSON (services_json_p);

															if (plugin_name_s)
																{
																	SetPluginNameForServiceMatcher (matcher_p, plugin_name_s);		

																	num_added_services = GetMatchingReferrableServices (grassroots_p, (ServiceMatcher *) matcher_p, user_p, services_json_p, services_p, true);
																}
															else
																{
																	PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to get service name from", reference_file_node_p -> sln_string_s);
																}

														}		/* if (services_json_p) */

#if SERVICE_DEBUG >= STM_LEVEL_FINEST
													PrintLog (STM_LEVEL_FINEST, __FILE__, __LINE__, "Added " UINT32_FMT " reference services for %s and refcount is %d", num_added_services, reference_file_node_p -> sln_string_s, config_p -> refcount);
#endif

													if (json_s)
														{
															free (json_s);
														}

													json_decref (reference_config_p);
												}		/* if (config_p) */
											else
												{
													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to load reference file %s", reference_file_node_p -> sln_string_s);
												}

											reference_file_node_p = (StringListNode *) (reference_file_node_p -> sln_node.ln_next_p);
										}		/* while (reference_file_node_p) */

									FreeServiceMatcher ((ServiceMatcher *) matcher_p);
								}
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate memory for references service matcher\n");
								}

							FreeLinkedList (matching_filenames_p);
						}		/* if (matching_filenames_p) */

					FreeCopiedString (path_and_pattern_s);
				}		/* if (path_and_pattern_s) */

			FreeCopiedString (full_references_path_s);
		}		/* if (full_services_path_s) */			
}




void SetServiceJobCustomFunctions (Service *service_p, ServiceJob *job_p)
{
	if (service_p -> se_customise_service_job_fn)
		{
			service_p -> se_customise_service_job_fn (service_p, job_p);
		}
	else
		{
			job_p -> sj_update_fn = NULL;
			job_p -> sj_free_fn = NULL;
		}
}


Service *GetServiceByName (GrassrootsServer *grassroots_p, const char * const service_name_s, const char * const service_alias_s)
{
	Service *service_p = NULL;
	LinkedList *services_p = AllocateLinkedList (FreeServiceNode);

	if (services_p)
		{
			LoadMatchingServicesByName (grassroots_p, services_p, SERVICES_PATH_S, service_name_s, service_alias_s, NULL);

			if (services_p -> ll_size == 1)
				{
					ServiceNode *service_node_p = (ServiceNode *) LinkedListRemHead (services_p);

					/* Detach service from node and free the node */
					service_p = service_node_p -> sn_service_p;
					service_node_p -> sn_service_p = NULL;

					FreeServiceNode ((ListItem *) service_node_p);
				}		/* if (services_p -> ll_size == 1) */

			FreeLinkedList (services_p);
		}		/* if (services_p) */

	return service_p;
}





static uint32 AddMatchingServicesFromServicesArray (ServicesArray *services_p, LinkedList *matching_services_p, ServiceMatcher *matcher_p, bool multiple_match_flag)
{
	Service **service_pp = services_p -> sa_services_pp;
	uint32 i = services_p -> sa_num_services;
	bool loop_flag = (i > 0);
	uint32 num_matched_services = 0;

	while (loop_flag)
		{
			if (*service_pp)
				{
					Service *service_p = *service_pp;
					GrassrootsServer *grassroots_p = GetGrassrootsServerFromService (service_p);
					const char *service_name_s = GetServiceName (service_p);

					if (IsServiceEnabled (grassroots_p, service_name_s))
						{
							bool using_service_flag = RunServiceMatcher (matcher_p, service_p);

							if (using_service_flag)
								{
									ServiceNode *service_node_p = AllocateServiceNode (service_p);

									if (service_node_p)
										{
											LinkedListAddTail (matching_services_p, (ListItem *) service_node_p);
											using_service_flag = true;

											++ num_matched_services;

											if (!multiple_match_flag)
												{
													loop_flag = false;
												}
										}
									else
										{
											/* failed to allocate service node */
										}
								}

							if (using_service_flag)
								{
									*service_pp = NULL;
								}

						}		/* if (!IsServiceDisabled (service_name_s)) */

				}		/* if (*service_pp) */

			if (loop_flag)
				{
					-- i;
					++ service_pp;

					loop_flag = (i > 0);
				}

		}		/* while (loop_flag) */

	return num_matched_services;
}


static uint32 GetMatchingReferrableServices (GrassrootsServer *grassroots_p, ServiceMatcher *matcher_p, UserDetails *user_p, const json_t *reference_config_p, LinkedList *services_list_p, bool multiple_match_flag)
{
	return FindMatchingServices (grassroots_p, matcher_p, user_p, reference_config_p, services_list_p, multiple_match_flag, false);
}


static uint32 GetMatchingServices (GrassrootsServer *grassroots_p, ServiceMatcher *matcher_p, UserDetails *user_p,  LinkedList *services_list_p, bool multiple_match_flag)
{
	return FindMatchingServices (grassroots_p, matcher_p, user_p, NULL, services_list_p, multiple_match_flag, true);
}


static uint32 FindMatchingServices (GrassrootsServer *grassroots_p, ServiceMatcher *matcher_p, UserDetails *user_p, const json_t *reference_config_p, LinkedList *services_list_p, bool multiple_match_flag, bool native_services_flag)
{
	uint32 num_matched_services = 0;
	const char *plugin_pattern_s = GetPluginPattern ();

	if (plugin_pattern_s)
		{
			const char *root_path_s = GetServerRootDirectory (grassroots_p);
			char *full_services_path_s = MakeFilename (root_path_s, "services");

			if (full_services_path_s)
				{
					char *path_and_pattern_s = MakeFilename (full_services_path_s, plugin_pattern_s);

					if (path_and_pattern_s)
						{
							LinkedList *matching_filenames_p = GetMatchingFiles (path_and_pattern_s, true);

							if (matching_filenames_p)
								{
									StringListNode *node_p = (StringListNode *) (matching_filenames_p -> ll_head_p);

									while (node_p)
										{
											Plugin *plugin_p = AllocatePlugin (node_p -> sln_string_s, grassroots_p);

											if (plugin_p)
												{
													bool using_plugin_flag = false;

													if (OpenPlugin (plugin_p))
														{																							
															ServicesArray *services_p = NULL;

															if (native_services_flag)
																{
																	services_p = GetServicesFromPlugin (plugin_p, user_p);
																}
															else
																{
																	services_p = GetReferrableServicesFromPlugin (plugin_p, user_p, reference_config_p);
																}

															if (services_p)
																{
																	uint32 i = AddMatchingServicesFromServicesArray (services_p, services_list_p, matcher_p, multiple_match_flag);

#if SERVICE_DEBUG >= STM_LEVEL_FINEST
																	PrintLog (STM_LEVEL_FINEST, __FILE__, __LINE__, "Got " UINT32_FMT " services from %s with open count of " INT32_FMT, i, node_p -> sln_string_s, plugin_p -> pl_open_count);
#endif

																	if (i > 0)
																		{
																			using_plugin_flag = true;
																			num_matched_services += i;
																		}

																	/*
																	 * If the open count is 0, then the plugin
																	 * should get freed by the call to FreeServicesArray
																	 */
																	if (i == 0)
																		{
																			plugin_p = NULL;
																		}

																	FreeServicesArray (services_p);
																}
															else
																{
																	/* failed to get service from plugin */
																}

														}		/* if (OpenPlugin (plugin_p)) */


													if (plugin_p && (!using_plugin_flag))
														{
															FreePlugin (plugin_p);
														}


												}		/* if (plugin_p) */

											if (node_p)
												{
													node_p = (StringListNode *) (node_p -> sln_node.ln_next_p);
												}
										}		/* while (node_p) */

									FreeLinkedList (matching_filenames_p);
								}		/* if (matching_filenames_p) */

							FreeCopiedString (path_and_pattern_s);
						}		/* if (path_and_pattern_s) */

					FreeCopiedString (full_services_path_s);
				}		/* if (full_services_path_s) */

		}		/* if (plugin_pattern_s) */

	return num_matched_services;
}


void LoadMatchingServicesByName (GrassrootsServer *grassroots_p, LinkedList *services_p, const char * const services_path_s, const char *service_name_s, const char *service_alias_s, UserDetails *user_p)
{
	NameServiceMatcher matcher;

	InitOperationNameServiceMatcher (&matcher, service_name_s, service_alias_s);

	/* Since we're after a service with a given name, we don't need multiple matches */
	GetMatchingServices (grassroots_p, & (matcher.nsm_base_matcher), user_p, services_p, false);

	if (services_p -> ll_size == 0)
		{
			AddReferenceServices (grassroots_p, services_p, grassroots_p -> gs_references_path_s, services_path_s, service_name_s, user_p);
		}
}



void LoadMatchingServices (GrassrootsServer *grassroots_p, LinkedList *services_p, const char * const services_path_s, Resource *resource_p, Handler *handler_p, UserDetails *user_p)
{
	ResourceServiceMatcher matcher;

	InitResourceServiceMatcher (&matcher, resource_p, handler_p);

	GetMatchingServices (grassroots_p, & (matcher.rsm_base_matcher), user_p, services_p, true);

	AddReferenceServices (grassroots_p, services_p, grassroots_p -> gs_references_path_s, services_path_s, NULL, user_p);
}


void LoadKeywordServices (GrassrootsServer *grassroots_p, LinkedList *services_p, const char * const services_path_s, UserDetails *user_p)
{
	KeywordServiceMatcher matcher;

	InitKeywordServiceMatcher (&matcher);

	GetMatchingServices (grassroots_p, & (matcher.ksm_base_matcher), user_p, services_p, true);

	AddReferenceServices (grassroots_p, services_p, grassroots_p -> gs_references_path_s, services_path_s, NULL, user_p);
}


ServiceJobSet *RunService (Service *service_p, ParameterSet *param_set_p, UserDetails *user_p, ProvidersStateTable *providers_p)
{
	ServiceJobSet *job_set_p = NULL;

	GenerateServiceUUID (service_p);

	job_set_p = service_p -> se_run_fn (service_p, param_set_p, user_p, providers_p);

	return job_set_p;
}


bool IsServiceRunning (Service *service_p)
{
	return service_p -> se_running_flag;
}


void SetServiceRunning (Service *service_p, bool b)
{
	service_p -> se_running_flag = b;
}


bool DoesServiceHaveCustomServiceJobSerialisation (struct Service *service_p)
{
	return ((service_p -> se_serialise_job_json_fn != NULL) || (service_p -> se_deserialise_job_json_fn != NULL));
}


ServiceJob *CreateSerialisedServiceJobFromService (struct Service *service_p, const json_t *service_job_json_p)
{
	ServiceJob *job_p = NULL;
	const char *service_name_s = GetServiceName (service_p);

	if (DoesServiceHaveCustomServiceJobSerialisation (service_p))
		{
			job_p = service_p -> se_deserialise_job_json_fn (service_p, service_job_json_p);

			if (!job_p)
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, service_job_json_p, "%s failed to deserialise job", service_name_s);
				}		/* if (!job_p) */

		}		/* if (DoesServiceHaveCustomServiceJobDeserialiser (service_p)) */
	else
		{
			PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "%s does not have a custom ServiceJob deserialiser", service_name_s);
		}

	return job_p;
}


json_t *CreateSerialisedJSONForServiceJobFromService (struct Service *service_p, struct ServiceJob *service_job_p, bool omit_results_flag)
{
	json_t *job_json_p = NULL;
	const char *service_name_s = GetServiceName (service_p);

	if (DoesServiceHaveCustomServiceJobSerialisation (service_p))
		{
			job_json_p = service_p -> se_serialise_job_json_fn (service_p, service_job_p, omit_results_flag);

			if (!job_json_p)
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "%s failed to serialise job", service_job_p -> sj_service_name_s);
				}		/* if (!job_json_p) */

		}		/* if (DoesServiceHaveCustomServiceJobDeserialiser (service_p)) */
	else
		{
			PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "%s does not have a custom ServiceJob serialiser", service_job_p -> sj_service_name_s);
		}

	return job_json_p;
}


ParameterSet *IsServiceMatch (Service *service_p, Resource *resource_p, Handler *handler_p)
{
	ParameterSet *params_p = NULL;

	if (service_p -> se_match_fn)
		{
			params_p = service_p -> se_match_fn (service_p, resource_p, handler_p);
		}

	return params_p;
}


/** Get the user-friendly name of the service. */
const char *GetServiceName (const Service *service_p)
{
	return service_p -> se_get_service_name_fn (service_p);
}


/** Get the user-friendly description of the service. */
const char *GetServiceDescription (const Service *service_p)
{
	return service_p -> se_get_service_description_fn (service_p);	
}


/** Get the user-friendly description of the service. */
const char *GetServiceAlias (const Service *service_p)
{
	const char *alias_s = NULL;

	if ((service_p -> se_data_p) && (service_p -> se_data_p -> sd_config_p))
		{
			alias_s = GetJSONString (service_p -> se_data_p -> sd_config_p, SERVICE_ALIAS_S);
		}

	if (!alias_s)
		{
			alias_s = service_p -> se_get_service_alias_fn (service_p);
		}

	return alias_s;
}



const char *GetServiceInformationURI (const Service *service_p)
{
	const char *uri_s = NULL;

	if (service_p -> se_get_service_info_uri_fn)
		{
			uri_s = service_p -> se_get_service_info_uri_fn (service_p);
		}

	return uri_s;
}


/** Get the indexing data for the Service. */
json_t *GetServiceIndexingData (Service *service_p)
{
	json_t *res_p = NULL;

	if (service_p -> se_get_indexing_data_fn)
		{
			res_p = service_p -> se_get_indexing_data_fn (service_p);
		}

	return res_p;
}


const uuid_t *GetServiceUUID (const Service *service_p)
{
	return & (service_p -> se_id);
}


char *GetServiceUUIDAsString (Service *service_p)
{
	char *uuid_s = NULL;
	const uuid_t *id_p = GetServiceUUID (service_p);

	if (uuid_is_null (*id_p) == 0)
		{
			uuid_s = GetUUIDAsString (*id_p);
		}

	return uuid_s;
}



ParameterSet *GetServiceParameters (Service *service_p, Resource *resource_p, UserDetails *user_p)
{
	return service_p -> se_get_params_fn (service_p, resource_p, user_p);
}


bool GetParameterTypeForNamedParameter (const struct Service *service_p, const char *param_name_s, ParameterType *pt_p)
{
	return service_p -> se_get_parameter_type_fn (service_p, param_name_s, pt_p);
}



void ReleaseServiceParameters (Service *service_p, ParameterSet *params_p)
{
	return service_p -> se_release_params_fn (service_p, params_p);
}


static const char *GetPluginNameFromJSON (const json_t * const root_p)
{
	return GetJSONString (root_p, PLUGIN_NAME_S);
}


static void GenerateServiceUUID (Service *service_p)
{
	uuid_generate (service_p -> se_id);
}


//
//	Get Symbol
//
ServicesArray *GetServicesFromPlugin (Plugin * const plugin_p, UserDetails *user_p)
{
	ServicesArray *services_p = NULL;

	if (plugin_p -> pl_type == PN_SERVICE)
		{
			services_p = plugin_p -> pl_value.pv_services_p;
		}
	else if (plugin_p -> pl_type == PN_UNKNOWN)
		{
			void *symbol_p = GetSymbolFromPlugin (plugin_p, "GetServices");

			if (symbol_p)
				{
					ServicesArray *(*fn_p) (UserDetails *, GrassrootsServer *) = (ServicesArray *(*) (UserDetails *, GrassrootsServer *)) symbol_p;

					services_p = fn_p (user_p, plugin_p -> pl_server_p);

					if (services_p)
						{
							AssignPluginForServicesArray (services_p, plugin_p);

							plugin_p -> pl_value.pv_services_p = services_p;
							plugin_p -> pl_type = PN_SERVICE;
						}
				}
		}

	return services_p;

}


ServicesArray *GetReferrableServicesFromPlugin (Plugin * const plugin_p, UserDetails *user_p, const json_t *service_config_p)
{
	ServicesArray *services_p = NULL;

	if (plugin_p -> pl_type == PN_SERVICE)
		{
			services_p = plugin_p -> pl_value.pv_services_p;
		}
	else if (plugin_p -> pl_type == PN_UNKNOWN)
		{
			void *symbol_p = GetSymbolFromPlugin (plugin_p, "GetReferenceServices");

			if (symbol_p)
				{
					ServicesArray *(*fn_p) (UserDetails *, const json_t *) = (ServicesArray *(*) (UserDetails *, const json_t *)) symbol_p;

					services_p = fn_p (user_p, service_config_p);

					if (services_p)
						{
							AssignPluginForServicesArray (services_p, plugin_p);

							plugin_p -> pl_value.pv_services_p = services_p;
							plugin_p -> pl_type = PN_SERVICE;
						}
				}
		}

	return services_p;
}


bool DeallocatePluginService (Plugin * const plugin_p)
{
	bool success_flag = false;

	if (plugin_p -> pl_type == PN_SERVICE)
		{
			if (plugin_p -> pl_value.pv_services_p)
				{
					void *symbol_p = GetSymbolFromPlugin (plugin_p, "ReleaseServices");

					if (symbol_p)
						{
							void (*fn_p) (ServicesArray * const) = (void (*) (ServicesArray * const)) symbol_p;

							fn_p (plugin_p -> pl_value.pv_services_p);

							ClearPluginValue (plugin_p);

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


json_t *GetServiceRefreshRequest (const char * const service_name_s, const ParameterSet *params_p, const SchemaVersion *sv_p, const bool run_flag, const ParameterLevel level)
{
	json_t *service_json_p = NULL;
	json_t *param_set_json_p = GetParameterSetAsJSON (params_p, sv_p, false);

	if (param_set_json_p)
		{
			service_json_p = GetServiceRefreshRequestFromJSON (service_name_s, param_set_json_p, sv_p, run_flag, level);
		}		/* if (param_set_json_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get ParameterSet as JSON");
		}

	return service_json_p;
}


json_t *GetServiceRunRequest (const char * const service_name_s, const ParameterSet *params_p, const SchemaVersion *sv_p, const bool run_flag, const ParameterLevel level)
{
	json_t *service_json_p = NULL;
	json_t *param_set_json_p = GetParameterSetAsJSON (params_p, sv_p, false);

	if (param_set_json_p)
		{
			service_json_p = GetServiceRunRequestFromJSON (service_name_s, param_set_json_p, sv_p, run_flag, level);
		}		/* if (param_set_json_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get ParameterSet as JSON");
		}

	return service_json_p;
}


json_t *GetServiceRefreshRequestFromJSON (const char * const service_name_s, json_t *param_set_json_p, const SchemaVersion *sv_p, const bool run_flag, const ParameterLevel level)
{
	return GetServiceProcessRequest (service_name_s, SERVICE_REFRESH_S, run_flag, param_set_json_p, sv_p, level);
}


json_t *GetServiceRunRequestFromJSON (const char * const service_name_s, json_t *param_set_json_p, const SchemaVersion *sv_p, const bool run_flag, const ParameterLevel level)
{
	return GetServiceProcessRequest (service_name_s, SERVICE_RUN_S, run_flag, param_set_json_p, sv_p, level);
}



static json_t *GetServiceProcessRequest (const char * const service_name_s, const char *request_key_s, const bool run_flag, json_t *param_set_json_p, const SchemaVersion *sv_p, const ParameterLevel level)
{
	json_t *service_json_p = NULL;
	json_error_t err;

	service_json_p = json_pack_ex (&err, 0, "{s:s,s:b}", SERVICE_NAME_S, service_name_s, request_key_s, run_flag);

	if (service_json_p)
		{
			if (run_flag)
				{
					if (json_object_set_new (service_json_p, PARAM_SET_KEY_S, param_set_json_p) == 0)
						{
							return service_json_p;
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add ParameterSet JSON");
							json_decref (param_set_json_p);
						}		/* if (json_object_set_new (service_json_p, PARAM_SET_KEY_S, param_set_json_p) != 0) */

				}		/* if (run_flag) */
			else
				{
					return service_json_p;
				}

			json_decref (service_json_p);
		}		/* if (service_json_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create service json, error at line %d, col %d, \"%s\"", err.line, err.column, err.text);
		}

	return NULL;
}


json_t *GetServiceAsJSON (Service * const service_p, Resource *resource_p, UserDetails *user_p, const bool add_id_flag)
{
	json_t *root_p = GetBaseServiceDataAsJSON (service_p, user_p);

	if (root_p)
		{
			bool success_flag = false;
			/* Add the operations for this service */
			json_t *operation_p = json_object ();

#if SERVICE_DEBUG >= STM_LEVEL_FINER
			PrintJSONToLog (STM_LEVEL_FINER, __FILE__, __LINE__, root_p, "GetServiceAsJSON - description :: ");
#endif

			if (operation_p)
				{
					GrassrootsServer *grassroots_p = GetGrassrootsServerFromService (service_p);
					const SchemaVersion *sv_p = GetSchemaVersion (grassroots_p);

					if (AddServiceParameterSetToJSON (service_p, operation_p, sv_p, true, resource_p, user_p))
						{
							if (json_object_set_new (operation_p, OPERATION_SYNCHRONOUS_S, (service_p -> se_synchronous == SY_SYNCHRONOUS) ? json_true () : json_false ()) == 0)
								{
									bool b = true;

									if (add_id_flag)
										{
											if (!IsUUIDSet (service_p -> se_id))
												{
													GenerateServiceUUID (service_p);
												}

											b = AddServiceUUIDToJSON (service_p, operation_p);
										}

									if (b)
										{
											const char *icon_uri_s = GetServiceIcon (service_p);

											if (icon_uri_s)
												{
													if (json_object_set_new (operation_p, OPERATION_ICON_URI_S, json_string (icon_uri_s)) != 0)
														{
															PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to add icon uri \"%s\" for service \"%s\"", icon_uri_s, GetServiceName (service_p));
														}
												}

											AddOperationInformationURIToJSON (service_p, operation_p);

											success_flag = true;
										}		/* if (b) */

								}		/* if (json_object_set_new (operation_p, OPERATION_SYNCHRONOUS_S, service_p -> se_synchronous_flag ? json_true () : json_false ()) == 0) */

						}		/* if (AddServiceParameterSetToJSON (service_p, operation_p, true, resource_p, json_p)) */

					if (success_flag)
						{
							if (json_object_set_new (root_p, SERVER_OPERATION_S, operation_p) == 0)
								{
									success_flag = true;
								}
						}
					else
						{
							json_object_clear (root_p);
							json_decref (root_p);
							root_p = NULL;
						}

					if (!success_flag)
						{
							json_decref (operation_p);
						}

				}		/* if (operation_p) */

			if (success_flag)
				{
					return root_p;
				}

			json_decref (root_p);
		}		/* if (root_p) */

	return NULL;
}


json_t *GetServiceIndexingDataAsJSON (Service * const service_p, Resource *resource_p, UserDetails *user_p)
{
	json_t *res_p = GetBaseServiceDataAsJSON (service_p, user_p);

	if (res_p)
		{
			json_t *service_data_p = GetServiceIndexingData (service_p);

			if (service_data_p)
				{
					if (json_object_set_new (res_p, "custom", service_data_p) != 0)
						{
							PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, service_data_p, "Failed to append custom indexing data for \"%s\"", GetServiceName (service_p));
							json_decref (service_data_p);
						}
				}

		}		/* if (res_p) */

	return res_p;
}



json_t *GetBaseServiceDataAsJSONOld (Service * const service_p, UserDetails *user_p)
{
	json_t *root_p = json_object ();

	if (root_p)
		{
			const char *service_name_s = GetServiceName (service_p);

			if (json_object_set_new (root_p, "@type", json_string ("grassroots_service")) == 0)
				{
					bool success_flag = true;

					/* Add the key-value pair */
					if (service_name_s)
						{
							success_flag = (json_object_set_new (root_p, SERVICE_NAME_S, json_string (service_name_s)) == 0);
						}

					if (success_flag)
						{
							GrassrootsServer *grassroots_p = GetGrassrootsServerFromService (service_p);
							const json_t *provider_p = GetProviderAsJSON (grassroots_p);

							if (provider_p)
								{
									json_t *copied_provider_p = json_deep_copy (provider_p);

									if (copied_provider_p)
										{
											if (SetProviderType (copied_provider_p))
												{
													/* Add any paired services details */
													if (service_p -> se_paired_services.ll_size > 0)
														{
															json_t *providers_array_p = json_array ();

															if (providers_array_p)
																{
																	if (json_array_append_new (providers_array_p, copied_provider_p) == 0)
																		{
																			ServersManager *servers_manager_p = GetServersManager (grassroots_p);
																			PairedServiceNode *node_p = (PairedServiceNode *) (service_p -> se_paired_services.ll_head_p);

																			while (node_p)
																				{
																					PairedService *paired_service_p = node_p -> psn_paired_service_p;
																					ExternalServer *external_server_p = GetExternalServerFromServersManager (servers_manager_p, paired_service_p -> ps_server_uri_s, NULL);

																					if (external_server_p)
																						{
																							json_t *external_provider_p = paired_service_p -> ps_provider_p;

																							if (external_provider_p)
																								{
																									if (json_array_append (providers_array_p, external_provider_p) != 0)
																										{
																											PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, external_provider_p, "Failed to append external provider to providers array");

																											json_decref (external_provider_p);
																										}		/* if (json_array_append_new (providers_array_p, external_provider_p) != 0) */

																								}		/* if (external_provider_p) */
																							else
																								{
																									PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to create provider for external server %s at %s array", external_server_p -> es_name_s, external_server_p -> es_uri_s);
																								}


																						}		/* if (external_server_p) */
																					else
																						{
																							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get external server for paired service at %s", paired_service_p -> ps_server_uri_s);
																						}

																					node_p = (PairedServiceNode *) (node_p -> psn_node.ln_next_p);
																				}		/* while (node_p) */


																			if (json_object_set_new (root_p, SERVER_MULTIPLE_PROVIDERS_S, providers_array_p) != 0)
																				{
																					PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, providers_array_p, "Failed to set providers array");

																					json_decref (providers_array_p);
																				}		/* if (json_object_set_new (root_p, SERVER_PROVIDER_S, providers_array_p) != 0) */

																		}		/* if (json_array_append_new (providers_array_p, copied_provider_p) == 0) */
																	else
																		{
																			PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, copied_provider_p, "Failed to append copied provider to providers array");

																			json_decref (copied_provider_p);
																			json_decref (providers_array_p);
																		}

																}		/* if (providers_array_p) */
															else
																{
																	PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to create providers array for %s", service_name_s);
																}

														}		/* if (service_p -> se_paired_services -> ll_size > 0) */
													else
														{
															if (json_object_set_new (root_p, SERVER_PROVIDER_S, copied_provider_p) != 0)
																{
																	PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, copied_provider_p, "Failed to set copied provider");

																	json_decref (copied_provider_p);
																	success_flag = false;
																}		/* if (json_object_set_new (root_p, SERVER_PROVIDER_S, copied_provider_p) != 0) */
														}

												}		/* if (SetProviderType (copied_provider_p)) */
											else
												{

												}


										}		/* if (copied_provider_p) */
									else
										{
											success_flag = false;
											PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, provider_p, "Failed to make copy of default provider");
										}
								}
						}


					if (success_flag)
						{
#if SERVICE_DEBUG >= STM_LEVEL_FINER
							PrintJSONToLog (STM_LEVEL_FINER, __FILE__, __LINE__, root_p, "GetServiceAsJSON - path :: ");
#endif

							const char *value_s = GetServiceDescription (service_p);

							if (value_s)
								{
									success_flag = (json_object_set_new (root_p, SERVICE_DESCRIPTION_S, json_string (value_s)) == 0);
								}


							if (success_flag && (service_p -> se_metadata_p))
								{
									success_flag = AddServiceMetadataToJSON (service_p -> se_metadata_p, root_p);
								}

						}

				}		/* if (json_object_set_new (root_p, "@type", json_string ("grassroots_service")) == 0) */
			else
				{
					PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to add \"@type\" =  \"grassroots_service\" to Service JSON for %s", service_name_s ? service_name_s : "unknown service");
				}


#if SERVICE_DEBUG >= STM_LEVEL_FINER
			PrintJSONToLog (STM_LEVEL_FINER, __FILE__, __LINE__, root_p, "GetServiceAsJSON - service :: ");
#endif

		}		/* if (root_p) */


	return root_p;
}


json_t *GetBaseServiceDataAsJSON (Service * const service_p, UserDetails *user_p)
{
	json_t *root_p = json_object ();

	if (root_p)
		{
			if (SetJSONString (root_p, "@type", "grassroots_service"))
				{
					const char *service_name_s = GetServiceName (service_p);

					if (SetJSONString (root_p, SERVICE_NAME_S, service_name_s))
						{
							const char *value_s = GetServiceDescription (service_p);

							if (SetJSONString (root_p, SERVICE_DESCRIPTION_S, value_s))
								{
									value_s = GetServiceAlias (service_p);

									if (SetJSONString (root_p, SERVICE_ALIAS_S, value_s))
										{
											GrassrootsServer *grassroots_p = GetGrassrootsServerFromService (service_p);
											const json_t *provider_p = GetProviderAsJSON (grassroots_p);

											if (provider_p)
												{
													json_t *copied_provider_p = json_deep_copy (provider_p);

													if (copied_provider_p)
														{
															if (SetProviderType (copied_provider_p))
																{
																	bool success_flag = true;

																	/* Add any paired services details */
																	if (service_p -> se_paired_services.ll_size > 0)
																		{
																			json_t *providers_array_p = json_array ();

																			if (providers_array_p)
																				{
																					if (json_array_append_new (providers_array_p, copied_provider_p) == 0)
																						{
																							ServersManager *servers_manager_p = GetServersManager (grassroots_p);
																							PairedServiceNode *node_p = (PairedServiceNode *) (service_p -> se_paired_services.ll_head_p);

																							while (node_p)
																								{
																									PairedService *paired_service_p = node_p -> psn_paired_service_p;
																									ExternalServer *external_server_p = GetExternalServerFromServersManager (servers_manager_p, paired_service_p -> ps_server_uri_s, NULL);

																									if (external_server_p)
																										{
																											json_t *external_provider_p = paired_service_p -> ps_provider_p;

																											if (external_provider_p)
																												{
																													if (json_array_append (providers_array_p, external_provider_p) != 0)
																														{
																															PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, external_provider_p, "Failed to append external provider to providers array");

																															json_decref (external_provider_p);
																														}		/* if (json_array_append_new (providers_array_p, external_provider_p) != 0) */

																												}		/* if (external_provider_p) */
																											else
																												{
																													PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to create provider for external server %s at %s array", external_server_p -> es_name_s, external_server_p -> es_uri_s);
																												}


																										}		/* if (external_server_p) */
																									else
																										{
																											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get external server for paired service at %s", paired_service_p -> ps_server_uri_s);
																										}

																									node_p = (PairedServiceNode *) (node_p -> psn_node.ln_next_p);
																								}		/* while (node_p) */


																							if (json_object_set_new (root_p, SERVER_MULTIPLE_PROVIDERS_S, providers_array_p) != 0)
																								{
																									PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, providers_array_p, "Failed to set providers array");

																									json_decref (providers_array_p);
																								}		/* if (json_object_set_new (root_p, SERVER_PROVIDER_S, providers_array_p) != 0) */

																						}		/* if (json_array_append_new (providers_array_p, copied_provider_p) == 0) */
																					else
																						{
																							PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, copied_provider_p, "Failed to append copied provider to providers array");

																							json_decref (copied_provider_p);
																							json_decref (providers_array_p);
																						}

																				}		/* if (providers_array_p) */
																			else
																				{
																					PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to create providers array for %s", service_name_s);
																				}

																		}		/* if (service_p -> se_paired_services -> ll_size > 0) */
																	else
																		{
																			if (json_object_set_new (root_p, SERVER_PROVIDER_S, copied_provider_p) != 0)
																				{
																					PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, copied_provider_p, "Failed to set copied provider");

																					json_decref (copied_provider_p);
																					success_flag = false;
																				}		/* if (json_object_set_new (root_p, SERVER_PROVIDER_S, copied_provider_p) != 0) */
																		}

																	if (success_flag)
																		{
																			if (service_p -> se_metadata_p)
																				{
																					success_flag = AddServiceMetadataToJSON (service_p -> se_metadata_p, root_p);
																				}

																			if (success_flag)
																				{
																					#if SERVICE_DEBUG >= STM_LEVEL_FINER
																					PrintJSONToLog (STM_LEVEL_FINER, __FILE__, __LINE__, root_p, "GetServiceAsJSON - service :: ");
																					#endif

																					return root_p;
																				}
																		}

																}		/* if (SetProviderType (copied_provider_p)) */
															else
																{

																}

														}		/* if (copied_provider_p) */
													else
														{
															PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, provider_p, "Failed to make copy of default provider");
														}

												}		/* if (provider_p) */

										}		/* if (SetJSONString (root_p, SERVICE_ALIAS_S, value_s)) */
									else
										{
											PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, root_p, "Failed to add \"%s\": \"%s\" to Service JSON", SERVICE_ALIAS_S, value_s);
										}

								}		/* if (SetJSONString (root_p, SERVICE_DESCRIPTION_S, value_s)) */
							else
								{
									PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, root_p, "Failed to add \"%s\": \"%s\" to Service JSON", SERVICE_DESCRIPTION_S, value_s);
								}

						}		/* if (SetJSONString (root_p, SERVICE_NAME_S, value_s)) */
					else
						{
							PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, root_p, "Failed to add \"%s\": \"%s\" to Service JSON", SERVICE_NAME_S, service_name_s);
						}

				}		/* if (SetJSONString (root_p, "@type", "grassroots_service")) */
			else
				{
					PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, root_p, "Failed to add \"@type\" =  \"grassroots_service\" to Service JSON");
				}

			json_decref (root_p);
		}		/* if (root_p) */


	return NULL;
}


const char *GetServiceIcon (const Service *service_p)
{
	const char *icon_uri_s = NULL;

	if ((service_p -> se_data_p) && (service_p -> se_data_p -> sd_config_p))
		{
			icon_uri_s = GetJSONString (service_p -> se_data_p -> sd_config_p, OPERATION_ICON_URI_S);
		}

	return icon_uri_s;
}


const char *GetServiceInformationPage (const Service *service_p)
{
	const char *info_uri_s = NULL;

	if ((service_p -> se_data_p) && (service_p -> se_data_p -> sd_config_p))
		{
			info_uri_s = GetJSONString (service_p -> se_data_p -> sd_config_p, OPERATION_INFORMATION_URI_S);
		}

	return info_uri_s;
}


static bool AddServiceNameToJSON (Service * const service_p, json_t *root_p)
{
	bool success_flag = false;
	const char *name_s = GetServiceName (service_p);

	if (name_s)
		{
			success_flag = (json_object_set_new (root_p, OPERATION_ID_S, json_string (name_s)) == 0);
		}

#if SERVICE_DEBUG >= STM_LEVEL_FINER
	PrintJSONToLog (STM_LEVEL_FINER, __FILE__, __LINE__, root_p, "AddServiceNameToJSON  :: ");
#endif

	return success_flag;
}


static bool AddServiceDescriptionToJSON (Service * const service_p, json_t *root_p)
{
	bool success_flag = false;
	const char *description_s = GetServiceDescription (service_p);

	if (description_s)
		{
			success_flag = (json_object_set_new (root_p, OPERATION_DESCRIPTION_S, json_string (description_s)) == 0);
		}

#if SERVICE_DEBUG >= STM_LEVEL_FINER
	PrintJSONToLog (STM_LEVEL_FINER, __FILE__, __LINE__, root_p, "AddServiceDescriptionToJSON  :: ");
#endif

	return success_flag;
}


static bool AddServiceUUIDToJSON (Service * const service_p, json_t *root_p)
{
	bool success_flag = false;
	char *uuid_s = GetServiceUUIDAsString (service_p);

	if (uuid_s)
		{
			success_flag = (json_object_set_new (root_p, SERVICE_UUID_S, json_string (uuid_s)) == 0);
			FreeUUIDString (uuid_s);
		}

#if SERVICE_DEBUG >= STM_LEVEL_FINER
	PrintJSONToLog (STM_LEVEL_FINER, __FILE__, __LINE__, root_p, "AddServiceUUIDToJSON  :: ");
#endif

	return success_flag;
}



static bool AddOperationInformationURIToJSON (Service * const service_p, json_t *root_p)
{
	bool success_flag = false;
	const char *uri_s = GetServiceInformationURI (service_p);

	if (uri_s)
		{
			success_flag = (json_object_set_new (root_p, OPERATION_INFORMATION_URI_S, json_string (uri_s)) == 0);
		}


#if SERVICE_DEBUG >= STM_LEVEL_FINER
	PrintJSONToLog (STM_LEVEL_FINER, __FILE__, __LINE__, root_p, "AddOperationInformationURIToJSON  :: ");
#endif

	return success_flag;
}


static bool AddServiceParameterSetToJSON (Service * const service_p, json_t *root_p, const SchemaVersion * const sv_p, const bool full_definition_flag, Resource *resource_p, UserDetails *user_p)
{
	bool success_flag = false;
	ParameterSet *param_set_p = GetServiceParameters (service_p, resource_p, user_p);

	if (param_set_p)
		{
			json_t *param_set_json_p = GetParameterSetAsJSON (param_set_p, sv_p, full_definition_flag);

			if (param_set_json_p)
				{
					if (json_object_set_new (root_p, PARAM_SET_KEY_S, param_set_json_p) == 0)
						{
							success_flag = true;
						}
					else
						{
							json_decref (param_set_json_p);
						}
				}

			/* could set this to be cached ... */
			ReleaseServiceParameters (service_p, param_set_p);
		}

#if SERVICE_DEBUG >= STM_LEVEL_FINER
	PrintJSONToLog (STM_LEVEL_FINER, __FILE__, __LINE__, root_p, "AddServiceParameterSetToJSON  :: ");
#endif

	return success_flag;
}



bool CreateAndAddPairedService (Service *service_p, struct ExternalServer *external_server_p, const char *remote_service_name_s, const json_t *op_p, const json_t *provider_p)
{
	PairedService *paired_service_p = AllocatePairedService (external_server_p -> es_id, remote_service_name_s, external_server_p -> es_uri_s, external_server_p -> es_name_s, op_p, provider_p);

	if (paired_service_p)
		{
			if (AddPairedService (service_p, paired_service_p))
				{
					return true;
				}

			FreePairedService (paired_service_p);
		}

	return false;
}


bool AddPairedService (Service *service_p, PairedService *paired_service_p)
{
	bool success_flag = false;
	PairedServiceNode *node_p = AllocatePairedServiceNode (paired_service_p);

	if (node_p)
		{
			LinkedListAddTail (& (service_p -> se_paired_services), (ListItem *) node_p);
			success_flag = true;
		}

	return success_flag;
}



static uint32 AddLinkedServices (Service *service_p, GrassrootsServer *grassroots_p)
{
	uint32 num_added_services = 0;

	if (service_p -> se_data_p)
		{
			if (service_p -> se_data_p -> sd_config_p)
				{
					const json_t *linked_services_config_p = json_object_get (service_p -> se_data_p -> sd_config_p, LINKED_SERVICES_S);

					if (linked_services_config_p)
						{
							if (json_is_array (linked_services_config_p))
								{
									json_t *link_config_p;
									size_t i;

									json_array_foreach (linked_services_config_p, i, link_config_p)
									{
										if (!CreateAndAddLinkedService (service_p, link_config_p, grassroots_p))
											{
												PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, link_config_p, "Failed to add LinkedService to \"%s\"", GetServiceName (service_p));
											}
									}


								}		/* if (json_is_array (linked_services_config_p)) */
							else
								{
									if (!CreateAndAddLinkedService (service_p, linked_services_config_p, grassroots_p))
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, linked_services_config_p, "Failed to add LinkedService to \"%s\"", GetServiceName (service_p));
										}
								}

						}		/* if (linked_services_config_p) */

				}		/* if (service_p -> se_data_p -> sd_config_p) */

		}		/* if (service_p -> se_data_p) */



	return num_added_services;
}


bool CreateAndAddLinkedService (Service *service_p, const json_t *service_config_p, GrassrootsServer *grassroots_p)
{
	LinkedService *linked_service_p = CreateLinkedServiceFromJSON (service_p, service_config_p, grassroots_p);

	if (linked_service_p)
		{
			if (AddLinkedService (service_p, linked_service_p))
				{
					return true;
				}

			FreeLinkedService (linked_service_p);
		}

	return false;
}


bool AddLinkedService (Service *service_p, LinkedService *linked_service_p)
{
	bool success_flag = false;
	LinkedServiceNode *node_p = AllocateLinkedServiceNode (linked_service_p);

	if (node_p)
		{
			LinkedListAddTail (& (service_p -> se_linked_services), (ListItem *) node_p);
			success_flag = true;
		}

	return success_flag;
}



json_t *GetServicesListAsJSON (LinkedList *services_list_p, Resource *resource_p, UserDetails *user_p, const bool add_service_ids_flag, ProvidersStateTable *providers_p)
{
	json_t *services_list_json_p = json_array ();

	if (services_list_json_p)
		{
			if (services_list_p)
				{
					ServiceNode *service_node_p = (ServiceNode *) (services_list_p -> ll_head_p);

					while (service_node_p)
						{
							json_t *service_json_p = GetServiceAsJSON (service_node_p -> sn_service_p, resource_p, user_p, add_service_ids_flag);

#if SERVICE_DEBUG >= STM_LEVEL_FINER
							PrintJSONToLog (STM_LEVEL_FINER, __FILE__, __LINE__, service_json_p, "service");
#endif

							if (service_json_p)
								{
									if (json_array_append_new (services_list_json_p, service_json_p) != 0)
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add JSON for service \"%s\" to list\n", GetServiceName (service_node_p -> sn_service_p));
										}
								}
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get JSON for service \"%s\"\n", GetServiceName (service_node_p -> sn_service_p));
								}

							service_node_p = (ServiceNode *) (service_node_p -> sn_node.ln_next_p);
						}

				}		/* if (services_list_p) */

#if SERVICE_DEBUG >= STM_LEVEL_FINE
			PrintJSONToLog (STM_LEVEL_FINE, __FILE__, __LINE__, services_list_json_p, "services list:");
#endif

			return services_list_json_p;
		}		/* if (services_list_json_p) */


	return NULL;
}


ServicesArray *AllocateServicesArray (const uint32 num_services)
{
	Service **services_pp = (Service **) AllocMemoryArray (num_services, sizeof (Service *));

	if (services_pp)
		{
			ServicesArray *services_array_p = (ServicesArray *) AllocMemory (sizeof (ServicesArray));

			if (services_array_p)
				{
					services_array_p -> sa_services_pp = services_pp;
					services_array_p -> sa_num_services = num_services;

					return services_array_p;					
				}

			FreeMemory (services_pp);
		}

	return NULL;
}


void FreeServicesArray (ServicesArray *services_p)
{
	uint32 i = services_p -> sa_num_services;
	Service **service_pp = services_p -> sa_services_pp;

	for ( ; i > 0; -- i, ++ service_pp)
		{
			if (*service_pp)
				{
					FreeService (*service_pp);
				}
		}

	FreeMemory (services_p -> sa_services_pp);
	FreeMemory (services_p);	
}



void AssignPluginForServicesArray (ServicesArray *services_p, Plugin *plugin_p)
{
	Service **service_pp = services_p -> sa_services_pp;
	int i = services_p -> sa_num_services;

	for ( ; i > 0; -- i, ++ service_pp)
		{
			if (*service_pp)
				{
					(*service_pp) -> se_plugin_p = plugin_p;
					IncrementPluginOpenCount (plugin_p);
				}
		}
}




/*
 "response": {
      "type": "object",
      "description": "The server's response from having run a service",
      "properties": {
        "service": { 
          "type": "string",
          "description": "The name of the service"
        },
        "status": { 
          "type": "string",
          "description": "The status of the job on the service",
          "enum": [
            "waiting",
            "failed to start",
            "started",
            "finished",
            "failed"
          ]    
        },
        "result": { 
          "type": "string",
          "description": "A value to be shown to the user"
        },
      }
    }, 
 */
bool AddServiceResponseHeader (Service *service_p, json_t *response_p)
{
	bool success_flag = false;
	const char *service_name_s = GetServiceName (service_p);


	if (service_name_s)
		{
			if (json_object_set_new (response_p, SERVICE_NAME_S, json_string (service_name_s)) == 0)
				{
					const char *service_description_s = GetServiceDescription (service_p);

					if (service_description_s)
						{
							if (json_object_set_new (response_p, SERVICE_DESCRIPTION_S, json_string (service_description_s)) == 0)
								{
									const char *info_uri_s = GetServiceInformationURI (service_p);

									if (info_uri_s)
										{
											if (json_object_set_new (response_p, OPERATION_INFORMATION_URI_S, json_string (info_uri_s)) != 0)
												{
													PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to add operation info uri \"%s\" to service response for \"%s\"", info_uri_s, service_name_s);
												}
										}

									success_flag = true;
								}
						}
				}
		}


#if SERVICE_DEBUG >= STM_LEVEL_FINE
	PrintJSONToLog (STM_LEVEL_FINER, __FILE__, __LINE__, response_p, "service response header");
#endif

	return success_flag;
}


bool SortServicesListByName (LinkedList *services_list_p)
{
	bool success_flag = false;

	success_flag = LinkedListSort (services_list_p, CompareServicesByName);

	return success_flag;
}


static int CompareServicesByName (const void *v0_p, const void *v1_p)
{
	int res = 0;
	const ServiceNode **node_0_pp = (const ServiceNode **) v0_p;
	const ServiceNode **node_1_pp = (const ServiceNode **) v1_p;
	const char *service_0_s = GetServiceName ((*node_0_pp) -> sn_service_p);
	const char *service_1_s = GetServiceName ((*node_1_pp) -> sn_service_p);

	res = Stricmp (service_0_s, service_1_s);

	return res;
}


ServicesArray *GetReferenceServicesFromJSON (json_t *config_p, const char *plugin_name_s, Service *(*get_service_fn) (json_t *config_p, size_t i, GrassrootsServer *grassroots_p), GrassrootsServer *grassroots_p)
{
	ServicesArray *services_p = NULL;

#if SERVICE_DEBUG >= STM_LEVEL_FINER
	PrintJSONToLog (STM_LEVEL_FINER, __FILE__, __LINE__, config_p, "GetReferenceServicesFromJSON: config");
#endif

	if (config_p)
		{
			if (json_is_object (config_p))
				{
					services_p = GetServiceFromConfigJSON (config_p, plugin_name_s, get_service_fn, grassroots_p);
				}
			else if (json_is_array (config_p))
				{
					size_t i;
					json_t *service_config_p;

					json_array_foreach (config_p, i, service_config_p)
					{

					}

				}
			else
				{
					PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, config_p, "reference config is not of required type");
				}

		}		/* if (config_p) */

	return services_p;
}



json_t *GetInterestedServiceJSON (Service *service_p, const char *keyword_s, const ParameterSet * const params_p, const bool full_definition_flag)
{
	json_t *res_p = NULL;
	json_error_t json_err;
	const char *service_name_s = GetServiceName (service_p);


	res_p = json_pack_ex (&json_err, 0, "{s:s,s:b,s:s}", SERVICE_NAME_S, service_name_s, SERVICE_RUN_S, true, "@type", "grassroots_service");

	if (res_p)
		{
			GrassrootsServer *grassroots_p = GetGrassrootsServerFromService (service_p);
			const SchemaVersion *sv_p = GetSchemaVersion (grassroots_p);
			json_t *params_json_p = GetParameterSetAsJSON (params_p, sv_p, full_definition_flag);

			if (params_json_p)
				{
					if (json_object_set_new (res_p, PARAM_SET_KEY_S, params_json_p) == 0)
						{
							const char *icon_s = GetServiceIcon (service_p);

							if (icon_s)
								{
									if (json_object_set_new (res_p, OPERATION_ICON_URI_S, json_string (icon_s)) != 0)
										{
											PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to add icon uri \"%s\" for service \"%s\"", icon_s, service_name_s);
										}
								}
						}
					else
						{
							json_decref (params_json_p);
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add parameters JSON to InterestedServiceJSON");
						}
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "GetParameterSetAsJSON failed");
				}
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "GetInterestedServiceJSON failed, %s", json_err.text);
		}

	return res_p;
}



void SetMetadataForService (Service *service_p, SchemaTerm *category_p, SchemaTerm *subcategory_p)
{
	SetServiceMetadataValues (service_p -> se_metadata_p, category_p, subcategory_p);
}



bool IsServiceLockable (const Service *service_p)
{
	return (service_p -> se_sync_data_p != NULL);
}


bool LockService (Service *service_p)
{
	bool success_flag = true;

	if (service_p -> se_sync_data_p)
		{
			success_flag = AcquireSyncDataLock (service_p -> se_sync_data_p);
		}

	return success_flag;
}


bool UnlockService (Service *service_p)
{
	bool success_flag = true;

	if (service_p -> se_sync_data_p)
		{
			success_flag = ReleaseSyncDataLock (service_p -> se_sync_data_p);
		}

	return success_flag;
}


bool AddServiceJobToService (Service *service_p, ServiceJob *job_p)
{
	bool added_flag = false;

	ServiceJobNode *node_p = AllocateServiceJobNode (job_p);

	if (node_p)
		{
			if (! (service_p -> se_jobs_p))
				{
					service_p -> se_jobs_p = AllocateServiceJobSet (service_p);
				}

			if (service_p -> se_jobs_p)
				{
					if (service_p -> se_sync_data_p)
						{
							if (AcquireSyncDataLock (service_p -> se_sync_data_p))
								{
									/*
									 * Is it already in the Service's job set?
									 */
									if (!FindServiceJobNodeByUUIDInServiceJobSet (service_p -> se_jobs_p, job_p -> sj_id))
										{
											LinkedListAddTail (service_p -> se_jobs_p -> sjs_jobs_p, (ListItem *) node_p);

											if (ReleaseSyncDataLock (service_p -> se_sync_data_p))
												{
													job_p -> sj_service_p = service_p;
													added_flag = true;
												}
											else
												{
													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to unlock SyncData for adding job \"%s\" to \"%s\"", job_p -> sj_name_s, GetServiceName (service_p));
												}
										}
									else
										{
											added_flag = true;
										}

								}
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to lock SyncData for adding job \"%s\" to \"%s\"", job_p -> sj_name_s, GetServiceName (service_p));
								}
						}
					else
						{
							if (!FindServiceJobNodeByUUIDInServiceJobSet (service_p -> se_jobs_p, job_p -> sj_id))
								{
									LinkedListAddTail (service_p -> se_jobs_p -> sjs_jobs_p, (ListItem *) node_p);
									job_p -> sj_service_p = service_p;
									added_flag = true;
								}
							else
								{
									added_flag = true;
								}
						}

				}		/* if (service_p -> se_jobs_p) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "No ServiceJobSet for \"%s\"", GetServiceName (service_p));
				}

			if (!added_flag)
				{
					node_p -> sjn_job_p = NULL;
					FreeServiceJobNode (& (node_p -> sjn_node));
				}

		}		/* if (node_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add job \"%s\" to \"%s\"", job_p -> sj_name_s, GetServiceName (service_p));
		}

	return added_flag;
}


bool RemoveServiceJobFromService (Service *service_p, ServiceJob *job_p)
{
	bool removed_flag = false;

	if (service_p -> se_sync_data_p)
		{
			if (AcquireSyncDataLock (service_p -> se_sync_data_p))
				{
					if (RemoveServiceJobByUUIDFromServiceJobSet (service_p -> se_jobs_p, job_p -> sj_id))
						{
							removed_flag = true;
						}
					else
						{
							char uuid_s [UUID_STRING_BUFFER_SIZE];

							ConvertUUIDToString (job_p -> sj_id, uuid_s);
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to find job \"%s\" for service \"%s\"", uuid_s, GetServiceName (service_p));
						}

					if (ReleaseSyncDataLock (service_p -> se_sync_data_p))
						{
							removed_flag = false;
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to unlock SyncData for removing job \"%s\" to \"%s\"", job_p -> sj_name_s, GetServiceName (service_p));
						}
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to lock SyncData for removing job \"%s\" to \"%s\"", job_p -> sj_name_s, GetServiceName (service_p));
				}
		}
	else
		{
			ServiceJobNode *node_p = FindServiceJobNodeByUUIDInServiceJobSet (service_p -> se_jobs_p, job_p -> sj_id);

			if (node_p)
				{
					LinkedListRemove (service_p -> se_jobs_p -> sjs_jobs_p, (ListItem *) node_p);
					removed_flag = true;
				}
			else
				{
					char uuid_s [UUID_STRING_BUFFER_SIZE];

					ConvertUUIDToString (job_p -> sj_id, uuid_s);
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to find job \"%s\" for service \"%s\"", uuid_s, GetServiceName (service_p));
				}
		}

	return removed_flag;
}


int32 GetNumberOfLiveJobs (Service *service_p)
{
	int32 num_live_jobs = 0;

	if (service_p -> se_jobs_p)
		{
			if ((!IsServiceLockable (service_p)) || (LockService (service_p)))
				{
					ServiceJobNode *node_p = (ServiceJobNode *) (service_p -> se_jobs_p -> sjs_jobs_p -> ll_head_p);

					while (node_p)
						{
							OperationStatus status = GetCachedServiceJobStatus (node_p -> sjn_job_p);

							if ((status == OS_PENDING) || (status == OS_STARTED))
								{
									++ num_live_jobs;
								}

							node_p = (ServiceJobNode *) (node_p -> sjn_node.ln_next_p);
						}

					if (! ((!IsServiceLockable (service_p)) || (UnlockService (service_p))))
						{
							num_live_jobs = -1;
						}
				}
			else
				{
					num_live_jobs = -1;
				}

		}		/* if (service_p -> se_jobs_p) */

	return num_live_jobs;
}


GrassrootsServer *GetGrassrootsServerFromService (const Service * const service_p)
{
	return service_p -> se_plugin_p -> pl_server_p;
}



bool DefaultGetParameterTypeForNamedParameter (const char *param_name_s, ParameterType *pt_p, const NamedParameterType *params_p)
{
  const NamedParameterType *param_p = params_p;
  
  while ((param_p) && (param_p -> npt_name_s))
    {
      if (strcmp (param_p -> npt_name_s, param_name_s) == 0)
        {
          *pt_p = param_p -> npt_type;
          return true;
        }
      else
        {
          ++ param_p;
        }
    }
  
  return false;
}


static ServicesArray *GetServiceFromConfigJSON (const json_t *service_config_p, const char * const plugin_name_s, Service *(*get_service_fn) (json_t *config_p, size_t i, GrassrootsServer *grassroots_p), GrassrootsServer *grassroots_p)
{
	ServicesArray *services_p = NULL;

	/* Make sure that the config refers to our service */
	json_t *value_p = json_object_get (service_config_p, PLUGIN_NAME_S);

	if (value_p)
		{
			if (json_is_string (value_p))
				{
					const char *value_s = json_string_value (value_p);

					if (strcmp (value_s, plugin_name_s) == 0)
						{
							json_t *ops_p = json_object_get (service_config_p, SERVER_OPERATION_S);

							if (ops_p)
								{
									if (json_is_array (ops_p))
										{
											size_t num_ops = json_array_size (ops_p);

											services_p = AllocateServicesArray (num_ops);

											if (services_p)
												{
													size_t i = 0;
													Service **service_pp = services_p -> sa_services_pp;

													while (i < num_ops)
														{
															json_t *op_p =  json_array_get (ops_p, i);
															json_t *copied_op_p = json_deep_copy (op_p);

															if (copied_op_p)
																{
																	Service *service_p = get_service_fn (copied_op_p, i, grassroots_p);

																	if (service_p)
																		{
																			*service_pp = service_p;
																		}
																	else
																		{
																			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, op_p, "Failed to create service %lu\n", i);
																			json_decref (copied_op_p);
																		}

																}		/* if (copied_op_p) */
															else
																{
																	PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to copy refererence service %lu\n", i);
																}


															++ i;
															++ service_pp;
														}		/* while (i < num_ops) */

												}		/* if (services_p) */

										}		/* if (json_is_array (ops_p)) */
									else
										{
											services_p = AllocateServicesArray (1);

											if (services_p)
												{
													Service **service_pp = services_p -> sa_services_pp;
													json_t *copied_service_config_p = json_deep_copy (service_config_p);

													if (copied_service_config_p)
														{
															Service *service_p = get_service_fn (copied_service_config_p, 0, grassroots_p);

															if (service_p)
																{
																	*service_pp = service_p;
																}
															else
																{
																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, ops_p, "Failed to create service\n");

																	FreeServicesArray (services_p);
																	services_p = NULL;
																	json_decref (copied_service_config_p);
																}

														}
													else
														{
															PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to copy single refererence service\n");
														}


												}		/* if (services_p)  */
										}

								}

						}		/* if (strcmp (value_s, plugin_name_s) == 0) */

				}		/* if (json_is_string (value_p)) */

		}		/* if (value_p) */

	return services_p;
}
