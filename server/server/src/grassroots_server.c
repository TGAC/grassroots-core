/*
 ** Copyright 2014-2018 The Earlham Institute
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
 * grassroots_server.c
 *
 *  Created on: 26 Jul 2019
 *      Author: billy
 */


#include <string.h>

#include "grassroots_server.h"
#include "memory_allocations.h"
#include "streams.h"

#include "service_matcher.h"
#include "jobs_manager.h"
#include "mongo_client_manager.h"
#include "key_value_pair.h"
#include "handler_utils.h"
#include "time_util.h"
#include "provider.h"
#include "string_parameter.h"
#include "uuid_util.h"

#include "service_util.h"


#define GRASSROOTS_SERVE_DEBUG (STM_LEVEL_FINEST)

/*
 * STATIC DECLARATIONS
 */


#if IRODS_ENABLED == 1
#include "user.h"
static json_t *GetAllModifiedData(const json_t *const req_p,
                                  UserDetails *user_p);
#endif


static json_t *LoadConfig (const char *root_path_s, const char *config_filename_s);


static JobsManager *LoadJobsManagerFromConfig (GrassrootsServer *server_p);

static ServersManager *LoadServersManagerFromConfig (GrassrootsServer *server_p);

static SchemaVersion *InitSchemaVersionDetails (json_t *config_p);

static json_t *GetInterestedServices (GrassrootsServer *grassroots_p, const json_t * const req_p, UserDetails *user_p);

static json_t *GetAllServices (GrassrootsServer *grassroots_p, const json_t * const req_p, UserDetails *user_p);

static json_t *RunKeywordServices (GrassrootsServer *grassroots_p, const json_t * const req_p, UserDetails *user_p, const char *keyword_s);

static json_t *GetServiceResultsAsJSON (GrassrootsServer *grassroots_p, const json_t * const req_p, UserDetails *user_p);

static json_t *GetServicesAsJSON (GrassrootsServer *grassroots_p, UserDetails *user_p, DataResource *resource_p, Handler *handler_p, ProvidersStateTable *providers_p);

//static json_t *GetNamedServices (GrassrootsServer *grassroots_p, const json_t * const req_p, UserDetails *user_p);

//static json_t *GetNamedServiceInfo (GrassrootsServer *grassroots_p, const json_t * const req_p, UserDetails *user_p);


static json_t *GetNamedServicesFunctionality (GrassrootsServer *grassroots_p, const json_t * const req_p, UserDetails *user_p, Operation op);

static json_t *GetServiceData (GrassrootsServer *grassroots_p, const json_t * const req_p, UserDetails *user_p, bool (*callback_fn) (GrassrootsServer *grassroots_p, json_t *services_p, uuid_t service_id, const char *uuid_s));

static int8 ProcessServiceFromJSON (GrassrootsServer *grassroots_p, const json_t *service_req_p, const json_t *paired_servers_req_p, UserDetails *user_p, json_t *res_p, uuid_t user_uuid, const char **key_ss);

static bool AddServiceStatusToJSON (GrassrootsServer *grassroots_p, json_t *results_p, uuid_t job_id, const char *uuid_s);

static bool AddServiceResultsToJSON (GrassrootsServer *grassroots_p, json_t *results_p, uuid_t job_id, const char *uuid_s);

static bool AddServiceDataToJSON (GrassrootsServer *grassroots_p, json_t *results_p, uuid_t job_id, const char *uuid_s, const char * const identifier_s, json_t *(*get_job_json_fn) (ServiceJob *job_p, bool omit_results_flag));

static json_t *GetServerStatus (GrassrootsServer *grassroots_p, const json_t * const req_p, UserDetails *user_p);

static json_t *GetRequestedResource (GrassrootsServer *grassroots_p, const json_t * const req_p, UserDetails *user_p);

static json_t *GenerateNamedServices (GrassrootsServer *grassroots_p, LinkedList *services_p, const json_t * const req_p, UserDetails *user_p, ProvidersStateTable *providers_p);

static int32 AddPairedServices (GrassrootsServer *grassroots_p, Service *internal_service_p, UserDetails *user_p, ProvidersStateTable *providers_p);

static int32 AddAllPairedServices (GrassrootsServer *grassroots_p, LinkedList *internal_services_p, UserDetails *user_p, ProvidersStateTable *providers_p);

static json_t *GenerateServiceIndexingData (GrassrootsServer *grassroots_p, LinkedList *services_p, const json_t * const req_p, UserDetails *user_p, ProvidersStateTable *providers_p);

static LinkedList *GetServicesList (GrassrootsServer *grassroots_p, UserDetails *user_p, DataResource *resource_p, Handler *handler_p, ProvidersStateTable *providers_p);

static bool IsRequiredExternalOperation (const json_t *external_op_p, const char *op_name_s);

static DataResource *GetResourceOfInterest (const json_t * const req_p);

static const char *GetProviderElement (const GrassrootsServer *grassroots_p, const char * const element_s);

static struct MongoClientManager *GetMongoClientManager (const json_t *config_p);

static DataResource *GetResourceFromRequest (const json_t *req_p);

static void ProcessServiceRequest (const json_t *service_req_p, json_t *services_res_p, GrassrootsServer *grassroots_p, ProvidersStateTable *providers_p, const char *server_s, UserDetails *user_p, Operation op);

static int8 RunServiceFromJSON (GrassrootsServer *grassroots_p, Service *service_p, const json_t *service_req_p, const json_t *paired_servers_req_p, UserDetails *user_p, json_t *res_p);

static int8 RefreshServiceFromJSON (GrassrootsServer *grassroots_p, Service *service_p, json_t *service_req_p, const json_t *paired_servers_req_p, UserDetails *user_p, json_t *res_p);

static int8 GetServiceIndexingInformation (GrassrootsServer *grassroots_p, Service *service_p, const json_t *service_req_p, const json_t *paired_servers_req_p, UserDetails *user_p, json_t *res_p);

static uint32 GetMatchingServices (GrassrootsServer *grassroots_p,
                                  ServiceMatcher *matcher_p,
                                  UserDetails *user_p,
                                  LinkedList *services_list_p,
                                  bool multiple_match_flag);

static uint32 GetMatchingReferrableServices (GrassrootsServer *grassroots_p,
                                            ServiceMatcher *matcher_p,
                                            UserDetails *user_p,
                                            const json_t *reference_config_p,
                                            LinkedList *services_list_p,
                                            bool multiple_match_flag);

static uint32 FindMatchingServices(GrassrootsServer *grassroots_p, ServiceMatcher *matcher_p,
                     UserDetails *user_p, const json_t *reference_config_p,
                     LinkedList *services_list_p, bool multiple_match_flag,
                     bool native_services_flag);

static const char *GetPluginNameFromJSON (const json_t *const root_p);


static uint32 AddMatchingServicesFromServicesArray (ServicesArray *services_p, LinkedList *matching_services_p, ServiceMatcher *matcher_p, bool multiple_match_flag);

static void PrintGrassrootsServer (const GrassrootsServer *grassroots_p);

/*
 * API DEFINITIONS
 */

