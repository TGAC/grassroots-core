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
 * running_services_table.h
 *
 *  Created on: 17 Apr 2015
 *      Author: tyrrells
 */

#ifndef JOBS_MANAGER_H
#define JOBS_MANAGER_H

#include "hash_table.h"
#include "service.h"
#include "service_job.h"
#include "grassroots_service_manager_library.h"
#include "uuid/uuid.h"


#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifdef ALLOCATE_JOBS_MANAGER_TAGS
	#define JOBS_MANAGER_PREFIX GRASSROOTS_SERVICE_MANAGER_API
	#define JOBS_MANAGER_VAL(x)	= x
#else
	#define JOBS_MANAGER_PREFIX extern
	#define JOBS_MANAGER_VAL(x)
#endif

#endif



/**
 * This specifies the relative filesystem path to where the Service
 * plugins are stored.
 */
JOBS_MANAGER_PREFIX const char *JOBS_MANAGERS_PATH_S JOBS_MANAGER_VAL("jobs_managers");



/* forward declaration */
struct JobsManager;
struct Plugin;

/**
 * A typedef'd function for serialising a ServiceJob into an
 * array of raw bytes.
 *
 * @param job_p The ServiceJob to serialise.
 * @param length_p If the serialisation is successful, the
 * length of the resultant array will be stored here.
 * @return The raw array or <code>NULL</code> upon error.
 * @ingroup server_group
 */
typedef unsigned char *(*ServiceJobSerialiser) (ServiceJob *job_p, uint32 *length_p);


/**
 * A typedef'd function for deserialising a ServiceJob from an
 * array of raw bytes.
 *
 * @param input_data_p The array of raw bytes from a previously serialised ServiceJob.
 * @param config_p If the ServiceJobDeserialiser requires some configuration data, it
 * can be passed in here. This can be <code>NULL</code>.
 * @return The ServiceJob or <code>NULL</code> upon error.
 * @ingroup server_group
 */
typedef ServiceJob *(*ServiceJobDeserialiser) (unsigned char *input_data_p, void *config_p);


/**
 * @brief A datatype for monitoring ServiceJobs.
 *
 * A JobsManager is used to store details of the ServiceJobs that are currently
 * running on a Server. ServiceJobs can be added, removed and searched for.
 * @ingroup server_group
 */
typedef struct JobsManager
{
	/** The plugin that thjis JobsManager was loaded from. */
	struct Plugin *jm_plugin_p;

	/**
	 * @brief Add a ServiceJob to the JobsManager.
	 *
	 * @param manager_p The JobsManager to add the ServiceJob to.
	 * @param job_key The uuid_t for the ServiceJob to search for.
	 * @param job_p The json_t representing the ServiceJob to add.
	 * @return <code>true</code> if the ServiceJob was added successfully,
	 * <code>false</code> otherwise.
	 * @memberof JobsManager
	 * @see AddServiceJobToJobsManager
	 */
	bool (*jm_add_job_fn) (struct JobsManager *manager_p, uuid_t job_key, ServiceJob *job_p);

	/**
	 * @brief Find a ServiceJob.
	 *
	 * Get a previously-added ServiceJob.
	 *
	 * @param manager_p The JobsManager to search on.
	 * @param key The uuid_t for the ServiceJob to search for.
	 * @return A json_t for the matching ServiceJob or <code>NULL</code>
	 * if it could not be found.
	 * @memberof JobsManager
	 * @see GetServiceJobFromJobsManager
	 */
	ServiceJob *(*jm_get_job_fn) (struct JobsManager *manager_p, const uuid_t key);


	/**
	 * @brief Remove a ServiceJob.
	 *
	 * Remove a previously-added ServiceJob from the JobsManager.
	 *
	 * @param manager_p The JobsManager to search on.
	 * @param key The uuid_t for the ServiceJob to search for.
	 * @param get_job_flag If this is <code>true</code> then the removed ServiceJon will be returned if
	 * possible. If this is <code>false</code>, then <code>NULL</code> will be returned.
	 * @return The removed ServiceJob or <code>NULL</code>, depending upon get_job_flag.
	 * @memberof JobsManager
	 * @see RemoveServiceJobFromJobsManager
	 */
	ServiceJob *(*jm_remove_job_fn) (struct JobsManager *manager_p, const uuid_t key, bool get_job_flag);



	/**
 	 * @brief Get all ServiceJobs
	 *
	 * Get a LinkedList of ServiceJobs that are currently stored in the JobsManager.
	 *
	 * @param manager_p The JobsManager to get the ServiceJobs from.
	 * @return A LinkedList of ServiceJobNodes of all of the ServiceJobs or <code>NULL</code> if there are not any.
	 * @see GetAllServiceJobsFromJobsManager
	 */
	LinkedList *(*jm_get_all_jobs_fn) (struct JobsManager *manager_p);

	/**
	 * The function to use to delete this JobsManager,
	 *
	 * @param manager_p The JobsManager to delete
	 * @return <code>true</code> if the JobsManager was freed successfullly, <code>false</code> otherwise.
	 */
	bool (*jm_delete_manager_fn) (struct JobsManager *manager_p);

} JobsManager;


