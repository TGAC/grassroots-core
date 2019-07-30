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


#include "grassroots_server.h"
#include "memory_allocations.h"
#include "streams.h"


/*
 * STATIC DECLARATIONS
 */

static json_t *LoadConfig (const char *root_path_s, const char *config_filename_s);

static JobsManager *LoadJobsManagerFromConfig (const json_t *config_p);

static ServersManager *LoadServersManagerFromConfig (const json_t *config_p);

static SchemaVersion *InitSchemaVersionDetails (json_t *config_p);

/*
 * API DEFINITIONS
 */

GrassrootsServer *AllocateGrassrootsServer (const char *grassroots_path_s, const char *config_filename_s, JobsManager *external_jobs_manager_p, MEM_FLAG jobs_manager_flag, ServersManager *external_servers_manager_p, MEM_FLAG servers_manager_flag)
{
	char *copied_path_s = EasyCopyToNewString (grassroots_path_s);

	if (copied_path_s)
		{
			char *copied_config_filename_s = EasyCopyToNewString (config_filename_s ? config_filename_s: "grassroots.config");

			if (copied_config_filename_s)
				{
					json_t *config_p = LoadConfig (copied_path_s, copied_config_filename_s);

					if (config_p)
						{
							JobsManager *jobs_manager_p = NULL;

							/*
							 * Load the jobs manager
							 */
							if (external_jobs_manager_p)
								{
									jobs_manager_p = external_jobs_manager_p;
								}
							else
								{
									jobs_manager_p = LoadJobsManagerFromConfig (config_p);
									jobs_manager_flag = MF_SHALLOW_COPY;
								}

							if (jobs_manager_p)
								{
									ServersManager *servers_manager_p = NULL;

									/*
									 * Load the servers manager
									 */
									if (external_servers_manager_p)
										{
											servers_manager_p = external_servers_manager_p;
										}
									else
										{
											servers_manager_p = LoadServersManagerFromConfig (config_p);
											servers_manager_flag = MF_SHALLOW_COPY;
										}


									if (servers_manager_p)
										{

											SchemaVersion *sv_p = InitSchemaVersionDetails (config_p);

											if (sv_p)
												{
													GrassrootsServer *server_p = (GrassrootsServer *) AllocMemory (sizeof (GrassrootsServer));

													if (server_p)
														{
															server_p -> gs_path_s = copied_path_s;
															server_p -> gs_config_filename_s = copied_config_filename_s;
															server_p -> gs_config_p = config_p;

															server_p -> gs_jobs_manager_p = jobs_manager_p;
															server_p -> gs_job_manager_mem = jobs_manager_flag;

															server_p -> gs_servers_manager_p = servers_manager_p;
															server_p -> gs_servers_manager_mem = servers_manager_flag;

															server_p -> gs_schema_version_p = sv_p;

															return server_p;
														}		/* if (server_p) */

													FreeSchemaVersion (sv_p);
												}		/* if (sv_p) */


											if (servers_manager_p != external_servers_manager_p)
												{
													FreeServersManager (servers_manager_p);
												}

										}		/* if (servers_manager_p) */


									if (jobs_manager_p != external_jobs_manager_p)
										{
											FreeJobsManager (jobs_manager_p);
										}
								}		/* if (jobs_manager_p) */

							json_decref (config_p);
						}		/* if (config_p) */

					FreeCopiedString (copied_config_filename_s);
				}		/* if (copied_config_filename_s) */


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
	FreeCopiedString (server_p -> gs_path_s);

	if (server_p -> gs_jobs_manager_p)
		{
			switch (server_p -> gs_job_manager_mem)
				{
					case MF_SHALLOW_COPY:
					case MF_DEEP_COPY:
						FreeJobsManager (server_p -> gs_jobs_manager_p);
						break;

					default:
						break;
				}
		}


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

	FreeMemory (server_p);
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


json_t *ProcessServerJSONMessage (GrassrootsServer *server_p, json_t *req_p, const char **error_ss)
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
									uuid_t key;

									if (ConvertStringToUUID (uuid_s, key))
										{
											ServersManager *manager_p = GetServersManager (server_p);

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
										res_p = GetAllServices (server_p, req_p, user_p);
										break;

//									case OP_IRODS_MODIFIED_DATA:
//										{
//											#if IRODS_ENABLED == 1
//											res_p = GetAllModifiedData (req_p, user_p);
//											#endif
//										}
//										break;

									case OP_LIST_INTERESTED_SERVICES:
										res_p = GetInterestedServices (server_p, req_p, user_p);
										break;

									case OP_RUN_KEYWORD_SERVICES:
										{
											json_t *keyword_json_group_p = json_object_get (req_p, KEYWORDS_QUERY_S);

											if (keyword_json_group_p)
												{
													const char *keyword_s = GetJSONString (keyword_json_group_p, KEYWORDS_QUERY_S);

													if (keyword_s)
														{
															res_p = RunKeywordServices (server_p, req_p, user_p, keyword_s);
														}
													else
														{
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, keyword_json_group_p, "Failed to get query keyword");
														}
												}
											else
												{
													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, req_p, "Failed to get query group");
												}
										}
										break;

									case OP_GET_NAMED_SERVICES:
										res_p = GetNamedServices (server_p, req_p, user_p);
										break;

									case OP_GET_SERVICE_RESULTS:
										res_p = GetServiceResultsAsJSON (server_p, req_p, user_p);
										break;

									case OP_GET_RESOURCE:
										res_p = GetRequestedResource (server_p, req_p, user_p);
										break;

									case OP_SERVER_STATUS:
										res_p = GetServerStatus (server_p, req_p, user_p);
										break;

									case OP_GET_SERVICE_INFO:
										res_p = GetNamedServiceInfo (server_p, req_p, user_p);
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
									uuid_t user_uuid;

									uuid_clear (user_uuid);

									res_p = GetInitialisedResponseOnServer (req_p, SERVICE_RESULTS_S, service_results_p);

									if (res_p)
										{
											const json_t *external_servers_req_p = json_object_get (req_p, SERVERS_S);

											if (json_is_array (op_p))
												{
													size_t i;
													json_t *value_p;

													json_array_foreach (op_p, i, value_p)
														{
															int8 res = RunServiceFromJSON (value_p, external_servers_req_p, user_p, service_results_p, user_uuid);

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
													int8 res = RunServiceFromJSON (op_p, external_servers_req_p, user_p, service_results_p, user_uuid);

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

										}		/* if (res_p) */
									else
										{
											*error_ss = "Failed to create service response";
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create response");
										}

								}		/* if (service_results_p) */
							else
								{
									*error_ss = "Failed to create service response";
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


/*
 * STATIC DEFINITIONS
 */

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

	return NULL;
}


static JobsManager *LoadJobsManagerFromConfig (const json_t *config_p)
{
	const char *manager_s = GetJSONString (config_p, JOBS_MANAGER_S);

	if (manager_s)
		{
			JobsManager *jobs_manager_p = LoadJobsManager (manager_s);

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
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, config_p, "No jobs manager key, \"%s\" in config file", JOBS_MANAGER_S);
		}

	return NULL;
}


static ServersManager *LoadServersManagerFromConfig (const json_t *config_p)
{
	const char *manager_s = GetJSONString (config_p, SERVERS_MANAGER_S);

	if (manager_s)
		{
			ServersManager *manager_p = LoadServersManager (manager_s);

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
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, config_p, "No jobs manager key, \"%s\" in config file", SERVERS_MANAGER_S);
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
			GetJSONInteger (schema_p, VERSION_MAJOR_S, (int *) &major);
			GetJSONInteger (schema_p, VERSION_MINOR_S, (int *) &minor);
		}

	sv_p = AllocateSchemaVersion (major, minor);

	if (!sv_p)
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "AllocateSchemaVersion failed for " UINT32_FMT " " UINT32_FMT, major, minor);
		}

	return sv_p;
}