GrassrootsServer *AllocateGrassrootsServer (const char *grassroots_path_s, const char *config_filename_s, const char *service_config_path_s, const char *services_path_s, const char *references_path_s,
	const char *jobs_managers_path_s, const char *servers_managers_path_s, JobsManager *external_jobs_manager_p, MEM_FLAG jobs_manager_flag, ServersManager *external_servers_manager_p, MEM_FLAG servers_manager_flag)
{
	char *copied_path_s = EasyCopyToNewString (grassroots_path_s);

	if (copied_path_s)
		{
			char *copied_config_filename_s = EasyCopyToNewString (config_filename_s ? config_filename_s: "grassroots.config");

			if (copied_config_filename_s)
				{
					char *copied_service_config_path_s = EasyCopyToNewString (service_config_path_s ? service_config_path_s: "config");

					if (copied_service_config_path_s)
						{
              char *copied_services_path_s = EasyCopyToNewString (services_path_s ? services_path_s: "services");

              if (copied_services_path_s)
								{
									char *copied_references_path_s = EasyCopyToNewString (references_path_s ? references_path_s: "references");

									if (copied_references_path_s)
										{
											char *copied_jobs_managers_path_s = EasyCopyToNewString (jobs_managers_path_s ? jobs_managers_path_s: "jobs_managers");

											if (copied_jobs_managers_path_s)
												{
													char *copied_servers_managers_path_s = EasyCopyToNewString (servers_managers_path_s ? servers_managers_path_s: "servers");

													if (copied_servers_managers_path_s)
														{
															json_t *config_p = LoadConfig (copied_path_s, copied_config_filename_s);

															if (config_p)
																{
																	SchemaVersion *sv_p = InitSchemaVersionDetails (config_p);

																	if (sv_p)
																		{
																			struct MongoClientManager *mongo_manager_p = GetMongoClientManager (config_p);

																			if (mongo_manager_p)
																				{
																					GrassrootsServer *grassroots_p = (GrassrootsServer *) AllocMemory (sizeof (GrassrootsServer));

																					if (grassroots_p)
																						{
																							grassroots_p -> gs_path_s = copied_path_s;
																							grassroots_p -> gs_config_filename_s = copied_config_filename_s;
																							grassroots_p -> gs_config_p = config_p;
																							grassroots_p -> gs_config_path_s = copied_service_config_path_s;
																							grassroots_p -> gs_services_path_s = copied_services_path_s;
																							grassroots_p -> gs_references_path_s = copied_references_path_s;

																							grassroots_p -> gs_jobs_managers_path_s = copied_jobs_managers_path_s;

																							grassroots_p -> gs_jobs_manager_p = external_jobs_manager_p;
																							grassroots_p -> gs_jobs_manager_mem = jobs_manager_flag;

																							grassroots_p -> gs_servers_managers_path_s = copied_servers_managers_path_s;

																							grassroots_p -> gs_servers_manager_p = external_servers_manager_p;
																							grassroots_p -> gs_servers_manager_mem = servers_manager_flag;

																							grassroots_p -> gs_schema_version_p = sv_p;

																							grassroots_p -> gs_mongo_manager_p = mongo_manager_p;

																							/*
																							 * Load the jobs manager
																							 */
																							if (!external_jobs_manager_p)
																								{
																									JobsManager *jobs_manager_p = LoadJobsManagerFromConfig (grassroots_p);

																									if (jobs_manager_p)
																										{
																											grassroots_p -> gs_jobs_manager_p = jobs_manager_p;
																											grassroots_p -> gs_jobs_manager_mem = MF_SHALLOW_COPY;
																										}
																								}


																							/*
																							 * Load the servers manager
																							 */
																							if (!external_servers_manager_p)
																								{
																									ServersManager *servers_manager_p = LoadServersManagerFromConfig (grassroots_p);

																									if (servers_manager_p)
																										{
																											grassroots_p -> gs_servers_manager_p = servers_manager_p;
																											grassroots_p -> gs_servers_manager_mem = MF_SHALLOW_COPY;
																										}

																								}

																							if (grassroots_p -> gs_servers_manager_p)
																								{
																									ConnectToExternalServers (grassroots_p);
																								}



																							/*
																								#ifdef DRMAA_ENABLED
																								if (res_flag)
																									{
																										res_flag = InitDrmaaEnvironment ();
																									}
																								#endif


																								#ifdef IRODS_ENABLED
																								if (res_flag)
																									{
																										InitRodsEnv ();
																									}
																								#endif
																							 */

																							/*
																							ConnectToExternalServers (grassroots_p);
																							 */

																							return grassroots_p;
																						}		/* if (grassroots_p) */
																					else
																						{
																							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate grassroots server");
																						}


																					FreeMongoClientManager (mongo_manager_p);
																				}		/* if (mongo_manager_p) */
																			else
																				{
																					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get mongo client manager");
																				}

																			FreeSchemaVersion (sv_p);
																		}		/* if (sv_p) */
																	else
																		{
																			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate SchemaVersion");
																		}

																	json_decref (config_p);
																}		/* if (config_p) */
															else
																{
																	PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to load config from  \"%s\" \"%s\"", copied_path_s, copied_config_filename_s);
																}

															FreeCopiedString (copied_servers_managers_path_s);
														}		/* if (copied_servers_managers_path_s) */
													else
														{
															PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to copy servers_managers_path_s \"%s\"", servers_managers_path_s);
														}

													FreeCopiedString (copied_jobs_managers_path_s);
												}		/* if (copied_jobs_managers_path_s) */
											else
												{
													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to copy jobs_managers_path_s \"%s\"", jobs_managers_path_s);
												}

											FreeCopiedString (copied_references_path_s);
										}		/* if (copied_service_config_path_s) */
									else
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to copy references_path_s \"%s\"", references_path_s);
										}

									FreeCopiedString (copied_services_path_s);
								}		/* if (copied_services_path_s) */
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to copy services_path_s \"%s\"", services_path_s);
								}

							FreeCopiedString (copied_service_config_path_s);
						}		/* if (copied_service_config_path_s) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to copy service config path \"%s\"", service_config_path_s);
						}

					FreeCopiedString (copied_config_filename_s);
				}		/* if (copied_config_filename_s) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to copy config filename \"%s\"", config_filename_s);
				}

			FreeCopiedString (copied_path_s);
		}		/* if (copied_path_s) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to copy grassroots path \"%s\"", grassroots_path_s);
		}

	return NULL;
}


void FreeGrassrootsServer (GrassrootsServer *server_p)
{
	if (server_p -> gs_jobs_manager_p)
		{
			switch (server_p -> gs_jobs_manager_mem)
			{
				case MF_SHALLOW_COPY:
				case MF_DEEP_COPY:
					FreeJobsManager (server_p -> gs_jobs_manager_p);
					break;

				default:
					break;
			}
		}


	DisconnectFromExternalServers (server_p);


	if (server_p -> gs_servers_manager_p)
		{
			switch (server_p -> gs_servers_manager_mem)
			{
				case MF_SHALLOW_COPY:
				case MF_DEEP_COPY:
					FreeServersManager (server_p -> gs_servers_manager_p);
					break;

				default:
					break;
			}
		}

	json_decref (server_p -> gs_config_p);

	FreeCopiedString (server_p -> gs_path_s);
	FreeCopiedString (server_p -> gs_config_filename_s);
	FreeCopiedString (server_p -> gs_config_path_s);
	FreeCopiedString (server_p -> gs_references_path_s);
	FreeCopiedString (server_p -> gs_services_path_s);

	FreeCopiedString (server_p -> gs_jobs_managers_path_s);
	FreeCopiedString (server_p -> gs_servers_managers_path_s);


	if (server_p -> gs_mongo_manager_p)
		{
			FreeMongoClientManager (server_p -> gs_mongo_manager_p);
		}


	FreeSchemaVersion (server_p -> gs_schema_version_p);

	FreeMemory (server_p);
}


const char *GetServerRootDirectory (const GrassrootsServer * const grassroots_p)
{
	return grassroots_p -> gs_path_s;
}


void LoadMatchingServicesByName (GrassrootsServer *grassroots_p, LinkedList *services_p, const char *service_name_s, const char *service_alias_s, UserDetails *user_p)
{
	NameServiceMatcher matcher;

	InitOperationNameServiceMatcher (&matcher, service_name_s, service_alias_s);

	/* Since we're after a service with a given name, we don't need multiple matches */
	GetMatchingServices (grassroots_p, & (matcher.nsm_base_matcher), user_p, services_p, false);

	if (services_p -> ll_size == 0)
		{
			AddReferenceServices (grassroots_p, services_p, service_name_s, user_p);
		}
}



void LoadMatchingServices (GrassrootsServer *grassroots_p, LinkedList *services_p, DataResource *resource_p, Handler *handler_p, UserDetails *user_p)
{
	ResourceServiceMatcher matcher;

	InitResourceServiceMatcher (&matcher, resource_p, handler_p);

	GetMatchingServices (grassroots_p, & (matcher.rsm_base_matcher), user_p, services_p, true);

	AddReferenceServices (grassroots_p, services_p, NULL, user_p);
}


void LoadKeywordServices (GrassrootsServer *grassroots_p, LinkedList *services_p, UserDetails *user_p)
{
	KeywordServiceMatcher matcher;

	InitKeywordServiceMatcher (&matcher);

	GetMatchingServices (grassroots_p, & (matcher.ksm_base_matcher), user_p, services_p, true);

	AddReferenceServices (grassroots_p, services_p, NULL, user_p);
}


/**
 * Load any json stubs for external services that are used to configure generic services,
 * e.g. web services
 */
void AddReferenceServices (GrassrootsServer *grassroots_p, LinkedList *services_p, const char *operation_name_s, UserDetails *user_p)
{
	const char *root_path_s = GetServerRootDirectory (grassroots_p);
	char *full_references_path_s = MakeFilename (root_path_s, grassroots_p -> gs_references_path_s);

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


json_t *ProcessServerRawMessage (GrassrootsServer *server_p, const char * const request_s, const char **error_ss)
{
	json_error_t error;
	json_t *req_p = json_loads (request_s, JSON_PRESERVE_ORDER, &error);
	json_t *res_p = NULL;

	if (req_p)
		{
			res_p = ProcessServerJSONMessage (server_p, req_p, error_ss);

			if (*error_ss)
				{
					PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Error \"%s\" from ProcessServerJSONMessage for :\n%s\n", *error_ss, request_s);
				}

			json_decref (req_p);
		}
	else
		{
			/* error decoding the request */
			PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Could not get load json from:\n%s\n", request_s);
		}

	return res_p;
}


