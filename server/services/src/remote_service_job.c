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
 * remote_service_job.c
 *
 *  Created on: 14 Feb 2016
 *      Author: billy
 */

#include <string.h>

#define ALLOCATE_REMOTE_SERVICE_JOB_TAGS (1)
#include "remote_service_job.h"
#include "memory_allocations.h"
#include "service.h"
#include "string_utils.h"


#ifdef _DEBUG
	#define REMOTE_SERVICE_JOB_DEBUG	(STM_LEVEL_FINEST)
#else
	#define REMOTE_SERVICE_JOB_DEBUG	(STM_LEVEL_NONE)
#endif



static bool UpdateRemoteServiceJob (ServiceJob *job_p);


static bool CalculateResultForRemoteServiceJob (ServiceJob *job_p);




RemoteServiceJob *CreateRemoteServiceJobFromResultsJSON (const char *remote_service_s, const char *remote_uri_s, const uuid_t remote_id, const json_t *results_p, Service *service_p, const char *name_s, const char *description_s, OperationStatus status)
{
	RemoteServiceJob *job_p = (RemoteServiceJob *) AllocMemory (sizeof (RemoteServiceJob));

	if (job_p)
		{
			if (InitServiceJobFromResultsJSON (& (job_p -> rsj_job), results_p, service_p, name_s, description_s, status, RSJ_TYPE_S))
				{
					if (SetRemoteServiceJobDetails (job_p, remote_service_s, remote_uri_s, remote_id))
						{
							return job_p;
						}		/* if (SetRemoteServiceJobDetails (job_p, remote_service_s, remote_uri_s, remote_id))) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "SetRemoteServiceJobDetails failed for \"%s\"", name_s);
						}

				}		/* if (InitServiceJobFromResultsJSON (& (job_p -> rsj_job), results_p, service_p, name_s, description_s, status)) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "InitServiceJobFromResultsJSON failed for \"%s\"", name_s);
				}

			FreeMemory (job_p);
		}		/* if (job_p) */

	return NULL;
}





RemoteServiceJob *AllocateRemoteServiceJob (Service *service_p, const char *job_name_s, const char *job_description_s, const char *remote_service_s, const char *remote_uri_s, uuid_t remote_job_id)
{
	RemoteServiceJob *remote_job_p = (RemoteServiceJob *) AllocMemory (sizeof (RemoteServiceJob));

	if (remote_job_p)
		{
			ServiceJob * const base_service_job_p = & (remote_job_p -> rsj_job);

			memset (remote_job_p, 0, sizeof (RemoteServiceJob));

			if (InitServiceJob (base_service_job_p, service_p, job_name_s, job_description_s, NULL, NULL, FreeRemoteServiceJob, NULL, RSJ_TYPE_S))
				{
					if (SetRemoteServiceJobDetails (remote_job_p, remote_service_s, remote_uri_s, remote_job_id))
						{
							return remote_job_p;
						}		/* if (SetRemoteServiceJobDetails (remote_job_p, remote_service_s, remote_uri_s, remote_job_id)) */

				}		/* if (InitServiceJob (base_service_job_p, service_p, job_name_s, job_description_s, NULL)) */

			FreeServiceJob (base_service_job_p);
			FreeMemory (remote_job_p);
		}		/* if (remote_job_p) */

	return NULL;
}


void FreeRemoteServiceJob (ServiceJob *job_p)
{
	RemoteServiceJob *remote_job_p = (RemoteServiceJob *) job_p;

	if (remote_job_p -> rsj_service_name_s)
		{
			FreeCopiedString (remote_job_p -> rsj_service_name_s);
		}

	if (remote_job_p -> rsj_uri_s)
		{
			FreeCopiedString (remote_job_p -> rsj_uri_s);
		}

	ClearServiceJob (& (remote_job_p -> rsj_job));
	FreeMemory (remote_job_p);
}



json_t *GetRemoteServiceJobAsJSON (RemoteServiceJob *job_p, bool omit_results_flag)
{
	json_t *job_json_p = GetServiceJobAsJSON (& (job_p -> rsj_job), omit_results_flag);

	if (job_json_p)
		{
			if (json_object_set_new (job_json_p, JOB_REMOTE_S, json_true ()) == 0)
				{
					if (json_object_set_new (job_json_p, JOB_REMOTE_URI_S, json_string (job_p -> rsj_uri_s)) == 0)
						{
							char uuid_s [UUID_STRING_BUFFER_SIZE];

							ConvertUUIDToString (job_p -> rsj_remote_job_id, uuid_s);

							if (json_object_set_new (job_json_p, JOB_REMOTE_UUID_S, json_string (uuid_s)) == 0)
								{
									if (json_object_set_new (job_json_p, JOB_REMOTE_SERVICE_S, json_string (job_p -> rsj_service_name_s)) == 0)
										{
											return job_json_p;
										}		/* if (json_object_set_new (job_json_p, JOB_REMOTE_SERVICE_S, json_string (job_p -> rsj_service_name_s)) == 0) */
									else
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, job_json_p, "Failed to add \"%s\": \"%s\"", JOB_REMOTE_SERVICE_S, job_p -> rsj_service_name_s);
										}

								}		/* if (json_object_set_new (job_json_p, JOB_REMOTE_UUID_S, json_string (uuid_s)) == 0) */
							else
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, job_json_p, "Failed to add \"%s\": \"%s\"", JOB_REMOTE_UUID_S, uuid_s);
								}

						}		/* if (json_object_set_new (job_json_p, JOB_REMOTE_URI_S, json_string (job_p -> rsj_uri_s)) == 0) */
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, job_json_p, "Failed to add \"%s\": \"%s\"", JOB_REMOTE_URI_S, job_p -> rsj_uri_s);
						}


				}		/* if (json_object_set_new (job_json_p, JOB_REMOTE_S, json_true ()) == 0) */
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, job_json_p, "Failed to add \"%s\": true", JOB_REMOTE_S);
				}

			json_decref (job_json_p);
		}		/* if (job_json_p) */
	else
		{
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, job_json_p, "Failed to allocate JSON object for RemoteServiceJob");
		}

	return NULL;
}


