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

/**
 * @file
 * @brief
 */

/*
 * remote_service_job.h
 *
 *  Created on: 14 Feb 2016
 *      Author: billy
 */

#ifndef SERVICES_LIB_INCLUDE_REMOTE_SERVICE_JOB_H_
#define SERVICES_LIB_INCLUDE_REMOTE_SERVICE_JOB_H_

#include "jansson.h"

#include "service_job.h"
#include "grassroots_service_manager_library.h"


#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifdef ALLOCATE_REMOTE_SERVICE_JOB_TAGS
	#define REMOTE_SERVICE_JOB_PREFIX GRASSROOTS_SERVICE_MANAGER_API
	#define REMOTE_SERVICE_JOB_VAL(x)	= x
#else
	#define REMOTE_SERVICE_JOB_PREFIX extern GRASSROOTS_SERVICE_MANAGER_API
	#define REMOTE_SERVICE_JOB_VAL(x)
#endif

#endif

/**
 * The job type for RemoteServiceJobs.
 *
 * @memberof RemoteServiceJob
 */
REMOTE_SERVICE_JOB_PREFIX const char * const RSJ_TYPE_S REMOTE_SERVICE_JOB_VAL ("remote_service_job");


/**
 * A datatype describing a ServiceJob that is run on a remote
 * Grassroots Server.
 *
 * @extends ServiceJob
 * @ingroup services_group
 */
typedef struct RemoteServiceJob
{
	/** The base ServiceJob. */
	ServiceJob rsj_job;

	/** The URI for the ExternalServer that is running the RemoteServiceJob. */
	char *rsj_uri_s;

	/** The name of the Service that will run the RemoteServiceJob. */
	char *rsj_service_name_s;

	/** The UUID of the RemoteServiceJob on its remote Server. */
	uuid_t rsj_remote_job_id;

} RemoteServiceJob;



#ifdef __cplusplus
extern "C"
{
#endif


/**
 * Allocate a RemoteServiceJob.
 *
 * @param service_p The local Service will connect to the ExternalServer that will run this RemoteServiceJob.
 * @param job_name_s The name of the RemoteServiceJob.
 * @param job_description_s An optional description to give the RemoteServiceJob. This can be <code>NULL</code>.
 * @param remote_service_s The name of the Service on the ExternalServer that will run the RemoteServiceJob.
 * @param remote_uri_s  The URI for the ExternalServer that is running the RemoteServiceJob.
 * @param remote_job_id The UUID of the RemoteServiceJob on this local Server.
 * @return The new RemoteServiceJob or <code>NULL</code> upon error.
 * @memberof RemoteServiceJob
 */
GRASSROOTS_SERVICE_MANAGER_API RemoteServiceJob *AllocateRemoteServiceJob (struct Service *service_p, const char *job_name_s, const char *job_description_s, const char *remote_service_s, const char *remote_uri_s, uuid_t remote_job_id);


/**
 * Free the RemoteServiceJob.
 *
 * @param job_p The RemoteServiceJob to free.
 * @memberof RemoteServiceJob
 */
GRASSROOTS_SERVICE_MANAGER_API void FreeRemoteServiceJob (ServiceJob *job_p);


/**
 * Create a RemoteServiceJob from the JSON fragment returned from the ExternalServer
 * that ran the job.
 *
 * @param remote_service_s The name of the Service on the ExternalServer that will run the RemoteServiceJob.
 * @param remote_uri_s  The URI for the ExternalServer that is running the RemoteServiceJob.
 * @param remote_job_id The UUID of the RemoteServiceJob on this local Server.
 * @param results_p The results of the RemoreServiceJob. This can be <code>NULL</code>.
 * @param service_p The local Service will connect to the ExternalServer that will run this RemoteServiceJob.
 * @param job_name_s The name of the RemoteServiceJob.
 * @param job_description_s An optional description to give the RemoteServiceJob. This can be <code>NULL</code>.
 * @param status The OperationStatus to set the RemoteServiceJob's status to.
 * @return The newly-created RemoteServiceJob or <code>NULL</code> upon error.
 * @memberof RemoteServiceJob
 */
GRASSROOTS_SERVICE_MANAGER_API RemoteServiceJob *CreateRemoteServiceJobFromResultsJSON (const char *remote_service_s, const char *remote_uri_s, const uuid_t remote_job_id, const json_t *results_p, struct Service *service_p, const char *job_name_s, const char *job_description_s, OperationStatus status);


/**
 * Test whether a given JSON object represents a RemoteServiceJob.
 *
 * @param job_json_p The JSON object to check.
 * @return <code>true</code> if theSON object represents a RemoteServiceJob,
 * <code>false</code> otherwise.
 * @memberof RemoteServiceJob
 */
GRASSROOTS_SERVICE_MANAGER_API bool IsRemoteServiceJobJSON (const json_t *job_json_p);



/**
 * Create a RemoteServiceJob from a given JSON fragment.
 *
 * @param job_json_p The JSON fragment to create the RemoteServiceJob from.
 * @return The newly-allocated RemoteServiceJob or <code>NULL</code> upon error.
 * @memberof RemoteServiceJob
 */
GRASSROOTS_SERVICE_MANAGER_API RemoteServiceJob *GetRemoteServiceJobFromJSON (const json_t *job_json_p, struct Service *service_p, GrassrootsServer *grassroots_p);



/**
 * Create a given JSON fragment for a given RemoteServiceJob.
 *
 * @param job_p The RemoteServiceJob to create the JSON fragment for.
 * @param omit_results_flag <code>true</code> if the BlastServiceJob is to exclude
 * any results from the created JSON fragment, <code>false</code> to include them.
 * @return The newly-created JSON fragment or <code>NULL</code> upon error.
 * @memberof RemoteServiceJob
 */
GRASSROOTS_SERVICE_MANAGER_API json_t *GetRemoteServiceJobAsJSON (RemoteServiceJob *job_p, bool omit_results_flag);


/**
 * Set the RemoteServiceJob-specific details for a given RemoteServiceJob.
 *
 * @param remote_job_p The RemoteServiceJob to update.
 * @param remote_service_s The name of the Service on the ExternalServer that will run the RemoteServiceJob.
 * @param remote_uri_s  The URI for the ExternalServer that is running the RemoteServiceJob.
 * @param remote_job_id The UUID of the RemoteServiceJob on the ExternalServer.
 * @return <code>true</code> if the RemoteServiceJob was updated successfully,
 * <code>false</code> upon error
 * @memberof RemoteServiceJob
 */
GRASSROOTS_SERVICE_MANAGER_API bool SetRemoteServiceJobDetails (RemoteServiceJob *remote_job_p, const char *remote_service_s, const char *remote_uri_s, const uuid_t remote_job_id);



#ifdef __cplusplus
}
#endif


#endif /* SERVICES_LIB_INCLUDE_REMOTE_SERVICE_JOB_H_ */