json_t *ProcessServerJSONMessage (GrassrootsServer *grassroots_p, json_t *req_p, const char **error_ss)
{
	json_t *res_p = NULL;

	if (req_p)
		{
			if (json_is_object (req_p))
				{
					Operation op;
					json_t *op_p = NULL;
					UserDetails *user_p = NULL;
					json_t *uri_p = NULL;
					json_t *config_p = json_object_get (req_p, CONFIG_S);

					if (config_p)
						{
							json_t *credentials_p = json_object_get (config_p, CREDENTIALS_S);

							/*
							 * Create the UserDetails from the JSON details.
							 */
							if (credentials_p)
								{
									user_p = AllocateUserDetails (credentials_p);

									if (!user_p)
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, credentials_p, "Failed to create UserDetails");
										}
								}
						}

#if SERVER_DEBUG >= STM_LEVEL_FINEST
					if (req_p)
						{
							PrintJSONToLog (req_p,"ProcessMessage - request: \n", STM_LEVEL_FINEST);
						}
#endif


					/*
					 * Is this request for an external server?
					 */
					uri_p = json_object_get (req_p, SERVER_URI_S);
					if (uri_p)
						{
							/*
							 * Find the matching external server,
							 * Remove the server uuid and proxy
							 * the request/response
							 */
							if (json_is_string (uri_p))
								{
									const char *uuid_s = json_string_value (uri_p);
									uuid_t key = { 0 };

									if (ConvertStringToUUID (uuid_s, key))
										{
											ServersManager *manager_p = GetServersManager (grassroots_p);

											if (manager_p)
												{
													ExternalServer *external_server_p = GetExternalServerFromServersManager (manager_p, uuid_s, NULL);

													if (external_server_p)
														{
															/* remove the server's uuid */
															if (json_object_del (req_p, SERVER_UUID_S) == 0)
																{
																	/* we can now proxy the request off to the given server */
																	json_t *response_p = MakeRemoteJSONCallToExternalServer (external_server_p, req_p);

																	if (response_p)
																		{
																			/*
																			 * We now need to add the ExternalServer's
																			 * uuid back in. Not sure if we can use uuid_s
																			 * as it may have gone out of scope when we called
																			 * json_obtject_del, so best to recreate it.
																			 */
																			char buffer [UUID_STRING_BUFFER_SIZE];

																			ConvertUUIDToString (key, buffer);

																			if (json_object_set_new (response_p, SERVER_UUID_S, json_string (buffer)) == 0)
																				{

																				}		/* if (json_object_set_new (response_p, SERVER_UUID_S, json_string (buffer) == 0)) */

																		}		/* if (response_p) */

																}		/* if (json_object_del (req_p, SERVER_UUID_S) == 0) */

														}		/* if (external_server_p)*/

												}		/* if (manager_p) */

										}		/* if (ConvertStringToUUID (uuid_s, key)) */

								}		/* if (json_is_string (uuid_p)) */

						}		/* if (uuid_p) */
					else
						{
							/* the request is for this server */
						}

					op = GetOperationFromTopLevelJSON (req_p);

					if (op != OP_NONE)
						{
							switch (op)
							{
								case OP_LIST_ALL_SERVICES:
									res_p = GetAllServices (grassroots_p, req_p, user_p);
									break;

									//									case OP_IRODS_MODIFIED_DATA:
									//										{
									//											#if IRODS_ENABLED == 1
									//											res_p = GetAllModifiedData (req_p, user_p);
									//											#endif
									//										}
									//										break;

								case OP_LIST_INTERESTED_SERVICES:
									res_p = GetInterestedServices (grassroots_p, req_p, user_p);
									break;

								case OP_GET_NAMED_SERVICES:
								case OP_GET_SERVICE_INFO:
									res_p = GetNamedServicesFunctionality (grassroots_p, req_p, user_p, op);
									break;

								case OP_GET_SERVICE_RESULTS:
									res_p = GetServiceResultsAsJSON (grassroots_p, req_p, user_p);
									break;

								case OP_GET_RESOURCE:
									res_p = GetRequestedResource (grassroots_p, req_p, user_p);
									break;

								case OP_SERVER_STATUS:
									res_p = GetServerStatus (grassroots_p, req_p, user_p);
									break;

								default:
									break;
							}		/* switch (op) */

						}		/* if (op != OP_NONE) */
					else if ((op_p = json_object_get (req_p, SERVICES_NAME_S)) != NULL)
						{
							json_t *service_results_p = json_array ();

							if (service_results_p)
								{
									const char *key_s = NULL;
									uuid_t user_uuid = {0};

									uuid_clear (user_uuid);
									const json_t *external_servers_req_p = json_object_get (req_p, SERVERS_S);

									if (json_is_array (op_p))
										{
											size_t i;
											json_t *value_p;

											json_array_foreach (op_p, i, value_p)
											{
												int8 res = ProcessServiceFromJSON (grassroots_p, value_p, external_servers_req_p, user_p, service_results_p, user_uuid, &key_s);

												if (res < 0)
													{
														char *value_s = json_dumps (value_p, JSON_INDENT (2));

														if (value_s)
															{
																PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to run service from %s", value_s);
																free (value_s);
															}
														else
															{
																PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to run service from");
															}
													}
											}		/* json_array_foreach (op_p, i, value_p) */
										}
									else
										{
											int8 res = ProcessServiceFromJSON (grassroots_p, op_p, external_servers_req_p, user_p, service_results_p, user_uuid, &key_s);

											if (res < 0)
												{
													char *value_s = json_dumps (op_p, JSON_INDENT (2));

													if (value_s)
														{
															PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to run service from %s", value_s);
															free (value_s);
														}
													else
														{
															PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to run service from");
														}
												}
										}

									if (key_s)
										{
											res_p = GetInitialisedResponseOnServer (grassroots_p, req_p, key_s, service_results_p);

											if (!res_p)
												{
													*error_ss = "Failed to create service response";
													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create service result response");
												}
										}


								}		/* if (service_results_p) */
							else
								{
									*error_ss = "Failed to create service results array";
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create service results array");
								}

						}		/* 	else if ((op_p = json_object_get (req_p, SERVICES_NAME_S)) != NULL) */


#if SERVER_DEBUG >= STM_LEVEL_FINER
					PrintJSONToLog (res_p, "ProcessMessage - response: \n", STM_LEVEL_FINER, __FILE__, __LINE__);
					FlushLog ();
#endif


					if (user_p)
						{
							FreeUserDetails (user_p);
						}

				}		/* if (json_is_object (req_p)) */
			else
				{
					*error_ss = "Request is not an explicit json object";
				}

		}		/* if (req_p) */
	else
		{
			*error_ss = "Request is NULL";
		}

	return res_p;
}



json_t *GetGlobalServiceConfig (GrassrootsServer *grassroots_p, const char * const service_name_s, bool *alloc_flag_p)
{
	json_t *res_p = NULL;
	char *conf_s = NULL;
	char sep_s [2];
	const char *config_s = grassroots_p -> gs_config_path_s ? grassroots_p -> gs_config_path_s : "config";

	*sep_s = GetFileSeparatorChar ();
	* (sep_s + 1) = '\0';

	*alloc_flag_p = false;

	conf_s = ConcatenateVarargsStrings (config_s, sep_s, service_name_s, NULL);

	if (conf_s)
		{
			char *full_config_path_s = MakeFilename (grassroots_p -> gs_path_s, conf_s);

			if (full_config_path_s)
				{
					if (IsPathValid (full_config_path_s))
						{
							res_p = LoadJSONFile (full_config_path_s);

							if (res_p)
								{
									*alloc_flag_p = true;
								}
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to load config filename for %s", full_config_path_s);
								}
						}
					else
						{
							PrintErrors (STM_LEVEL_FINEST, __FILE__, __LINE__, "Separate config file %s does not exist", full_config_path_s);
						}

					FreeCopiedString (full_config_path_s);
				}		/* if (full_config_path_s) */
			else
				{
					PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to create full config filename for %s", conf_s);
				}

			FreeCopiedString (conf_s);
		}		/* if (conf_s) */
	else
		{
			PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to create local config filename for %s", service_name_s);
		}

	if (!res_p)
		{
			const json_t *config_p = grassroots_p -> gs_config_p;

			if (config_p)
				{
					json_t *json_p = json_object_get (config_p, SERVICES_NAME_S);

					if (json_p)
						{
							if (json_is_object (json_p))
								{
									res_p = json_object_get (json_p, service_name_s);
								}		/* if (json_is_object (json_p)) */

						}		/* if (json_p) */

				}		/* if (config_p) */
			else
				{

				}

		}		/* if (!res_p) */


	return res_p;
}


JobsManager *GetJobsManager (GrassrootsServer *grassroots_p)
{
	return grassroots_p -> gs_jobs_manager_p;
}


json_t *GetInitialisedResponseOnServer (GrassrootsServer *server_p, const json_t *req_p, const char *key_s, json_t *value_p)
{
	const SchemaVersion * const sv_p = GetSchemaVersion (server_p);
	json_t *res_p = GetInitialisedMessage (sv_p);

	if (res_p)
		{
			if (req_p)
				{
					bool verbose_flag = false;

					GetJSONBoolean (req_p, REQUEST_VERBOSE_S, &verbose_flag);

					if (verbose_flag)
						{
							json_t *copied_req_p = json_deep_copy (req_p);

							if (copied_req_p)
								{
									if (json_object_set_new (res_p, REQUEST_S, copied_req_p) != 0)
										{
											PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to add request to response header");
											json_decref (copied_req_p);
										}

								}		/* if (copied_req_p) */
							else
								{
									PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, req_p, "Failed to add request to response header");
								}

						}		/* if (verbose_flag) */

				}		/* if (req_p) */

			if (key_s && value_p)
				{
					if (json_object_set_new (res_p, key_s, value_p) != 0)
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s to the response header", key_s);
							json_decref (res_p);
						}
				}		/* if (key_s && value_p) */

		}		/* if (res_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get initialised message");
		}

	return res_p;
}



const SchemaVersion *GetSchemaVersion (GrassrootsServer *server_p)
{
	return server_p -> gs_schema_version_p;
}


ServersManager *GetServersManager (GrassrootsServer *server_p)
{
	return server_p -> gs_servers_manager_p;
}



void DisconnectFromExternalServers (GrassrootsServer *server_p)
{

}


void ConnectToExternalServers (GrassrootsServer *grassroots_p)
{
	json_t *servers_p = json_object_get (grassroots_p -> gs_config_p, SERVERS_S);

	if (servers_p)
		{
			ServersManager *manager_p = grassroots_p -> gs_servers_manager_p;

			if (manager_p)
				{
					if (json_is_object (servers_p))
						{
							AddExternalServerFromJSON (manager_p, servers_p);
						}		/* if (json_is_object (json_p)) */
					else if (json_is_array (servers_p))
						{
							size_t index;
							json_t *element_p;

							json_array_foreach (servers_p, index, element_p)
							{
								AddExternalServerFromJSON (manager_p, element_p);
							}
						}

				}


		}		/* if (servers_p) */

}


json_t *GetGlobalConfigValue (const GrassrootsServer *grassroots_p, const char *key_s)
{
	json_t *value_p = json_object_get (grassroots_p -> gs_config_p, key_s);

	return value_p;
}


const char *GetServerProviderName (const GrassrootsServer *grassroots_p)
{
	return GetProviderElement (grassroots_p, PROVIDER_NAME_S);
}


const char *GetServerProviderDescription (const GrassrootsServer *grassroots_p)
{
	return GetProviderElement (grassroots_p, PROVIDER_DESCRIPTION_S);
}


const char *GetServerProviderURI (const GrassrootsServer *grassroots_p)
{
	return GetProviderElement (grassroots_p, PROVIDER_URI_S);
}


const json_t *GetProviderAsJSON (const GrassrootsServer *grassroots_p)
{
	const json_t *provider_p = GetProviderDetails (grassroots_p -> gs_config_p);

	return provider_p;
}


const char *GetJobLoggingURI (const GrassrootsServer *grassroots_p)
{
	const char *uri_s = NULL;
	const json_t *jobs_p = GetCompoundJSONObject (grassroots_p -> gs_config_p, "admin.jobs");

	if (jobs_p)
		{
			uri_s = GetJSONString (jobs_p, "uri");
		}

	return uri_s;
}


bool IsServiceEnabled (const GrassrootsServer *grassroots_p, const char *service_name_s)
{
	bool enabled_flag = true;
	const json_t *services_config_p = GetGlobalConfigValue (grassroots_p, SERVICES_NAME_S);

	if (services_config_p)
		{
			const json_t *service_statuses_p = json_object_get (services_config_p, SERVICES_STATUS_S);

			if (service_statuses_p)
				{
					const json_t *service_p = json_object_get (service_statuses_p, service_name_s);

					GetJSONBoolean (service_statuses_p, SERVICES_STATUS_DEFAULT_S, &enabled_flag);

					if (service_p)
						{
							if (json_is_true (service_p))
								{
									enabled_flag = true;
								}
							else if (json_is_false (service_p))
								{
									enabled_flag = false;
								}

						}		/* if (service_p) */

				}		/* if (service_statuses_p) */

		}		/* if (services_config_p) */

	return enabled_flag;
}