#ifdef __cplusplus
	extern "C" {
#endif

/**
 * Get the current Server-wide JobsManager.
 *
 * @return The JobsManager.
 * @memberof JobsManager
 */
GRASSROOTS_SERVICE_MANAGER_API JobsManager *GetJobsManager (void);


/**
 * @brief Initialise a JobsManager.
 *
 * Set up the callback functions for a given JobsManager.
 *
 * @param manager_p The JobsManager to set up.
 * @param add_job_fn The callback function to set for jm_add_job_fn for the given JobsManager.
 * @param get_job_fn The callback function to set for jm_get_job_fn for the given JobsManager.
 * @param remove_job_fn The callback function to set for jm_remove_job_fn for the given JobsManager.
 * @param get_all_jobs_fn The callback function to set for jm_get_all_jobs_fn for the given JobsManager.
 * @param delete_manager_fn The callback function to set for jm_delete_manager_fn for the given JobsManager.
 * @memberof JobsManager
 */
GRASSROOTS_SERVICE_MANAGER_API void InitJobsManager (JobsManager *manager_p,
                      bool (*add_job_fn) (JobsManager *manager_p, uuid_t job_key, ServiceJob *job_p),
                      ServiceJob *(*get_job_fn)  (JobsManager *manager_p, const uuid_t key),
                      ServiceJob *(*remove_job_fn) (JobsManager *manager_p, const uuid_t key, bool get_job_flag),
											LinkedList *(*get_all_jobs_fn) (struct JobsManager *manager_p),
											bool (*delete_manager_fn) (struct JobsManager *manager_p));



/**
 * @brief Free a JobsManager.
 *
 * @param manager_p manager_p The JobsManager to free.
 * @return <code>true</code> if the JobsManager was freed successfully,
 * <code>false</code> otherwise
 * @memberof JobsManager
 */
GRASSROOTS_SERVICE_MANAGER_API bool FreeJobsManager (JobsManager *manager_p);


/**
 * @brief Add a ServiceJob to the JobsManager.
 *
 * This is simply a convenience wrapper around jm_add_job_fn
 * for the given JobsManager.
 *
 * @param manager_p The JobsManager to add the ServiceJob to.
 * @param job_key The uuid_t for the ServiceJob to search for.
 * @param job_p The ServiceJob to add.
 * @return <code>true</code> if the ServiceJob was added successfully,
 * <code>false</code> otherwise.
 * @memberof JobsManager
 * @see jm_add_job_fn
 */
GRASSROOTS_SERVICE_MANAGER_API bool AddServiceJobToJobsManager (JobsManager *manager_p, uuid_t job_key, ServiceJob *job_p);


/**
 * @brief Find a ServiceJob.
 *
 * Get a previously-added ServiceJob. This is simply a convenience
 * wrapper around jm_get_job_fn for the given JobsManager.
 *
 * @param manager_p The JobsManager to search on.
 * @param key The uuid_t for the ServiceJob to search for.
 * @return A ServiceJob for the matching ServiceJob or <code>NULL</code>
 * if it could not be found.
 * @memberof JobsManager
 * @see jm_get_job_fn
 */
GRASSROOTS_SERVICE_MANAGER_API ServiceJob *GetServiceJobFromJobsManager (JobsManager *manager_p, const uuid_t key);


/**
 * @brief Remove a ServiceJob.
 *
 * Remove a previously-added ServiceJob from the JobsManager.
 * This is simply a convenience wrapper around jm_remove_job_fn
 * for the given JobsManager.
 *
 * @param manager_p The JobsManager to search on.
 * @param key The uuid_t for the ServiceJob to search for.
 * @param get_job_flag If this is <code>true</code>, then the removed
 * ServiceJob will be returned. If this is <code>false</code> then the
 * ServiceJob will not be returned.
 * @return A ServiceJob for the matching ServiceJob if get_job_flag
 * is set to true or <code>NULL</code> if it could not be found or
 * get_job_flag is false.
 * @memberof JobsManager
 * @see jm_remove_job_fn
 */
GRASSROOTS_SERVICE_MANAGER_API ServiceJob *RemoveServiceJobFromJobsManager (JobsManager *manager_p, const uuid_t key, bool get_job_flag);


/**
 * @brief Get all ServiceJobs
 *
 * Get a LinkedList of ServiceJobs that are currently stored in the JobsManager.
 *
 * @param manager_p The JobsManager to get the ServiceJobs from.
 * @return A LinkedList of ServiceJobNodes of all of the ServiceJobs or <code>NULL</code> if there are not any.
 * @see GetAllServiceJobsFromJobsManager
 * @memberof JobsManager
 * @see jm_get_all_jobs_fn
 */
GRASSROOTS_SERVICE_MANAGER_API LinkedList *GetAllServiceJobsFromJobsManager (struct JobsManager *manager_p);


/**
 * Load and create a JobsManager from the named plugin.
 *
 * @param jobs_manager_s The name of the plugin to use to create the JobsManager. For instance
 * if you wish to load libmy_jobs.so on a Unix platform, the value to use would be "my_jobs"
 * @return The loaded JobsManager or <code>NULL</code> upon error.
 * @memberof JobsManager
 */
GRASSROOTS_SERVICE_MANAGER_API JobsManager *LoadJobsManager (const char *jobs_manager_s);


#ifdef __cplusplus
}
#endif

#endif /* JOBS_MANAGER_H */
