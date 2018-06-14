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

#include "remote_service_job.h"
#include "memory_allocations.h"
#include "service.h"
#include "string_utils.h"



RemoteServiceJob *CreateRemoteServiceJobFromResultsJSON (const char *remote_service_s, const char *remote_uri_s, const uuid_t remote_id, const json_t *results_p, Service *service_p, const char *name_s, const char *description_s, OperationStatus status)
{
	RemoteServiceJob *job_p = (RemoteServiceJob *) AllocMemory (sizeof (RemoteServiceJob));

	if (job_p)
		{
			if (InitServiceJobFromResultsJSON (& (job_p -> rsj_job), results_p, service_p, name_s, description_s, status))
				{
					char *copied_service_s = EasyCopyToNewString (remote_service_s);

					if (copied_service_s)
						{
							char *copied_uri_s = EasyCopyToNewString (remote_uri_s);

							if (copied_uri_s)
								{
									job_p -> rsj_job.sj_free_fn = FreeRemoteServiceJob;

									uuid_copy (job_p -> rsj_remote_job_id, remote_id);
									job_p -> rsj_service_name_s = copied_service_s;
									job_p -> rsj_uri_s = copied_uri_s;

									return job_p;

								}		/* if (copied_uri_s) */
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to copy remote uri \"%s\"", remote_uri_s);
								}

						}		/* if (copied_service_s) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to copy remote service \"%s\"", remote_service_s);
						}

				}		/* if (InitServiceJobFromResultsJSON (& (job_p -> rsj_job), results_p, service_p, name_s, description_s, status)) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to copy remote service \"%s\"", remote_service_s);
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

			if (InitServiceJob (base_service_job_p, service_p, job_name_s, job_description_s, NULL, NULL, FreeRemoteServiceJob, NULL))
				{
					if (remote_uri_s && remote_service_s)
						{
							char *uri_s = EasyCopyToNewString (remote_uri_s);

							if (uri_s)
								{
									char *service_s = EasyCopyToNewString (remote_service_s);

									if (service_s)
										{
											remote_job_p -> rsj_service_name_s = service_s;
											remote_job_p -> rsj_uri_s = uri_s;

											uuid_copy (remote_job_p -> rsj_remote_job_id, remote_job_id);

											return remote_job_p;
										}

									FreeCopiedString (uri_s);
								}

							FreeCopiedString (uri_s);
						}		/* if (remote_uri_s) */

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


bool RefreshRemoteServiceJob (RemoteServiceJob *job_p)
{
	bool success_flag = false;
	//const uuid_t **id_pp = & (job_p -> rsj_remote_job_id);

//	json_t *req_p = GetServicesResultsRequest (id_pp, 1, connection_p, schema_p);


	return success_flag;
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
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, job_json_p, "Failed to allocate JSON object for RemoteServiceJob");
		}

	return NULL;
}


bool IsRemoteServiceJobJSON (const json_t *job_json_p)
{
	bool remote_flag = false;

	GetJSONBoolean (job_json_p, JOB_REMOTE_S, &remote_flag);

	return remote_flag;
}




/*
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
									}		 if (json_object_set_new (job_json_p, JOB_REMOTE_SERVICE_S, json_string (job_p -> rsj_service_name_s)) == 0)
								else
									{
										PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, job_json_p, "Failed to add \"%s\": \"%s\"", JOB_REMOTE_SERVICE_S, job_p -> rsj_service_name_s);
									}

							}		 if (json_object_set_new (job_json_p, JOB_REMOTE_UUID_S, json_string (uuid_s)) == 0)
						else
							{
								PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, job_json_p, "Failed to add \"%s\": \"%s\"", JOB_REMOTE_UUID_S, uuid_s);
							}

					}		 if (json_object_set_new (job_json_p, JOB_REMOTE_URI_S, json_string (job_p -> rsj_uri_s)) == 0)
				else
					{
						PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, job_json_p, "Failed to add \"%s\": \"%s\"", JOB_REMOTE_URI_S, job_p -> rsj_uri_s));
					}


			}		 if (json_object_set_new (job_json_p, JOB_REMOTE_S, json_true ()) == 0)
		else
			{
				PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, job_json_p, "Failed to add \"%s\": true", JOB_REMOTE_S);
			}

*/


RemoteServiceJob *GetRemoteServiceJobFromJSON (const json_t *job_json_p)
{
	if (IsRemoteServiceJobJSON (job_json_p))
		{
			RemoteServiceJob *job_p = (RemoteServiceJob *) AllocMemory (sizeof (RemoteServiceJob));

			if (job_p)
				{
					if (InitServiceJobFromJSON (& (job_p -> rsj_job), job_json_p))
						{
							const char *remote_service_s;

							if ((remote_service_s = GetJSONString (job_json_p, JOB_REMOTE_SERVICE_S)) != NULL)
								{
									const char *remote_uuid_s;

									if ((remote_uuid_s = GetJSONString (job_json_p, JOB_REMOTE_UUID_S)) != NULL)
										{
											uuid_t remote_id;

											if (uuid_parse (remote_uuid_s, remote_id))
												{
													const char *remote_uri_s;

													if ((remote_uri_s = GetJSONString (job_json_p, JOB_REMOTE_URI_S)) != NULL)
														{

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