/*
 * STATIC DEFINITIONS
 */


static const char *GetProviderElement (const GrassrootsServer *grassroots_p, const char * const element_s)
{
	const char *result_s = NULL;
	const json_t *provider_p = GetProviderAsJSON (grassroots_p);

	if (provider_p)
		{
			result_s = GetJSONString (provider_p, element_s);
		}

	return result_s;
}


static int8 ProcessServiceFromJSON (GrassrootsServer *grassroots_p, const json_t *service_req_p, const json_t *paired_servers_req_p, UserDetails *user_p, json_t *res_p, uuid_t user_uuid, const char **key_ss)
{
	/* Get the requested operation */
	int8 res = 0;
	json_t *op_p = json_object_get (service_req_p, SERVICE_RUN_S);
	typedef enum
	{
		RUN,
		REFRESH,
		INDEXING_DATA,
		NONE
	} Mode;
	Mode mode = NONE;

	if (op_p)
		{
			if (json_is_true (op_p))
				{
					mode = RUN;
					*key_ss = SERVICE_RESULTS_S;
				}
		}

	if (mode == NONE)
		{
			op_p = json_object_get (service_req_p, SERVICE_REFRESH_S);

			if (json_is_true (op_p))
				{
					mode = REFRESH;
					*key_ss = SERVICES_NAME_S;
				}
		}

	if (mode == NONE)
		{
			op_p = json_object_get (service_req_p, SERVICE_INDEXING_DATA_S);

			if (json_is_true (op_p))
				{
					mode = INDEXING_DATA;
					*key_ss = SERVICES_NAME_S;
				}
		}


	if (mode != NONE)
		{
			const char *name_s = GetServiceNameFromJSON (service_req_p);
			const char *alias_s = GetServiceAliasFromJSON (service_req_p);

			if (name_s || alias_s)
				{
					Service *service_p = GetServiceByName (grassroots_p, name_s, alias_s);

					if (service_p)
						{
							if (mode == RUN)
								{
									res = RunServiceFromJSON (grassroots_p, service_p, service_req_p, paired_servers_req_p, user_p, res_p);
								}		/* if (mode == RUN) */
							else if (mode == REFRESH)
								{
									res = RefreshServiceFromJSON (grassroots_p, service_p, service_req_p, paired_servers_req_p, user_p, res_p);
								}		/* if (mode == RUN) */
							else if (mode == INDEXING_DATA)
								{
									res = GetServiceIndexingInformation (grassroots_p, service_p, service_req_p, paired_servers_req_p, user_p, res_p);
								}
						}		/* if (service_p) */
					else
						{
							PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, service_req_p, "Failed to get service");
						}

				}


		}		/* if (mode != NONE) */
	else
		{
			PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__,  service_req_p, "Failed to get processing value from json");
		}

#if SERVER_DEBUG >= STM_LEVEL_FINE
	PrintJSONToLog (STM_LEVEL_FINE, __FILE__, __LINE__, res_p, "final result = ");
	FlushLog ();
#endif

#if SERVER_DEBUG >= STM_LEVEL_FINER
	PrintJSONRefCounts (STM_LEVEL_FINER, __FILE__, __LINE__, res_p, "final result: ");
#endif

	return res;
}


static int8 RefreshServiceFromJSON (GrassrootsServer *grassroots_p, Service *service_p, json_t *service_req_p, const json_t *paired_servers_req_p, UserDetails *user_p, json_t *res_p)
{
	int res = 0;
	const char *server_uri_s = GetServerProviderURI (grassroots_p);
	const char *service_name_s = GetServiceName (service_p);
	ProvidersStateTable *providers_p = GetInitialisedProvidersStateTableForSingleService (paired_servers_req_p, server_uri_s, service_name_s);

	if (providers_p)
		{
			AddPairedServices (grassroots_p, service_p, user_p, providers_p);

			DataResource *resource_p = AllocateDataResource (NULL, NULL, NULL);

			if (resource_p)
				{
					if (SetDataResourceData (resource_p, service_req_p, false))
						{
							json_t *service_json_p = GetServiceAsJSON (service_p, resource_p, user_p, false);

							if (service_json_p)
								{
									if (json_array_append_new (res_p, service_json_p) == 0)
										{
											res = 1;
										}
									else
										{
											PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, res_p, "Failed to add json_response for %s", service_name_s);
											json_decref (service_json_p);

											res = -1;
										}
								}		/* if (service_json_p) */
							else
								{
									PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, service_req_p, "GetServiceAsJSON %s", service_name_s);
								}

						}		/* if (SetResourceData (resource_p, service_req_p, false)) */

					FreeDataResource (resource_p);
				}		/* if (resource_p) */

			FreeProvidersStateTable (providers_p);
		}		/* if (providers_p) */


	FreeService (service_p);


	return res;
}




static int8 RunServiceFromJSON (GrassrootsServer *grassroots_p, Service *service_p, const json_t *service_req_p, const json_t *paired_servers_req_p, UserDetails *user_p, json_t *res_p)
{
	int res = 0;
	const char *server_uri_s = GetServerProviderURI (grassroots_p);
	const char *service_name_s = GetServiceName (service_p);
	ProvidersStateTable *providers_p = GetInitialisedProvidersStateTableForSingleService (paired_servers_req_p, server_uri_s, service_name_s);

	if (providers_p)
		{
			ParameterSet *params_p = NULL;
			bool delete_service_flag = true;

			AddPairedServices (grassroots_p, service_p, user_p, providers_p);

			/*
			 * If the service is asynchronous, let it
			 * take care of deleting itself rather
			 * than doing it after it has been run here.
			 */
			if (service_p -> se_synchronous != SY_SYNCHRONOUS)
				{
					delete_service_flag = false;
				}

			/*
			 * Convert the json parameter set into a ParameterSet
			 * to run the Service with.
			 */
			params_p = CreateParameterSetFromJSON (service_req_p, service_p, true);

			if (params_p)
				{
					ServiceJobSet *jobs_p = NULL;

#if SERVER_DEBUG >= STM_LEVEL_FINER
					PrintLog (STM_LEVEL_FINER, __FILE__, __LINE__, "about to run service \"%s\"\n", service_name_s);
#endif

					/*
					 * Now we reset the providers table back to its initial state
					 */
					if (ReinitProvidersStateTable (providers_p, paired_servers_req_p, server_uri_s, service_name_s))
						{
							if ((!IsServiceLockable (service_p)) || LockService (service_p))
								{
									jobs_p = RunService (service_p, params_p, user_p, providers_p);

									if (jobs_p)
										{
											if (ProcessServiceJobSet (jobs_p, res_p))
												{
													++ res;
												}

#if SERVER_DEBUG >= STM_LEVEL_FINER
											{
												PrintJSONToLog (res_p, "initial results", STM_LEVEL_FINER, __FILE__, __LINE__);
												FlushLog ();
												PrintJSONRefCounts (res_p, "initial results: ",  STM_LEVEL_FINER, __FILE__, __LINE__);
											}
#endif


										}		/* if (jobs_p) */
									else
										{
											PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, service_req_p, "No jobs from running %s", service_name_s);
										}

									/*
									 * If the Service is asynchronous, unlock it and from this point on it
									 * we can not assume anything about the current status of the Service
									 * as it's controlled and live in separate threads.
									 */
									if (! ((!IsServiceLockable (service_p)) || UnlockService (service_p)))
										{

										}
								}		/* if ((!IsServiceLockable ()) || LockService (service_p)) */
							else
								{
									PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__,  "Failed to lock Service %s", service_name_s);
								}


						}		/* if (ReinitProvidersStateTable (providers_p, req_p, server_uri_s, service_name_s)) */


					FreeParameterSet  (params_p);
				}		/* if (params_p) */
			else
				{
					PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, service_req_p, "Failed to get params");
				}


			if (delete_service_flag)
				{
					FreeService (service_p);
				}
			else
				{
					/*
					 * Let the service know that we have finished with it and it can
					 * delete itself when appropriate e.g. after its jobs have finished
					 * running.
					 */
					ReleaseService (service_p);
				}

			FreeProvidersStateTable (providers_p);
		}		/* if (providers_p) */

	return res;
}




static int8 GetServiceIndexingInformation (GrassrootsServer *grassroots_p, Service *service_p, const json_t *service_req_p, const json_t *paired_servers_req_p, UserDetails *user_p, json_t *res_p)
{
	int res = 0;
	const char *server_uri_s = GetServerProviderURI (grassroots_p);
	const char *service_name_s = GetServiceName (service_p);
	ProvidersStateTable *providers_p = GetInitialisedProvidersStateTableForSingleService (paired_servers_req_p, server_uri_s, service_name_s);

	if (providers_p)
		{
			ParameterSet *params_p = NULL;

			AddPairedServices (grassroots_p, service_p, user_p, providers_p);

			ServiceJobSet *jobs_p = AllocateSimpleServiceJobSet (service_p, "Indexing", "Indexing");

			if (jobs_p)
				{
#if SERVER_DEBUG >= STM_LEVEL_FINER
					PrintLog (STM_LEVEL_FINER, __FILE__, __LINE__, "about to index service \"%s\"\n", service_name_s);
#endif
					ServiceJob *job_p = GetServiceJobFromServiceJobSet (jobs_p, 0);

					/*
					 * Now we reset the providers table back to its initial state
					 */
					if (ReinitProvidersStateTable (providers_p, paired_servers_req_p, server_uri_s, service_name_s))
						{
							if ((!IsServiceLockable (service_p)) || LockService (service_p))
								{
									json_t *indexing_p = GetServiceIndexingData (service_p);

									if (indexing_p)
										{
											AddResultToServiceJob (job_p, indexing_p);

											if (ProcessServiceJobSet (jobs_p, res_p))
												{
													++ res;
												}

#if SERVER_DEBUG >= STM_LEVEL_FINER
											{
												PrintJSONToLog (res_p, "initial results", STM_LEVEL_FINER, __FILE__, __LINE__);
												FlushLog ();
												PrintJSONRefCounts (res_p, "initial results: ",  STM_LEVEL_FINER, __FILE__, __LINE__);
											}
#endif


										}		/* if (jobs_p) */
									else
										{
											PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, service_req_p, "No jobs from running %s", service_name_s);
										}

									/*
									 * If the Service is asynchronous, unlock it and from this point on it
									 * we can not assume anything about the current status of the Service
									 * as it's controlled and live in separate threads.
									 */
									if (! ((!IsServiceLockable (service_p)) || UnlockService (service_p)))
										{

										}
								}		/* if ((!IsServiceLockable ()) || LockService (service_p)) */
							else
								{
									PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__,  "Failed to lock Service %s", service_name_s);
								}


						}		/* if (ReinitProvidersStateTable (providers_p, req_p, server_uri_s, service_name_s)) */

				}		/* if (jobs_p) */

			FreeService (service_p);

			FreeProvidersStateTable (providers_p);
		}		/* if (providers_p) */

	return res;
}