bool IsRemoteServiceJobJSON (const json_t *job_json_p)
{
	bool remote_flag = false;

	GetJSONBoolean (job_json_p, JOB_REMOTE_S, &remote_flag);

	return remote_flag;
}


RemoteServiceJob *GetRemoteServiceJobFromJSON (const json_t *job_json_p, Service *service_p, GrassrootsServer *grassroots_p)
{
	if (IsRemoteServiceJobJSON (job_json_p))
		{
			RemoteServiceJob *job_p = (RemoteServiceJob *) AllocMemory (sizeof (RemoteServiceJob));

			if (job_p)
				{
					if (InitServiceJobFromJSON (& (job_p -> rsj_job), job_json_p, service_p, grassroots_p))
						{
							const char *remote_service_s;

							if ((remote_service_s = GetJSONString (job_json_p, JOB_REMOTE_SERVICE_S)) != NULL)
								{
									const char *remote_uuid_s;

									if ((remote_uuid_s = GetJSONString (job_json_p, JOB_REMOTE_UUID_S)) != NULL)
										{
											uuid_t remote_id;

											if (uuid_parse (remote_uuid_s, remote_id) == 0)
												{
													const char *remote_uri_s;

													if ((remote_uri_s = GetJSONString (job_json_p, JOB_REMOTE_URI_S)) != NULL)
														{
															if (SetRemoteServiceJobDetails (job_p, remote_service_s, remote_uri_s, remote_id))
																{
																	return job_p;
																}		/* if (SetRemoteServiceJobDetails (job_p, remote_service_s, remote_uri_s, remote_id)) */
															else
																{
																	PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "SetRemoteServiceJobDetails failed  for service name \"%s\", remote uri \"%s\", remote uuid \"%s\"", remote_service_s, remote_uri_s, remote_uuid_s);
																}

														}		/* if ((remote_uri_s = GetJSONString (job_json_p, JOB_REMOTE_URI_S)) != NULL) */
													else
														{
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, job_json_p, "Failed to get \"%s\"", JOB_REMOTE_UUID_S);
														}

												}		/* if (uuid_parse (remote_uuid_s, remote_id)) */
											else
												{
													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to parse \"%s\" to uuid", remote_uuid_s);
												}


										}		/* if ((remote_uuid_s = GetJSONString (job_json_p, JOB_REMOTE_UUID_S)) != NULL) */
									else
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, job_json_p, "Failed to get \"%s\"", JOB_REMOTE_UUID_S);
										}

								}		/* if ((remote_service_s = GetJSONString (job_json_p, JOB_REMOTE_SERVICE_S)) != NULL) */
							else
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, job_json_p, "Failed to get \"%s\"", JOB_REMOTE_SERVICE_S);
								}

						}		/* if (InitServiceJobFromJSON (& (job_p -> rsj_job), job_json_p)) */

					FreeMemory (job_p);
				}		/* if (job_p) */

		}		/* if (IsRemoteServiceJobJSON (job_json_p)) */

	return NULL;
}


