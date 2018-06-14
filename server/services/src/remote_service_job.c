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



RemoteServiceJob *CreateRemoteServiceJobFromResultsJSON (const json_t *results_p, Service *service_p, const char *name_s, const char *description_s, OperationStatus status)
{
	RemoteServiceJob *job_p = (RemoteServiceJob *) AllocMemory (sizeof (RemoteServiceJob));

	if (job_p)
		{
			if (InitServiceJobFromResultsJSON (& (job_p -> rsj_job), results_p, service_p, name_s, description_s, status))
				{
					job_p -> rsj_job.sj_free_fn = FreeRemoteServiceJob;

					uuid_clear (job_p -> rsj_remote_job_id);
					job_p -> rsj_service_name_s = NULL;
					job_p -> rsj_uri_s = NULL;

					return job_p;
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
	const uuid_t **id_pp = & (job_p -> rsj_remote_job_id);

	json_t *req_p = GetServicesResultsRequest (id_pp, 1, connection_p, schema_p);


	return success_flag;
}



json_t *GetRemoteServiceJobAsJSON (RemoteServiceJob *job_p, bool omit_results_flag)
{
	json_t *job_json_p = GetServiceJobAsJSON (& (job_p -> rsj_job), omit_results_flag);

	if (job_json_p)
		{

			return job_json_p;
		}

	return NULL;
}