static json_t *LoadConfig (const char *root_path_s, const char *config_filename_s)
{
	json_t *config_p = NULL;
	char *full_config_path_s = MakeFilename (root_path_s, config_filename_s ? config_filename_s : "grassroots.config");

	if (full_config_path_s)
		{
			json_error_t error;

			config_p = json_load_file (full_config_path_s, 0, &error);

			if (!config_p)
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to load config from %s", full_config_path_s);
				}

			FreeCopiedString (full_config_path_s);
		}

	return config_p;
}


static JobsManager *LoadJobsManagerFromConfig (GrassrootsServer *server_p)
{
	const char *manager_s = GetJSONString (server_p -> gs_config_p, JOBS_MANAGER_S);

	if (manager_s)
		{
			JobsManager *jobs_manager_p = LoadJobsManager (manager_s, server_p);

			if (jobs_manager_p)
				{
					return jobs_manager_p;
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to load jobs manager from %s", manager_s);
				}

		}		/* if (manager_s) */
	else
		{
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, server_p -> gs_config_p, "No jobs manager key, \"%s\" in config file", JOBS_MANAGER_S);
		}

	return NULL;
}


static ServersManager *LoadServersManagerFromConfig (GrassrootsServer *server_p)
{
	const char *manager_s = GetJSONString (server_p -> gs_config_p, SERVERS_MANAGER_S);

	if (manager_s)
		{
			ServersManager *manager_p = LoadServersManager (manager_s, server_p);

			if (manager_p)
				{
					return manager_p;
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to load jobs manager from %s", manager_s);
				}

		}		/* if (manager_s) */
	else
		{
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, server_p -> gs_config_p, "No servers manager key, \"%s\" in config file", SERVERS_MANAGER_S);
		}

	return NULL;
}


static SchemaVersion *InitSchemaVersionDetails (json_t *config_p)
{
	SchemaVersion *sv_p = NULL;
	uint32 major = CURRENT_SCHEMA_VERSION_MAJOR;
	uint32 minor = CURRENT_SCHEMA_VERSION_MINOR;
	const json_t *schema_p = json_object_get (config_p, SCHEMA_S);

	if (schema_p)
		{
			GetJSONUnsignedInteger (schema_p, VERSION_MAJOR_S, &major);
			GetJSONUnsignedInteger (schema_p, VERSION_MINOR_S, &minor);
		}

	sv_p = AllocateSchemaVersion (major, minor);

	if (!sv_p)
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "AllocateSchemaVersion failed for " UINT32_FMT " " UINT32_FMT, major, minor);
		}

	return sv_p;
}


static json_t *GetInterestedServices (GrassrootsServer *grassroots_p, const json_t * const req_p, UserDetails *user_p)
{
	json_t *res_p = NULL;
	DataResource *resource_p = GetResourceOfInterest (req_p);

	if (resource_p)
		{
			Handler *handler_p = GetResourceHandler (resource_p, grassroots_p, user_p);

			if (handler_p)
				{
					json_t *paired_servers_p = (req_p != NULL) ? json_object_get (req_p, SERVERS_S) : NULL;
					ProvidersStateTable *providers_p = AllocateProvidersStateTable (paired_servers_p);

					if (providers_p)
						{
							json_t *services_p = GetServicesAsJSON (grassroots_p, user_p, resource_p, handler_p, providers_p);

							if (services_p)
								{
									res_p = GetInitialisedResponseOnServer (grassroots_p, req_p, SERVICES_NAME_S, services_p);

									if (!res_p)
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "GetInitialisedResponse failed for adding services");

											json_decref (services_p);
										}
								}
							else
								{
									const SchemaVersion *sv_p = GetSchemaVersion (grassroots_p);
									res_p = GetInitialisedMessage (sv_p);

									if (!res_p)
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "GetInitialisedMessage failed");
										}

									PrintGrassrootsServer (grassroots_p);
								}

							FreeProvidersStateTable (providers_p);
						}

					FreeHandler (handler_p);
				}

			FreeDataResource (resource_p);
		}

	return res_p;
}


static DataResource *GetResourceFromRequest (const json_t *req_p)
{
	DataResource *resource_p = NULL;
	const json_t *resource_json_p = json_object_get (req_p, RESOURCE_S);

	if (resource_json_p)
		{
			resource_p = GetDataResourceFromJSON (resource_json_p);

			if (!resource_p)
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, resource_json_p, "Failed to create Resource from JSON");
				}
		}

	return resource_p;
}



static void PrintGrassrootsServer (const GrassrootsServer *grassroots_p)
{
	PrintLog (STM_LEVEL_INFO, __FILE__, __LINE__, "grassroots_p -> gs_path_s \"%s\"", grassroots_p -> gs_path_s);
	PrintLog (STM_LEVEL_INFO, __FILE__, __LINE__, "grassroots_p -> gs_config_filename_s \"%s\"", grassroots_p -> gs_config_filename_s);
	PrintLog (STM_LEVEL_INFO, __FILE__, __LINE__, "grassroots_p -> gs_config_path_s \"%s\"", grassroots_p -> gs_config_path_s);
	PrintLog (STM_LEVEL_INFO, __FILE__, __LINE__, "grassroots_p -> gs_services_path_s \"%s\"", grassroots_p -> gs_services_path_s);
}


static json_t *GetAllServices (GrassrootsServer *grassroots_p, const json_t * const req_p, UserDetails *user_p)
{
	json_t *paired_servers_p = (req_p != NULL) ? json_object_get (req_p, SERVERS_S) : NULL;
	ProvidersStateTable *providers_p = AllocateProvidersStateTable (paired_servers_p);

	if (providers_p)
		{
			DataResource *resource_p = GetResourceFromRequest (req_p);

			/* Get the local services */
			json_t *services_p = GetServicesAsJSON (grassroots_p, user_p, resource_p, NULL, providers_p);

			FreeProvidersStateTable (providers_p);

			if (services_p)
				{
					json_t *res_p = GetInitialisedResponseOnServer (grassroots_p, req_p, SERVICES_NAME_S, services_p);

					if (res_p)
						{
							return res_p;
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "GetInitialisedResponse failed for adding services");
						}

					json_decref (services_p);
				}
			else
				{
					PrintLog (STM_LEVEL_FINE, __FILE__, __LINE__, "GetServicesAsJSON returned no services");
					PrintGrassrootsServer (grassroots_p);
				}

			if (resource_p)
				{
					FreeDataResource (resource_p);
				}

		}		/* if (providers_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate ProvidersStateTable");
		}

	return NULL;
}


static json_t *RunKeywordServices (GrassrootsServer *grassroots_p, const json_t * const req_p, UserDetails *user_p, const char *keyword_s)
{
	json_t *res_p = NULL;
	json_t *results_p = json_array ();

	if (results_p)
		{
			res_p = GetInitialisedResponseOnServer (grassroots_p, req_p, SERVICE_RESULTS_S, results_p);

			if (res_p)
				{
					DataResource *resource_p = AllocateDataResource (PROTOCOL_TEXT_S, keyword_s, keyword_s);

					if (resource_p)
						{
							json_t *paired_servers_p = (req_p != NULL) ? json_object_get (req_p, SERVERS_S) : NULL;
							ProvidersStateTable *providers_p = AllocateProvidersStateTable (paired_servers_p);

							if (providers_p)
								{
									LinkedList *services_p = GetServicesList (grassroots_p, user_p, resource_p, NULL, providers_p);

									if (services_p)
										{
											ServiceMatcher *matcher_p = AllocateKeywordServiceMatcher ();

											if (matcher_p)
												{
													/* For each service, set its keyword parameter */
													ServiceNode *service_node_p = (ServiceNode *) (services_p -> ll_head_p);

													while (service_node_p)
														{
															Service *service_p = service_node_p -> sn_service_p;

															if (RunServiceMatcher (matcher_p, service_p))
																{
																	ParameterSet *params_p = NULL;
																	bool param_flag = true;

																	params_p = GetServiceParameters (service_p, NULL, user_p);

																	if (params_p)
																		{
																			ParameterNode *param_node_p = (ParameterNode *) params_p -> ps_params_p -> ll_head_p;

																			param_flag = false;

																			while (param_node_p)
																				{
																					Parameter *param_p = param_node_p -> pn_parameter_p;

																					/* set the keyword parameter */
																					if (param_p -> pa_type == PT_KEYWORD)
																						{
																							if (IsStringParameter (param_p))
																								{
																									if (SetStringParameterCurrentValue ((StringParameter *) param_p, keyword_s))
																										{
																											param_flag = true;
																										}
																									else
																										{
																											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to set service param \"%s\" - \"%s\" to \"%s\"", GetServiceName (service_p), param_p -> pa_name_s, keyword_s);
																											param_flag = false;
																										}
																								}
																							else
																								{
																									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Parameter is keyword but not a string param for service \"%s\" - \"%s\" to \"%s\"", GetServiceName (service_p), param_p -> pa_name_s, keyword_s);
																									param_flag = false;
																								}
																						}

																					param_node_p = (ParameterNode *) (param_node_p -> pn_node.ln_next_p);
																				}		/* while (param_node_p) */

																			/* Now run the service */
																			if (param_flag)
																				{
																					ServiceJobSet *jobs_set_p = RunService (service_p, params_p, user_p, providers_p);

																					if (jobs_set_p)
																						{
																							ProcessServiceJobSet (jobs_set_p, results_p);
																						}
																					else
																						{
																							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to run service \"%s\" with keyword \"%s\"", GetServiceName (service_p), keyword_s);
																						}
																				}

																			ReleaseServiceParameters (service_p, params_p);

																		}		/* if (params_p) */
																	else
																		{
																			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get parameters for service \"%s\" to run with keyword \"%s\"", GetServiceName (service_p), keyword_s);
																		}

																}		/* if (RunServiceMatcher (matcher_p, service_p)) */
															else
																{
																	ParameterSet *params_p = IsServiceMatch (service_p, resource_p, NULL);

																	/*
																	 * Does the service match for running against this keyword?
																	 */
																	if (params_p)
																		{
																			/*
																			 * Add the information that the service is interested in this keyword
																			 * and can be ran.
																			 */
																			json_t *interested_app_p = GetInterestedServiceJSON (service_p, keyword_s, params_p, true);

																			if (interested_app_p)
																				{
																					if (json_array_append_new (results_p, interested_app_p) != 0)
																						{
																							json_decref (interested_app_p);
																							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add interested service \"%s\" for keyword \"%s\" to results", GetServiceName (service_p), keyword_s);
																						}		/* if (json_array_append_new (res_p, interested_app_p) != 0) */

																				}		/* if (interested_app_p) */
																			else
																				{
																					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create JSON for interested service \"%s\" for keyword \"%s\" to results", GetServiceName (service_p), keyword_s);
																				}

																			ReleaseServiceParameters (service_p, params_p);
																		}		/* if (params_p) */

																}		/* if (RunServiceMatcher (matcher_p, service_p)) else */

															service_node_p = (ServiceNode *) (service_node_p -> sn_node.ln_next_p);
														}		/* while (service_node_p) */

													FreeServiceMatcher (matcher_p);
												}		/* if (matcher_p) */

											FreeLinkedList (services_p);
										}		/* if (services_p) */

									FreeProvidersStateTable (providers_p);
								}		/* if (providers_p) */

							if (user_p)
								{
									FreeUserDetails (user_p);
								}

							FreeDataResource (resource_p);
						}		/* if (resource_p) */

				}		/* if (res_p) */

		}		/* if (results_p) */

	return res_p;
}