bool SetRemoteServiceJobDetails (RemoteServiceJob *remote_job_p, const char *remote_service_s, const char *remote_uri_s, const uuid_t remote_job_id)
{
	if (remote_uri_s)
		{
			if (remote_service_s)
				{
					char *uri_s = EasyCopyToNewString (remote_uri_s);

					if (uri_s)
						{
							char *service_s = EasyCopyToNewString (remote_service_s);

							if (service_s)
								{
									ServiceJob *base_job_p = & (remote_job_p -> rsj_job);

									remote_job_p -> rsj_service_name_s = service_s;
									remote_job_p -> rsj_uri_s = uri_s;

									uuid_copy (remote_job_p -> rsj_remote_job_id, remote_job_id);

									SetServiceJobUpdateFunction (base_job_p, UpdateRemoteServiceJob);
									SetServiceJobFreeFunction (base_job_p, FreeRemoteServiceJob);
									SetServiceJobCalculateResultFunction (base_job_p, CalculateResultForRemoteServiceJob);


									return true;
								}		/* if (service_s) */
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to copy remote service name \"%s\"", service_s);
								}

							FreeCopiedString (uri_s);
						}		/* if (uri_s) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to copy remote uri \"%s\"", remote_uri_s);
						}

				}		/* if (remote_service_s) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "No remote service name");
				}

		}		/* if (remote_uri_s) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "No remote service uri");
		}

	return false;
}


static bool UpdateRemoteServiceJob (ServiceJob *job_p)
{
	bool success_flag = false;
	RemoteServiceJob *remote_job_p = (RemoteServiceJob *) job_p;
	SchemaVersion *schema_p = AllocateSchemaVersion (CURRENT_SCHEMA_VERSION_MAJOR, CURRENT_SCHEMA_VERSION_MINOR);

	if (schema_p)
		{
			Connection *connection_p = AllocateWebServerConnection (remote_job_p -> rsj_uri_s);

			if (connection_p)
				{
					const uuid_t *id_pp [1];
					json_t *req_p;

					*id_pp = & (remote_job_p -> rsj_remote_job_id);
					req_p = GetServicesResultsRequest (id_pp, 1, connection_p, schema_p);

					if (req_p)
						{
							json_t *response_p = MakeRemoteJsonCall (req_p, connection_p);

							if (response_p)
								{
									#if REMOTE_SERVICE_JOB_DEBUG >= STM_LEVEL_FINER
									PrintJSONToLog (STM_LEVEL_FINER, __FILE__, __LINE__, response_p, "UpdateRemoteServiceJob response");
									#endif

									if (json_is_array (response_p))
										{
											size_t i;
											json_t *job_json_p;

											json_array_foreach (response_p, i, job_json_p)
												{
													const char *uuid_s =  GetJSONString (job_json_p, JOB_UUID_S);

													/*
													 *  Check that the uuid matches our RemoteServiceJob's
													 *  remote uuid.
													 */
													if (uuid_s)
														{
															uuid_t remote_id;

															if (uuid_parse (uuid_s, remote_id) == 0)
																{
																	if (uuid_compare (remote_id, remote_job_p -> rsj_remote_job_id) == 0)
																		{
																			OperationStatus remote_status = OS_ERROR;

																			if (GetOperationStatusFromServiceJobJSON (job_json_p, &remote_status))
																				{
																					const bool job_results_flag = (remote_status == OS_SUCCEEDED) || (remote_status == OS_PARTIALLY_SUCCEEDED);

																					SetServiceJobStatus (job_p, remote_status);

																					if (job_results_flag)
																						{
		 																					json_t *job_results_p = json_object_get (job_json_p, JOB_RESULTS_S);

																							if (job_results_p)
																								{
																									if (json_is_array (job_results_p))
																										{
																											/*
																											 * Add each of the results
																											 */
																											size_t j;
																											json_t *job_result_p;
																											size_t num_results_copied = 0;

																											json_array_foreach (job_results_p, j, job_result_p)
																												{
																													json_t *copied_result_p = json_deep_copy (job_result_p);

																													if (copied_result_p)
																														{
																															if (AddResultToServiceJob (job_p, copied_result_p))
																																{
																																	++ num_results_copied;
																																}		/* if (AddResultToServiceJob (job_p, copied_result_p))*/
																															else
																																{
																																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, job_json_p, "AddResultToServiceJob faIled");
																																}

																														}		/* if (copied_result_p) */
																													else
																														{
																															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, job_result_p, "Failed to copy JSON chunk");
																														}

																												}		/* json_array_foreach (job_results_p, j, job_result_p) */

																											if (num_results_copied == json_array_size (job_results_p))
																												{
																													success_flag = true;
																												}

																										}		/* if (json_is_array (job_results_p)) */
																									else
																										{
																											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, job_results_p, "\"%s\" is not an array");
																										}

																								}		/* if (job_results_p) */
																							else
																								{
																									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, job_json_p, "Failed to get \"%s\"", JOB_RESULTS_S);
																								}


																							if (!success_flag)
																								{
																									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, job_json_p, "Failed to update results for job \"%s\", setting status to failed", uuid_s);
																									SetServiceJobStatus (job_p, OS_FAILED);
																								}

																						}		/* if (job_results_flag) */

																				}		/* if (GetOperationStatusFromServiceJobJSON (job_json_p, &remote_status)) */
																			else
																				{
																					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, job_json_p, "Failed to get Operation status");
																				}


																		}		/* if (uuid_compare (remote_id, remote_job_p) == 0) */

																}		/* if (uuid_parse (uuid_s, remote_id) == 0) */
															else
																{
																	PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to parse \"%s\" to a uuid", uuid_s);
																}

														}		/* if (uiuid_s) */
													else
														{
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, response_p, "Failed to get \"%s\"", JOB_UUID_S);
														}

												}		/* json_array_foreach (all_results_p, i, job_p) */

										}		/* if (json_is_array (all_results_p)) */
									else
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, response_p, "\"%s\" is not an array", SERVICE_RESULTS_S);
										}

									json_decref (response_p);
								}		/* if (response_p) */
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "MakeRemoteJsonCall failed for \"%s\"", remote_job_p -> rsj_uri_s);
								}

							json_decref (req_p);
						}		/* if (req_p) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "GetServicesResultsRequest failed for \"%s\"", remote_job_p -> rsj_uri_s);
						}

					FreeConnection (connection_p);
				}		/* if (connection_p) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate connection to \"%s\"", remote_job_p -> rsj_uri_s);
				}

			FreeSchemaVersion (schema_p);
		}		/* if (schema_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate schema");
		}

	return success_flag;
}


/*
 * Rather than make two separate requests, one for getting the job status and one for getting its
 * results, UpdateRemoteServiceJob takes care of both. So all we do here is call it if it has not
 * already been called previously
 */
static bool CalculateResultForRemoteServiceJob (ServiceJob *job_p)
{
	bool success_flag = false;

	if (job_p -> sj_result_p)
		{
			success_flag = true;
		}
	else
		{
			success_flag = UpdateRemoteServiceJob (job_p);
		}

	return success_flag;
}