/*
static json_t *GetNamedServices (GrassrootsServer *grassroots_p, const json_t * const req_p, UserDetails *user_p)
{
	return GetNamedServicesFunctionality (grassroots_p, req_p, user_p);
}


static json_t *GetNamedServiceInfo (GrassrootsServer *grassroots_p, const json_t * const req_p, UserDetails *user_p)
{
	return GetNamedServicesFunctionality (grassroots_p, req_p, user_p);
}
 */

static json_t *GetNamedServicesFunctionality (GrassrootsServer *grassroots_p, const json_t * const req_p, UserDetails *user_p, Operation op)
{
	json_t *res_p = NULL;
	json_t *services_req_p = json_object_get (req_p, SERVICES_NAME_S);

	if (services_req_p)
		{
			ProvidersStateTable *providers_p = AllocateProvidersStateTable (req_p);

			if (providers_p)
				{
					const char *server_s = GetServerProviderURI (grassroots_p);
					json_t *services_res_p = json_array ();

					res_p = GetInitialisedResponseOnServer (grassroots_p, req_p, SERVICES_NAME_S, services_res_p);

					if (res_p)
						{

							if (json_is_array (services_req_p))
								{
									size_t i;
									const size_t num_requested_services = json_array_size (services_req_p);

									/*@TODO
									 * This is inefficient and would be better to loop through in
									 * a LoadServices.... method passing in an array of service names
									 */
									for (i = 0; i < num_requested_services; ++ i)
										{
											json_t *service_req_p = json_array_get (services_req_p, i);

											ProcessServiceRequest (service_req_p, services_res_p, grassroots_p, providers_p, server_s, user_p, op);
										}		/* for (i = 0; i < num_requested_services; ++ i) */
								}
							else
								{
									ProcessServiceRequest (services_req_p, services_res_p, grassroots_p, providers_p, server_s, user_p, op);
								}

						}

					FreeProvidersStateTable (providers_p);
				}		/* if (providers_p) */

		}		/* if (services_req_p) */
	else
		{
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, req_p, "No \"%s\" key", SERVICES_NAME_S);
		}

	return res_p;
}



static void ProcessServiceRequest (const json_t *service_req_p, json_t *services_res_p, GrassrootsServer *grassroots_p, ProvidersStateTable *providers_p, const char *server_s, UserDetails *user_p, Operation op)
{
	const char *service_name_s = GetJSONString (service_req_p, SERVICE_NAME_S);
	const char *service_alias_s = GetJSONString (service_req_p, SERVICE_ALIAS_S);

	if (service_name_s || service_alias_s)
		{
			DataResource *resource_p = GetResourceFromRequest (service_req_p);
			Service *service_p = GetServiceByName (grassroots_p, service_name_s, service_alias_s);

			if (service_p)
				{
					AddPairedServices (grassroots_p, service_p, user_p, providers_p);

					/*
					 * If the Service was matched from only the alias, then the name can be NULL
					 * so make sure we have it.
					 */
					if (!service_name_s)
						{
							service_name_s = GetServiceName (service_p);
						}

					if (AddToProvidersStateTable (providers_p, server_s, service_name_s))
						{
							json_t *service_res_p = NULL;

							if (op == OP_GET_NAMED_SERVICES)
								{
									service_res_p = GetServiceAsJSON (service_p, resource_p, user_p, false);
								}
							else if (op == OP_GET_SERVICE_INFO)
								{
									service_res_p = GetServiceIndexingData (service_p);
								}

							if (service_res_p)
								{
									if (json_array_append_new (services_res_p, service_res_p) != 0)
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, service_res_p, "Failed to add JSON for service \"%s\"", service_name_s);
											json_decref (service_res_p);
										}
								}
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get Service JSON for service \"%s\"", service_name_s);
								}

						}

					FreeService (service_p);
				}		/* if (service_p) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to load service \"%s\"", service_name_s);
				}

			if (resource_p)
				{
					FreeDataResource (resource_p);
				}
		}
	else
		{
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, service_req_p, "Failed to load service \"%s\"", service_name_s);
		}
}


static json_t *GetServicesAsJSON (GrassrootsServer *grassroots_p, UserDetails *user_p, DataResource *resource_p, Handler *handler_p, ProvidersStateTable *providers_p)
{
	json_t *json_p = NULL;
	LinkedList *services_p = GetServicesList (grassroots_p, user_p, resource_p, handler_p, providers_p);

	if (services_p)
		{
			json_p = GetServicesListAsJSON (services_p, resource_p, user_p, false, providers_p);
			FreeLinkedList (services_p);
		}
	else
		{
			ServersManager *servers_manager_p = GetServersManager (grassroots_p);

			PrintLog (STM_LEVEL_WARNING, __FILE__, __LINE__, "GetServicesList () returned no services");

			if (servers_manager_p)
				{
					json_p = AddExternalServerOperationsToJSON (servers_manager_p, OP_LIST_ALL_SERVICES);
				}
		}


	return json_p;
}


static LinkedList *GetServicesList (GrassrootsServer *grassroots_p, UserDetails *user_p, DataResource *resource_p, Handler *handler_p, ProvidersStateTable *providers_p)
{
	LinkedList *services_p = AllocateLinkedList (FreeServiceNode);

	if (services_p)
		{
			LoadMatchingServices (grassroots_p, services_p, resource_p, handler_p, user_p);

			if (services_p -> ll_size > 0)
				{
					if (AddServicesListToProvidersStateTable (providers_p, services_p, grassroots_p))
						{
							AddAllPairedServices (grassroots_p, services_p, user_p, providers_p);

							SortServicesListByName (services_p);
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "AddServicesListToProvidersStateTable failed");
						}

					return services_p;
				}
			else
				{
					PrintLog (STM_LEVEL_WARNING, __FILE__, __LINE__, "LoadMatchingServices () didn't find any services");
				}

			FreeLinkedList (services_p);
		}		/* if (services_p) */
	else
		{
			PrintLog(STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to allocate services list");
		}

	return NULL;
}




static int32 AddAllPairedServices (GrassrootsServer *grassroots_p, LinkedList *internal_services_p, UserDetails *user_p, ProvidersStateTable *providers_p)
{
	int32 num_added_services = 0;

	if (internal_services_p)
		{
			ServiceNode *internal_service_node_p = (ServiceNode *) (internal_services_p -> ll_head_p);

			/*
			 * Loop through our internal services trying to find a match
			 */
			while (internal_service_node_p)
				{
					int32 i = AddPairedServices (grassroots_p, internal_service_node_p -> sn_service_p, user_p, providers_p);

					num_added_services += i;
					internal_service_node_p = (ServiceNode *) (internal_service_node_p -> sn_node.ln_next_p);
				}		/* while (internal_service_node_p) */

		}		/* if (internal_services_p) */

	return num_added_services;
}



static json_t *GetServiceResultsAsJSON (GrassrootsServer *grassroots_p, const json_t * const req_p, UserDetails *user_p)
{
	return GetServiceData (grassroots_p, req_p, user_p, AddServiceResultsToJSON);
}



static bool AddServiceStatusToJSON (GrassrootsServer *grassroots_p, json_t *results_p, uuid_t job_id, const char *uuid_s)
{
	return AddServiceDataToJSON (grassroots_p, results_p, job_id, uuid_s, "status", GetServiceJobStatusAsJSON);
}


static bool AddServiceResultsToJSON (GrassrootsServer *grassroots_p, json_t *results_p, uuid_t job_id, const char *uuid_s)
{
	return AddServiceDataToJSON (grassroots_p, results_p, job_id, uuid_s, "results", GetServiceJobAsJSON);
}



static bool AddServiceDataToJSON (GrassrootsServer *grassroots_p, json_t *results_p, uuid_t job_id, const char *uuid_s, const char * const identifier_s, json_t *(*get_job_json_fn) (ServiceJob *job_p, bool omit_results_flag))
{
	bool success_flag = false;
	JobsManager *manager_p = grassroots_p -> gs_jobs_manager_p;
	ServiceJob *job_p = GetServiceJobFromJobsManager (manager_p, job_id);
	json_t *job_json_p = NULL;

	if (job_p)
		{
			OperationStatus old_status = GetCachedServiceJobStatus (job_p);
			OperationStatus current_status = GetServiceJobStatus (job_p);
			Service *service_p = job_p -> sj_service_p;
			int32 num_live_jobs = 0;

			/* Has the ServiceJob changed its status since the last check? */
			//			if (current_status != old_status)
			{
				switch (current_status)
				{

					/*
					 * If the job has finished, then remove it from the jobs manager.
					 */
					case OS_FINISHED:
					case OS_SUCCEEDED:
					case OS_PARTIALLY_SUCCEEDED:
						{
							//RemoveServiceJobFromJobsManager (manager_p, job_id, false);

							if (! (job_p -> sj_result_p))
								{
									if (! (CalculateServiceJobResult (job_p)))
										{
											char job_uuid_s [UUID_STRING_BUFFER_SIZE];

											ConvertUUIDToString (job_id, job_uuid_s);
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to calculate results for job \"%s\"", job_uuid_s);
										}
								}
						}
						break;


					case OS_ERROR:
					case OS_FAILED_TO_START:
					case OS_FAILED:
						{
							if (current_status != old_status)
								{
									RemoveServiceJobFromJobsManager (manager_p, job_id, false);
								}
						}
						break;

						/*
						 * If the job has updated its status but not yet finished running,
						 * then update the value stored in the jobs manager
						 */
					case OS_PENDING:
					case OS_STARTED:
						{
							if (current_status != old_status)
								{
									if (!AddServiceJobToJobsManager (manager_p, job_id, job_p))
										{
											char job_uuid_s [UUID_STRING_BUFFER_SIZE];

											ConvertUUIDToString (job_id, job_uuid_s);
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to update job %s from status %d to %d", job_uuid_s, old_status, current_status);
										}
								}
						}
						break;

					default:
						break;
				}		/* switch (current_status) */

			}		/* if (current_status != old_status) */


			job_json_p = get_job_json_fn (job_p, false);

			if (!job_json_p)
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get job %s for \"%s\" %s", identifier_s, job_p -> sj_name_s ? job_p -> sj_name_s : "", uuid_s);
				}

			num_live_jobs = 0; //GetNumberOfLiveJobs (service_p);


#if SERVER_DEBUG >= STM_LEVEL_FINE
			PrintLog (STM_LEVEL_FINE, __FILE__, __LINE__, "num_live_jobs = " INT32_FMT " for %s", num_live_jobs, GetServiceName (service_p));
#endif

			if (num_live_jobs == 0)
				{
					FreeService (service_p);
				}
			else
				{
					if (num_live_jobs < 0)
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get number of live jobs for \"%s\"", GetServiceName (service_p));
						}

					FreeServiceJob (job_p);
				}

		}		/* if (job_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to deserialise job %s from services table", uuid_s);

			job_json_p = json_pack ("{s:s,s:s}", JOB_UUID_S, uuid_s, ERROR_S, "Failed to deserialise job from services table");

			if (!job_json_p)
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get create error json for %s", uuid_s);
				}
		}

	if (job_json_p)
		{
			if (json_array_append_new (results_p, job_json_p) == 0)
				{
					success_flag = true;
				}
			else
				{
					json_decref (job_json_p);
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, job_json_p, "Failed to add job %s for %s", identifier_s, uuid_s);
				}
		}

	return success_flag;
}





static json_t *GetServiceData (GrassrootsServer *grassroots_p, const json_t * const req_p, UserDetails *user_p, bool (*callback_fn) (GrassrootsServer *grassroots_p, json_t *services_p, uuid_t service_id, const char *uuid_s))
{
	json_t *results_array_p = json_array ();

	if (results_array_p)
		{
			json_t *service_uuids_json_p = json_object_get (req_p, SERVICES_NAME_S);

			if (service_uuids_json_p)
				{
					if (json_is_array (service_uuids_json_p))
						{
							size_t i;
							json_t *service_uuid_json_p;
							size_t num_successes = 0;
							size_t num_uuids = json_array_size (service_uuids_json_p);

							json_array_foreach (service_uuids_json_p, i, service_uuid_json_p)
							{
								if (json_is_string (service_uuid_json_p))
									{
										const char *uuid_s = json_string_value (service_uuid_json_p);
										uuid_t service_id = { 0 };

										if (ConvertStringToUUID (uuid_s, service_id))
											{
												if (callback_fn (grassroots_p, results_array_p, service_id, uuid_s))
													{
														++ num_successes;
													}
												else
													{
														PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to add callback json results for \"%s\"", uuid_s);
													}

											}		/* if (ConvertStringToUUID (uuid_s, service_id)) */
										else
											{
												PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to convert \"%s\" to uuid", uuid_s);
											}

									}		/* if (json_is_string (service_uuid_json_p)) */
								else
									{
										PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "service_uuid_json_p is %d not a string", json_typeof (service_uuid_json_p));
									}

							}		/* json_array_foreach (service_uuids_json_p, i, service_uuid_json_p) */


							if (num_uuids == num_successes)
								{
									//CloseService (job_p -> sj_service_p);
								}

						}		/* if (json_is_array (service_uuids_json_p)) */
					else
						{
							PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "service_uuids_json_p is %d not an array", json_typeof (service_uuids_json_p));
						}

				}		/* if (service_uuids_json_p) */

		}		/* if (results_array_p) */

	return results_array_p;
}



static json_t *GetServerStatus (GrassrootsServer *grassroots_p, const json_t * const req_p, UserDetails *user_p)
{
	json_t *res_p = json_object ();

	if (res_p)
		{
			JobsManager *jobs_manager_p = GetJobsManager (grassroots_p);

			if (jobs_manager_p)
				{
					LinkedList *jobs_list_p = GetAllServiceJobsFromJobsManager (jobs_manager_p);

					if (jobs_list_p)
						{
							if (jobs_list_p -> ll_size > 0)
								{
									json_t *jobs_array_p = json_array ();

									if (jobs_array_p)
										{
											ServiceJobNode *job_node_p = (ServiceJobNode *) (jobs_list_p -> ll_head_p);
											bool success_flag = true;

											while ((success_flag == true) && (job_node_p != NULL))
												{
													json_t *job_json_p = GetServiceJobAsJSON (job_node_p -> sjn_job_p, true);

													if (job_json_p)
														{
															if (json_array_append_new (jobs_array_p, job_json_p) == 0)
																{
																	job_node_p = (ServiceJobNode *) (job_node_p -> sjn_node.ln_next_p);
																}
															else
																{
																	success_flag = false;
																	json_decref (job_json_p);
																}

														}		/* if (job_json_p) */
													else
														{
															success_flag = false;
														}

												}		/* while ((success_flag == true) && (job_node_p != NULL)) */

											if (success_flag)
												{
													if (json_object_set (res_p, SERVICE_JOBS_S, jobs_array_p) != 0)
														{
															success_flag = false;
															json_decref (jobs_array_p);
														}
												}
											else
												{
													json_decref (jobs_array_p);
												}

										}		/* if (jobs_array_p) */

								}		/* if (jobs_list_p -> ll_size > 0) */

							FreeLinkedList (jobs_list_p);
						}		/* if (jobs_list_p) */

				}		/* if (jobs_manager_p) */

		}		/* if (res_p) */



	return res_p;
}



static json_t *GetRequestedResource (GrassrootsServer *grassroots_p, const json_t * const req_p, UserDetails *user_p)
{
	json_t *res_p = NULL;

	return res_p;
}



static json_t *GenerateNamedServices (GrassrootsServer *grassroots_p, LinkedList *services_p, const json_t * const req_p, UserDetails *user_p, ProvidersStateTable *providers_p)
{
	json_t *res_p = NULL;
	json_t *services_json_p = NULL;

	//GetUsernameAndPassword (credentials_p, &username_s, &password_s);
	AddAllPairedServices (grassroots_p, services_p, user_p, providers_p);

	services_json_p = GetServicesListAsJSON (services_p, NULL, user_p, false, providers_p);

	if (services_json_p)
		{
			res_p = GetInitialisedResponseOnServer (grassroots_p, req_p, SERVICES_NAME_S, services_json_p);

			if (!res_p)
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__,  "Failed to create response for the services array");
					json_decref (services_json_p);
				}
		}

	return res_p;
}


static int32 AddPairedServices (GrassrootsServer *grassroots_p, Service *internal_service_p, UserDetails *user_p, ProvidersStateTable *providers_p)
{
	int32 num_added_services = 0;
	ServersManager *servers_manager_p = GetServersManager (grassroots_p);

	if (servers_manager_p)
		{
			LinkedList *external_servers_p = GetAllExternalServersFromServersManager (servers_manager_p, DeserialiseExternalServerFromJSON);

			if (external_servers_p)
				{
					const SchemaVersion *sv_p = GetSchemaVersion (grassroots_p);
					const char *internal_service_name_s = GetServiceName (internal_service_p);
					ExternalServerNode *external_server_node_p = (ExternalServerNode *) (external_servers_p -> ll_head_p);

					while (external_server_node_p)
						{
							ExternalServer *external_server_p = external_server_node_p -> esn_server_p;

							/* If it has paired services try and match them up */
							if (external_server_p -> es_paired_services_p)
								{
									KeyValuePairNode *pairs_node_p = (KeyValuePairNode *) (external_server_p -> es_paired_services_p -> ll_head_p);

									while (pairs_node_p)
										{
											const char *external_service_name_s = pairs_node_p -> kvpn_pair_p -> kvp_key_s;

											if (strcmp (external_service_name_s, internal_service_name_s) == 0)
												{
													if (!IsServiceInProvidersStateTable (providers_p, external_server_p -> es_uri_s, external_service_name_s))
														{
															const char *service_name_s = pairs_node_p -> kvpn_pair_p -> kvp_value_s;
															json_t *req_p = GetAvailableServicesRequestForAllProviders (providers_p, user_p, sv_p);

															/* We don't need to loop after this iteration */
															pairs_node_p = NULL;

															if (req_p)
																{
																	json_t *response_p = MakeRemoteJSONCallToExternalServer (external_server_p, req_p);

																	if (response_p)
																		{
																			json_t *services_p = json_object_get (response_p, SERVICES_NAME_S);

																			if (services_p)
																				{
																					/*
																					 * Get the required Service from the ExternalServer
																					 */
																					if (json_is_array (services_p))
																						{
																							const size_t size = json_array_size (services_p);
																							size_t i;

																							for (i = 0; i < size; ++ i)
																								{
																									json_t *service_response_p = json_array_get (services_p, i);

																									/* Do we have our remote service definition? */
																									if (IsRequiredExternalOperation (service_response_p, service_name_s))
																										{
																											/*
																											 * Merge the external service with our own and
																											 * if successful, then remove the external one
																											 * from the json array
																											 */
																											json_t *op_p = json_object_get (service_response_p, OPERATION_S);

																											if (op_p)
																												{
																													const json_t *provider_p = GetProviderDetails (service_response_p);

																													if (provider_p)
																														{
																															if (json_is_object (provider_p))
																																{
																																	if (CreateAndAddPairedService (internal_service_p, external_server_p, service_name_s, op_p, provider_p))
																																		{
																																			++ num_added_services;

																																			if (!AddToProvidersStateTable (providers_p, external_server_p -> es_uri_s, external_service_name_s))
																																				{
																																					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add external service %s:%s to providers table", external_server_p -> es_name_s, external_service_name_s);
																																				}

																																		}		/* if (CreateAndAddPairedService (matching_internal_service_p, external_server_p, matching_external_op_p)) */

																																}		/* if (json_is_object (provider_p)) */
																															else if (json_is_array (provider_p))
																																{

																																}
																															else
																																{

																																}

																														}		/* if (provider_p) */

																												}
																											else
																												{
																													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, service_response_p, "No \"%s\" key in service response", OPERATION_S);
																												}

																											i = size;		/* force exit from loop */
																										}

																								}		/* for (i = 0; i < size; ++ i) */

																						}		/* if (json_is_array (services_p)) */
																					else
																						{
																							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, services_p, "services is not a json array");
																						}

																				}		/* if (services_p) */
																			else
																				{
																					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, response_p, "Failed to get services from response");
																				}

																			json_decref (response_p);
																		}		/* if (response_p) */
																	else
																		{
																			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get response from %s at %s", external_server_p -> es_name_s, external_server_p -> es_uri_s);
																		}

																	json_decref (req_p);
																}		/* if (req_p) */
															else
																{
																	PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to build request for %s at %s", external_server_p -> es_name_s, external_server_p -> es_uri_s);
																}

														}		/* if (!IsServiceInProvidersStateTable (providers_p, external_server_p -> es_name_s, external_service_name_s)) */

												}		/* if (strcmp (service_name_s, internal_service_name_s) == 0) */

											if (pairs_node_p)
												{
													pairs_node_p = (KeyValuePairNode *) (pairs_node_p -> kvpn_node.ln_next_p);
												}

										}		/* while (pairs_node_p) */

								}		/* if (external_server_p -> es_paired_services_p) */

							external_server_node_p = (ExternalServerNode *) external_server_node_p -> esn_node.ln_next_p;
						}		/* (while (external_server_node_p) */

					FreeLinkedList (external_servers_p);
				}		/* if (external_servers_p) */

		}		/* if (servers_manager_p) */


	return num_added_services;
}


static json_t *GenerateServiceIndexingData (GrassrootsServer *grassroots_p, LinkedList *services_p, const json_t * const req_p, UserDetails *user_p, ProvidersStateTable *providers_p)
{
	if (services_p && (services_p -> ll_size > 0))
		{
			json_t *services_list_json_p = json_array ();

			if (services_list_json_p)
				{
					bool success_flag = true;
					ServiceNode *node_p = (ServiceNode *) (services_p -> ll_head_p);

					while (node_p && success_flag)
						{
							Service *service_p = node_p -> sn_service_p;
							json_t *res_p = GetServiceIndexingData (service_p);

							if (res_p)
								{
									if (json_array_append_new (services_list_json_p, res_p) != 0)
										{
											json_decref (res_p);
											success_flag = false;
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, res_p, "Failed to add to services array");
										}
								}		/* if (res_p) */
							else
								{
									success_flag = false;
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "GetServiceIndexingDataAsJSON failed for \"%s\"", GetServiceName (service_p));
								}

							node_p = (ServiceNode *) node_p -> sn_node.ln_next_p;
						}		/* while (node_p) */

					/*
					 * Have we added all of the services correctly?
					 */
					if (success_flag)
						{
							return services_list_json_p;
						}		/* if (success_flag) */

					json_decref (services_list_json_p);
				}		/* if (services_list_json_p) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate JSON array for services");
				}

		}		/* if (services_p && (services_p -> ll_size > 0)) */

	return NULL;
}


static DataResource *GetResourceOfInterest (const json_t * const req_p)
{
	DataResource *resource_p = NULL;
	json_t *file_data_p = json_object_get (req_p, RESOURCE_S);

	if (file_data_p)
		{
			json_t *protocol_p = json_object_get (file_data_p, RESOURCE_PROTOCOL_S);

			if (protocol_p)
				{
					if (json_is_string (protocol_p))
						{
							json_t *data_name_p = json_object_get (file_data_p, RESOURCE_VALUE_S);

							if (data_name_p && (json_is_string (data_name_p)))
								{
									const char *protocol_s = json_string_value (protocol_p);
									const char *data_name_s = json_string_value (data_name_p);

									resource_p = AllocateDataResource (protocol_s, data_name_s, NULL);
								}

						}		/* if (json_is_string (protocol_p)) */

				}		/* if (protocol_p) */

		}		/* if (file_data_p) */

	return resource_p;
}


static bool IsRequiredExternalOperation (const json_t *external_op_p, const char *op_name_s)
{
	bool match_flag = false;
	const char *external_op_s = GetOperationNameFromJSON (external_op_p);

	if (external_op_s)
		{
			match_flag = (strcmp (external_op_s, op_name_s) == 0);
		}

	return match_flag;
}



#if IRODS_ENABLED == 1
static json_t *GetAllModifiedData (const json_t * const req_p, UserDetails *user_p)
{
	json_t *res_p = NULL;
	const char *username_s = NULL;
	const char *password_s = NULL;
	const char *token_s = NULL;
	const UserAuthentication *user_auth_p = GetUserAuthenticationForSystem (user_p, PROTOCOL_IRODS_S);


	if (user_auth_p)
		{
			const char *from_s = NULL;
			const char *to_s = NULL;
			/* "from" defaults to the start of time */
			time_t from = 0;

			/* "to" defaults to now */
			time_t to = time (NULL);

			json_t *group_p = json_object_get (req_p, "");

			if (group_p)
				{
					json_t *interval_p = json_object_get (group_p, "");

					if (interval_p)
						{
							from_s = GetJSONString (interval_p, "from");
							to_s = GetJSONString (interval_p, "to");
						}
				}

			if (from_s)
				{
					if (!ConvertCompactStringToEpochTime (from_s, &from))
						{
							// error
						}
				}

			if (to_s)
				{
					if (!ConvertCompactStringToEpochTime (to_s, &to))
						{
							// error
						}
				}

			res_p = GetModifiedIRodsFiles (username_s, password_s, from, to);
		}

	return res_p;
}
#endif



static struct MongoClientManager *GetMongoClientManager (const json_t *config_p)
{
	const json_t *mongo_config_p = json_object_get (config_p, "mongodb");

	if (mongo_config_p)
		{
			const char *uri_s = GetJSONString (mongo_config_p, "uri");

			if (uri_s)
				{
					struct MongoClientManager *manager_p = AllocateMongoClientManager (uri_s);

					if (manager_p)
						{
							return manager_p;
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create MongoClientManager for %s", uri_s);
						}
				}
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, mongo_config_p, "No uri in config");
				}

		}
	else
		{
			PrintLog (STM_LEVEL_INFO, __FILE__, __LINE__, "No mongodb in config");
		}


	return NULL;
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
			char *full_services_path_s = MakeFilename (root_path_s, grassroots_p -> gs_services_path_s);

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

#if GRASSROOTS_SERVE_DEBUG >= STM_LEVEL_FINEST
											PrintLog (STM_LEVEL_FINEST, __FILE__, __LINE__, "AllocatePlugin () for \"%s\"", node_p -> sln_string_s);
#endif

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
																	PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to get services from \"%s\"", node_p -> sln_string_s);
																}

														}		/* if (OpenPlugin (plugin_p)) */
													else
														{
															PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "OpenPlugin () failed for \"%s\"", node_p -> sln_string_s);
														}

													if (plugin_p && (!using_plugin_flag))
														{
															FreePlugin (plugin_p);
														}


												}		/* if (plugin_p) */
											else
												{
													PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to allocated plugin for \"%s\"", node_p -> sln_string_s);
												}

											if (node_p)
												{
													node_p = (StringListNode *) (node_p -> sln_node.ln_next_p);
												}
										}		/* while (node_p) */

									FreeLinkedList (matching_filenames_p);
								}		/* if (matching_filenames_p) */
							else
								{
									PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "No matching filenames for \"%s\"", path_and_pattern_s);
								}

							FreeCopiedString (path_and_pattern_s);
						}		/* if (path_and_pattern_s) */
					else
						{
							PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, " MakeFilename () failed for \"%s\" and \"%s\"", full_services_path_s, plugin_pattern_s);
						}

					FreeCopiedString (full_services_path_s);
				}		/* if (full_services_path_s) */
			else
				{
					PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, " MakeFilename () failed for \"%s\" and \"%s\"", root_path_s, grassroots_p -> gs_services_path_s);
				}

		}		/* if (plugin_pattern_s) */
	else
		{
			PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to get plugin pattern");
		}

	return num_matched_services;
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


static const char *GetPluginNameFromJSON (const json_t *const root_p)
{
	return GetJSONString(root_p, PLUGIN_NAME_S);
}
