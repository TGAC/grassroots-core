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
 * service_job.h
 *
 *  Created on: 25 Apr 2015
 *      Author: billy
 */

#ifndef SERVICE_JOB_H_
#define SERVICE_JOB_H_

#include "typedefs.h"
#include "grassroots_service_library.h"
#include "operation.h"
#include "jansson.h"
#include "linked_list.h"
#include "memory_allocations.h"
#include "linked_service.h"


#include "uuid_defs.h"

/* forward declarations */
struct Service;
struct ServiceJob;
struct LinkedService;


#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifdef ALLOCATE_SERVICE_JOB_TAGS
	#define SERVICE_JOB_PREFIX GRASSROOTS_SERVICE_API
	#define SERVICE_JOB_VAL(x)	= x
#else
	#define SERVICE_JOB_PREFIX extern GRASSROOTS_SERVICE_API
	#define SERVICE_JOB_VAL(x)
#endif

#endif


/** The default type id for ServiceJobs. */
SERVICE_JOB_PREFIX const char *SJ_DEFAULT_TYPE_S SERVICE_JOB_VAL ("default_service_job");


/**
 * @brief A datatype to represent a running task.
 *
 * Each Service can run one or more ServiceJobs.
 *
 * @ingroup services_group
 */
typedef struct ServiceJob
{
	/** The Service that is running this job. */
	struct Service *sj_service_p;

	/** The unique identifier for this job. */
	uuid_t sj_id;

	/** Is the service currently in an open state? */
	enum OperationStatus sj_status;

	/** The name of the Service that this ServiceJob is for */
	char *sj_service_name_s;

	/** The name of the ServiceJob */
	char *sj_name_s;

	/** The description of the ServiceJob */
	char *sj_description_s;

	/** Any user-facing URL for showing results of running this ServiceJob. */
	char *sj_url_s;

	/**
	 * The JSON fragment for the results of this ServiceJob.
	 */
	json_t *sj_result_p;

	/**
	 * The JSON fragment for any extra information for this ServiceJob.
	 */
	json_t *sj_metadata_p;

	/**
	 * The JSON fragment for any errors that have occurred whilst
	 * running this ServiceJob.
	 */
	json_t *sj_errors_p;


	/**
	 * A JSON array where each object contains the details of
	 * running a different Service based upon the results of this
	 * ServiceJob
	 */
	json_t *sj_linked_services_p;


	/**
	 * The callback function to use when checking the status of
	 * this ServiceJob. This is useful for subclasses of ServiceJob that need custom
	 * behaviour. If this is <code>NULL</code>, then GetServiceJobStatus will be used.
	 */
	bool (*sj_update_fn) (struct ServiceJob *job_p);

	/**
   * The callback function to use when freeing
	 * this ServiceJob. This is useful for subclasses of ServiceJob that need custom
	 * behaviour. If this is <code>NULL</code>, then ClearServiceJob will be used before
	 * deallocating the memory ServiceJob structure.
	 */
	void (*sj_free_fn) (struct ServiceJob *job_p);



	/**
	 * The callback function to use when a ServiceJob needs to generate its results
	 * This is useful for subclasses of ServiceJob that need custom
	 * behaviour. This can be <code>NULL</code>.
	 */
	bool (*sj_calculate_result_fn) (struct ServiceJob *job_p);



	/**
	 * Is this ServiceJob in the process of updating itself?
	 */
	bool sj_is_updating_flag;


	/**
	 * The type of ServiceJob
	 *
	 * This is used to denote a subclass of ServiceJob if needed and
	 * you need to check its type
	 */
	char *sj_type_s;
} ServiceJob;


/**
 * A datatype used to store a ServiceJob within
 * a ServceJobSet using a LinkedList.
 *
 * @extends ListItem
 * @ingroup services_group
 */
typedef struct ServiceJobNode
{
	/** The node used to hold this in a LinkedList. */
	ListItem sjn_node;

	/** The ServiceJob. */
	ServiceJob *sjn_job_p;

} ServiceJobNode;


/**
 * A datatype to represent a collection of ServiceJobs.
 *
 * @ingroup services_group
 */
typedef struct ServiceJobSet
{
	/** The Service that is running these jobs. */
	struct Service *sjs_service_p;

	/** The ServiceJobs that are in use for the Service. */
	LinkedList *sjs_jobs_p;


} ServiceJobSet;


#ifdef __cplusplus
extern "C"
{
#endif


/**
 * Free a ServiceJob.
 *
 * @param job_p The ServiceJob to free.
 * @memberof ServiceJob
 */
GRASSROOTS_SERVICE_API void FreeServiceJob (ServiceJob *job_p);


/**
 * Free a ServiceJob ignoring if it is a subclass.
 *
 * Unless you want to explicitly delete the base ServiceJob only,
 * then FreeServiceJob() should be used instead. This is
 * primarily to be called from the functions that free
 * subclassed ServiceJobs.
 *
 * @param job_p The ServiceJob to free.
 * @memberof ServiceJob
 */
GRASSROOTS_SERVICE_API void FreeBaseServiceJob (ServiceJob *job_p);


/**
 * @brief Allocate a ServiceJob.
 *
 * @param service_p The Service that is running the ServiceJob.
 * @param job_name_s The name to give to the ServiceJob.
 * @param job_description_s The description to give to the ServiceJob.
 * @param update_fn The callback function to use when checking the ServiceJob
 * has been updated. This is useful for subclasses of ServiceJob that need custom
 * behaviour. If this is <code>NULL</code>, then GetServiceJobStatus will be used.
 * @param calculate_results_fn The callback function to use to determine the results
 * for this ServiceJob. This is useful for subclasses of ServiceJob that need custom
 * behaviour. If this is <code>NULL</code>, then the contents of sj_result_p will be
 * used as is.
 * @param free_job_fn The callback function to use when freeing
 * this ServiceJob. This is useful for subclasses of ServiceJob that need custom
 * behaviour. If this is <code>NULL</code>, then ClearServiceJob will be used before
 * deallocating the memory ServiceJob structure.
 * @param job_type_s The identifier to use for the given ServiceJob.
 * @return The newly-allocated ServiceJob or <code>NULL</code> upon error.
 * @see FreeServiceJob
 * @see InitServiceJob
 * @see GetServiceJobStatus
 * @see ClearServiceJob
 * @memberof ServiceJob
 */
GRASSROOTS_SERVICE_API ServiceJob *AllocateServiceJob (struct Service *service_p, const char *job_name_s, const char *job_description_s, bool (*update_fn) (struct ServiceJob *job_p), bool (*calculate_results_fn) (struct ServiceJob *job_p), void (*free_job_fn) (struct ServiceJob *job_p), const char *job_type_s);



/**
 * @brief Allocate a ServiceJob and add it to a ServiceJobSet.
 *
 * This will call AllocateServiceJob and if successful will then
 * add the new ServiceJob to the given Service.
 *
 * @param service_p The Service to add the new ServiceJob to.
 * @param job_name_s The name to give to the ServiceJob.
 * @param job_description_s The description to give to the ServiceJob.
 * @param update_fn The callback function to use when checking the ServiceJob
 * has been updated. This is useful for subclasses of ServiceJob that need custom
 * behaviour. If this is <code>NULL</code>, then GetServiceJobStatus will be used.
 * @param calculate_results_fn The callback function to use to determine the results
 * for this ServiceJob. This is useful for subclasses of ServiceJob that need custom
 * behaviour. If this is <code>NULL</code>, then the contents of sj_result_p will be
 * used as is.
 * @param free_job_fn The callback function to use when freeing
 * this ServiceJob. This is useful for subclasses of ServiceJob that need custom
 * behaviour. If this is <code>NULL</code>, then ClearServiceJob will be used before
 * deallocating the memory ServiceJob structure.
 * @return The newly-allocated ServiceJob or <code>NULL</code> upon error.
 * @see AllocateServiceJob
 * @memberof ServiceJob
 */
GRASSROOTS_SERVICE_API ServiceJob *CreateAndAddServiceJobToService (struct Service *service_p, const char *job_name_s, const char *job_description_s, bool (*update_fn) (struct ServiceJob *job_p), bool (*calculate_results_fn) (struct ServiceJob *job_p), void (*free_job_fn) (struct ServiceJob *job_p));


/**
 * @brief Initialise a ServiceJob.
 *
 * This will initialise a ServiceJob ready for use and will automatically get called for all
 * ServiceJobs in a ServiceJobSet when it is first created.
 *
 * @param job_p The ServiceJob to initialise.
 * @param job_name_s The name to give to the ServiceJob.
 * @param job_description_s The description to give to the ServiceJob.
 * @param service_p The Service that is running the ServiceJob.
 * @param update_fn The callback function to use when checking the ServiceJob
 * has been updated. This is useful for subclasses of ServiceJob that need custom
 * behaviour. If this is <code>NULL</code>, then GetServiceJobStatus will be used.
 * @param calculate_results_fn The callback function to use to determine the results
 * for this ServiceJob. This is useful for subclasses of ServiceJob that need custom
 * behaviour. If this is <code>NULL</code>, then the contents of sj_result_p will be
 * used as is.
 * @param free_job_fn The callback function to use when freeing
 * this ServiceJob. This is useful for subclasses of ServiceJob that need custom
 * behaviour. If this is <code>NULL</code>, then ClearServiceJob will be used before
 * deallocating the memory ServiceJob structure.
 * @param id_p Pointer to the the uuid to use as the id for this ServiceJob.
 * @param job_type_s The identifier to use for the given ServiceJob class.
 * @return <code>true</code> if the ServiceJob was initialised successfully, <code>false</code> otherwise
 * @memberof ServiceJob
 */
GRASSROOTS_SERVICE_API bool InitServiceJob (ServiceJob *job_p, struct Service *service_p, const char *job_name_s, const char *job_description_s, bool (*update_fn) (struct ServiceJob *job_p), bool (*calculate_results_fn) (struct ServiceJob *job_p), void (*free_job_fn) (struct ServiceJob *job_p), uuid_t *id_p, const char *job_type_s);


/**
 * @brief Clear a Service Job ready for reuse.
 *
 * @param job_p The ServiceJob to clear.
 * @memberof ServiceJob
 */
GRASSROOTS_SERVICE_API void ClearServiceJob (ServiceJob *job_p);


/**
 * Create a deep copy of a ServiceJob.
 *
 * @param src_p The ServiceJob to copy.
 * @return The newly-copied ServiceJob or <code>NULL</code> upon error.
 * @memberof ServiceJob
 */
GRASSROOTS_SERVICE_API ServiceJob *CloneServiceJob (const ServiceJob *src_p);


/**
 * Make a deep copy of one ServiceJob to another.
 *
 * @param src_p The ServiceJob to copy.
 * @param dest_p The ServiceJob where the values will be copied to.
 * @return <code>true</code> if the ServiceJob was copied successfully, <code>false</code> otherwise.
 * @memberof ServiceJob
 */
GRASSROOTS_SERVICE_API bool CopyServiceJob (const ServiceJob *src_p, ServiceJob *dest_p);


/**
 * Get the owning Service for a given ServiceJob.
 *
 * @param job_p The ServiceJOb to get the Service for.
 * @return The Service or <code>NULL</code> if there was an error.
 * @memberof ServiceJob
 */
GRASSROOTS_SERVICE_API struct Service *GetServiceFromServiceJob (ServiceJob *job_p);


/**
 * Find the ServiceJobNode for a given ServiceJob within a ServiceJobSet.
 *
 * @param job_set_p The ServiceJobSet to search.
 * @param job_p The ServiceJob to look for.
 * @return The ServiceJobNode that points to the requested ServiceJob or <code>
 * NULL</code> if it could not be found.
 * @memberof ServiceJobSet
 */
GRASSROOTS_SERVICE_API ServiceJobNode *FindServiceJobNodeInServiceJobSet (const ServiceJobSet *job_set_p, const ServiceJob *job_p);


GRASSROOTS_SERVICE_API bool IsServiceJobInServiceJobSet (const ServiceJobSet *job_set_p, const ServiceJob *job_p);


/**
 * Find the ServiceJobNode for a ServiceJob with a given uuid within a ServiceJobSet.
 *
 * @param job_set_p The ServiceJobSet to search.
 * @param job_id The uuid of the ServiceJob to find.
 * @return The ServiceJobNode that points to the requested ServiceJob or <code>
 * NULL</code> if it could not be found.
 * @memberof ServiceJobSet
 */
GRASSROOTS_SERVICE_API ServiceJobNode *FindServiceJobNodeByUUIDInServiceJobSet (const ServiceJobSet *job_set_p, const uuid_t job_id);


/**
 * Remove a ServiceJob with a given uuid from a ServiceJobSet.
 *
 * The ServiceJob is not deleted, it is just removed from the ServiceJobSet.
 *
 * @param job_set_p The ServiceJobSet to remove the given ServiceJob from.
 * @param job_id The uuid of the ServiceJob to remove.
 * @return <code>true</code> if the ServiceJob was removed successfully, <code>false</code> otherwise.
 * @memberof ServiceJobSet
 * @see RemoveServiceJobFromServiceJobSet()
 */
GRASSROOTS_SERVICE_API bool RemoveServiceJobByUUIDFromServiceJobSet (ServiceJobSet *job_set_p, uuid_t job_id);


/**
 * Remove a ServiceJob from a ServiceJobSet.
 *
 * The ServiceJob is not deleted, it is just removed from the ServiceJobSet.
 *
 * @param job_set_p The ServiceJobSet to remove the given ServiceJob from.
 * @param job_p The ServiceJob to remove.
 * @return <code>true</code> if the ServiceJob was removed successfully, <code>false</code> otherwise.
 * @memberof ServiceJobSet
 */
GRASSROOTS_SERVICE_API bool RemoveServiceJobFromServiceJobSet (ServiceJobSet *job_set_p, ServiceJob *job_p);



/**
 * @brief Set the description of ServiceJob.
 *
 * @param job_p The ServiceJob to alter.
 * @param description_s The description to set. A deep copy will be made of this string so
 * this value does not need to stay in scope.
 * @return <code>true</code> if the name was set successfully, <code>false</code> otherwise.
 * @memberof ServiceJob
 */
GRASSROOTS_SERVICE_API bool SetServiceJobDescription (ServiceJob *job_p, const char * const description_s);




/**
 * @brief Set the description of ServiceJob.
 *
 * @param job_p The ServiceJob to alter.
 * @param url_s The url to set. A deep copy will be made of this string so
 * this value does not need to stay in scope.
 * @return <code>true</code> if the name was set successfully, <code>false</code> otherwise.
 * @memberof ServiceJob
 */
GRASSROOTS_SERVICE_API bool SetServiceJobURL (ServiceJob *job_p, const char * const url_s);


/**
 * @brief Set the name of ServiceJob.
 *
 * @param job_p The ServiceJob to alter.
 * @param name_s The name to set. A deep copy will be made of this string so
 * this value does not need to stay in scope.
 * @return <code>true</code> if the name was set successfully, <code>false</code> otherwise.
 * @memberof ServiceJob
 */
GRASSROOTS_SERVICE_API bool SetServiceJobName (ServiceJob *job_p, const char * const name_s);


/**
 * @brief Allocate a ServiceJobSet.
 *
 * @param service_p The Service to allocate the ServiceJobSet for.
 * @return A newly-allocated ServiceJobSet or <code>NULL</code> upon error.
 * @memberof ServiceJobSet
 * @see InitServiceJob
 */
GRASSROOTS_SERVICE_API ServiceJobSet *AllocateServiceJobSet (struct Service *service_p);



/**
 * @brief Allocate a ServiceJobSet and populate it with a single ServiceJob
 *
 * @param service_p The Service to allocate the ServiceJobSet for.
 * @param job_name_s The name that will be given to the ServiceJob that will be created.
 * @param job_description_s The description that will be given to the ServiceJob that will be created.
 * This can be <code>NULL</code>.
 * @return A newly-allocated ServiceJobSet or <code>NULL</code> upon error.
 * @memberof ServiceJobSet
 * @see InitServiceJob
 */
GRASSROOTS_SERVICE_API ServiceJobSet *AllocateSimpleServiceJobSet (struct Service *service_p, const char *job_name_s, const char *job_description_s);


/**
 * Free a ServiceJobSet.
 *
 * @param job_set_p The ServiceJobSet to free.
 * @memberof ServiceJobSet
 */
GRASSROOTS_SERVICE_API void FreeServiceJobSet (ServiceJobSet *job_set_p);



/**
 * @brief Allocate a ServiceJobNode.
 *
 * @param job_p The ServiceJob to allocate a ServiceJobNode for.
 * @return A newly-allocated ServiceJobNode or <code>NULL</code> upon error.
 * @memberof ServiceJobNode
 */
GRASSROOTS_SERVICE_API ServiceJobNode *AllocateServiceJobNode (ServiceJob *job_p);


/**
 * Free a ServiceJobNode.
 *
 * @param service_job_node_p The ServiceJobNode to free.
 * @memberof ServiceJobNode
 */
GRASSROOTS_SERVICE_API void FreeServiceJobNode (ListItem *service_job_node_p);


/**
 * Search a ServiceJobSet for ServiceJob.
 *
 * @param jobs_p The ServiceJobSet to search.
 * @param job_id The uuid_t for the ServiceJob to find.
 * @return A pointer to the matching ServiceJob or <code>NULL
 * </code> if it could not be found.
 * @memberof ServiceJobSet
 */
GRASSROOTS_SERVICE_API ServiceJob *GetServiceJobFromServiceJobSetById (const ServiceJobSet *jobs_p, const uuid_t job_id);


/**
 * Get the json representation of a ServiceJobSet.
 *
 * @param jobs_p The ServiceJobSet to get the representation of.
 * @param omit_results_flag If this is <code>true</code> then just the minimal status information for
 * the ServiceJob will be returned. If it is <code>false</code> then the job results will be included too if possible.
 * @return The json_t representing the ServiceJobSet or <code>NULL
 * </code> upon error.
 * @memberof ServiceJobSet
 */
GRASSROOTS_SERVICE_API json_t *GetServiceJobSetAsJSON (const ServiceJobSet *jobs_p, bool omit_results_flag);


/**
 * @brief Get a ServiceJob as JSON.
 *
 * @param job_p The ServiceJob to convert into JSON.
 * @param omit_results_flag If this is <code>true</code> then just the minimal status information for
 * the ServiceJob will be returned. If it is <code>false</code> then the job results will be included too if possible.
 * @return The json_t object representing the ServiceJob.
 * or <code>NULL</code> upon error.
 * @memberof ServiceJob
 */
GRASSROOTS_SERVICE_API json_t *GetServiceJobAsJSON (ServiceJob * const job_p, bool omit_results_flag);


/**
 * @brief Get the Current OperationStatus of a ServiceJob as JSON.
 *
 * @param job_p The ServiceJob to query.
 * @param omit_results_flag If this is <code>true</code> then just the minimal status information for
 * the ServiceJob will be returned. If it is <code>false</code> then the job results will be included too if possible.
 * @return The current OperationStatus as a json_t object.
 * or <code>NULL</code> upon error.
 * @memberof ServiceJob
 */
GRASSROOTS_SERVICE_API json_t *GetServiceJobStatusAsJSON (ServiceJob *job_p, bool omit_results_flag);


/**
 * Process all ServiceJobs within a ServiceJobSet.
 *
 * Each ServiceJob will have its status checked and updated if necessary along
 * with setting up any LinkedServices if available.
 *
 * @param jobs_p The ServiceJobSet to process.
 * @param res_p The JSON array where are any results will get appended.
 * @return <code>true</code> if all ServiceJobs within the ServiceJobSet
 * were processed successfully, <code>false</code> otherwise.
 * @memberof ServiceJobSet
 */
GRASSROOTS_SERVICE_API bool ProcessServiceJobSet (ServiceJobSet *jobs_p, json_t *res_p);


/**
 * @brief Create a ServiceJob from a json_t object.
 *
 * This will create a ServiceJob from a json_t object. It will
 * allocate a new ServiceJob and then call <code>SetServiceFromJSON</code>.
 *
 * @param json_p The json object representing a ServiceJob.
 * @return <code>true</code> if the ServiceJob was created successfully,
 * <code>false</code> otherwise.
 * @memberof ServiceJob
 * @see SetServiceJobFromJSON
 *
 */
GRASSROOTS_SERVICE_API ServiceJob *CreateServiceJobFromJSON (const json_t *job_json_p, GrassrootsServer *grassroots_p);


/**
 * @brief Get a ServiceJob from a json_t object.
 *
 * @param job_p The ServiceJob which will be filled in from the json data.
 * @param json_p The json object representing a ServiceJob.
 * @return <code>true</code> if the ServiceJob was created successfully,
 * <code>false</code> otherwise.
 * @memberof ServiceJob
 */
GRASSROOTS_SERVICE_API bool InitServiceJobFromJSON (ServiceJob *job_p, const json_t *json_p, struct Service *service_p, GrassrootsServer *grassroots_p);


/**
 * @brief Get the Current OperationStatus of a ServiceJob.
 *
 * This will check and update the OperationStatus of the ServiceJob
 * to an up to date value before returning.
 *
 * @param job_p The ServiceJob to query.
 * @return The current OperationStatus.
 * @memberof ServiceJob
 */
GRASSROOTS_SERVICE_API OperationStatus GetServiceJobStatus (ServiceJob *job_p);


/**
 * @brief Get the last retrieved OperationStatus of a ServiceJob.
 *
 * This will not check to see if the current OperationStatus of
 * a ServiceJob has changed, for that use <code>GetServiceJobStatus</code>
 *
 *
 * @param job_p The ServiceJob to query.
 * @return The cached OperationStatus.
 * @see GetServiceJobStatus
 * @memberof ServiceJob
 */
GRASSROOTS_SERVICE_API OperationStatus GetCachedServiceJobStatus (const ServiceJob *job_p);


/**
 * @brief Get the name of ServiceJob.
 *
 * @param job_p The ServiceJob to query.
 * @return The name of the ServiceJob
 * @memberof ServiceJob
 */
GRASSROOTS_SERVICE_API const char *GetServiceJobName (const ServiceJob *job_p);


/**
 * @brief Close a ServiceJob.
 *
 * This will call the Services close callback function.
 *
 * @param job_p The ServiceJob to close.
 * @return <code>true</code> if the ServiceJob was closed successfully,
 * <code>false</code> otherwise.
 * @memberof ServiceJob
 */
GRASSROOTS_SERVICE_API bool CloseServiceJob (ServiceJob *job_p);


/**
 * Get the ServiceJob at a particular position in a ServiceJobSet.
 *
 * @param jobs_p The ServiceJobSet to get the ServiceJob from.
 * @param index The index of the ServiceJob to get.
 * @return The ServiceJob or <code>NULL</code> if there was an error
 * such as if the index was out of range.
 * @memberof ServiceJobSet
 */
GRASSROOTS_SERVICE_API ServiceJob *GetServiceJobFromServiceJobSet (const ServiceJobSet *jobs_p, const uint32 index);

/**
 * Get the number of ServiceJobs in a ServiceJobSet.
 *
 * @param jobs_p The ServiceJobSet to get size of.
 * @return The number of ServiceJobs in this ServiceJobSet.
 * @memberof ServiceJobSet
 */
GRASSROOTS_SERVICE_API uint32 GetServiceJobSetSize (const ServiceJobSet * const jobs_p);


/**
 * @brief Clear the results associated with a ServiceJob.
 *
 * This will call the Services close callback function.
 *
 * @param job_p The ServiceJob to close.
 * @param free_memory_flag If this is <code>true</code> then the ServiceJob's results will
 * be freed. If <code>false</code> then the results will just be set to NULL. This allows the
 * transfer of the results variable into another json_t object if needed for example.
 * @memberof ServiceJob
 */
GRASSROOTS_SERVICE_API void ClearServiceJobResults (ServiceJob *job_p, bool free_memory_flag);


/**
 * Save the ServiceJob to a persistent format that allows the ServiceJob
 * to be recreated in a potentially different thread and/or process. This is used to
 * save a ServiceJob in the JobsManager.
 *
 * @param job_p The ServiceJob.
 * @param omit_results_flag If this is <code>true</code> then just the minimal status information for
 * the ServiceJob will be returned. If it is <code>false</code> then the job results will be included too if possible.
 * @return The persistent representation of this ServiceJob or <code>NULL</code>
 * upon error.
 * @see JobsManager
 * @see DeserialiseServiceJobFromJSON
 * @memberof ServiceJob
 */
GRASSROOTS_SERVICE_API char *SerialiseServiceJobToJSON (ServiceJob * const job_p, bool omit_results_flag);


/**
 * @brief Create a ServiceJob from a JSON Resource fragment.
 *
 * This is useful when combining results from a PairedService. The resultant
 * ServiceJob will have its results field filled in using the JSON fragment
 * passed in.
 *
 * @param results_p A Resource JSON fragment.
 * @param service_p The Service that the newly-created ServiceJob will belong to.
 * @param name_s The name of the ServiceJob.
 * @param description_s The description of the ServiceJob. This can be <code>NULL</code>.
 * @param status The OperationStatus to set for the ServiceJob.
 * @param job_type_s The identifier to use for the given ServiceJob class.
 * @return The newly-allocated ServiceJob or <code>NULL</code> upon error.
 * @memberof ServiceJob
 * @see InitServiceJobFromResultsJSON
 */
GRASSROOTS_SERVICE_API ServiceJob *CreateServiceJobFromResultsJSON (const json_t *results_p, struct Service *service_p, const char *name_s, const char *description_s, OperationStatus status, const char *job_type_s);



/**
 * Fill in the data for a ServiceJob from a given JSON fragement.
 *
 * @param job_p The ServiceJob to fill in.
 * @param results_p The JSON fragment used to populate the data of the ServiceJob.
 * @param service_p The Service that refers to the ServiceJob
 * @param name_s The name to give to the ServiceJob. This can be <code>NULL</code>.
 * @param description_s The description to give to the ServiceJob. This can be <code>NULL</code>.
 * @param status The OperationStatus to set for the ServiceJob.
 * @param job_type_s The identifier to use for the given ServiceJob class.
 * @return <code>true</code> if the ServiceJob was initialised successfully,
 * <code>false</code> otherwise.
 * @memberof ServiceJob
 */
GRASSROOTS_SERVICE_API bool InitServiceJobFromResultsJSON (ServiceJob *job_p, const json_t *results_p, struct Service *service_p, const char *name_s, const char *description_s, OperationStatus status, const char *job_type_s);


/**
 *
 * Add a key-value pair error statement to a ServiceJob.
 *
 * @param job_p The ServiceJob to update.
 * @param key_s The key for the error.
 * @param value_s The value for the error.
 * @return <code>true</code> if the ServiceJob was amended successfully,
 * <code>false</code> otherwise.
 * @memberof ServiceJob
 */
GRASSROOTS_SERVICE_API bool AddParameterErrorMessageToServiceJob (ServiceJob *job_p, const char * const param_s, const ParameterType param_type, const char * const value_s);


/**
 * Add a key-value pair error statement to a ServiceJob.
 *
 * @param job_p The ServiceJob to update.
 * @param key_s The key for the error.
 * @param value_s The value for the error.
 * @return <code>true</code> if the ServiceJob was amended successfully,
 * <code>false</code> otherwise.
 * @memberof ServiceJob
 */
GRASSROOTS_SERVICE_API bool AddGeneralErrorMessageToServiceJob (ServiceJob *job_p, const char * const value_s);


/**
 * Add a key-value pair error statement to a ServiceJob.
 *
 * @param job_p The ServiceJob to update.
 * @param key_s The key for the error.
 * @param compound_error_s The value for the error.
 * @return <code>true</code> if the ServiceJob was amended successfully,
 * <code>false</code> otherwise.
 * @memberof ServiceJob
 */
GRASSROOTS_SERVICE_API bool AddCompoundErrorToServiceJob (ServiceJob *job_p, const char *param_s, const ParameterType param_type, json_t *error_details_p);


GRASSROOTS_SERVICE_API bool AddTabularParameterErrorMessageToServiceJob (ServiceJob *job_p, const char * const param_s, const ParameterType param_type, const char * const value_s, const uint32 row, const char *column_s);


/**
 * Add a result to ServiceJob.
 *
 * @param job_p The ServiceJob to add the result to.
 * @param result_p The result to add. This is a Resource stored in json format.
 * @return <code>true</code> if the ServiceJob was updated successfully,
 * <code>false</code> otherwise.
 * @memberof ServiceJob
 */
GRASSROOTS_SERVICE_API bool AddResultToServiceJob (ServiceJob *job_p, json_t *result_p);



/**
 * Set the current OperationStatus for a given ServiceJob.
 *
 * @param job_p The ServiceJob to update.
 * @param status The new OperationStatus value.
 * @memberof ServiceJob
 */
GRASSROOTS_SERVICE_API void SetServiceJobStatus (ServiceJob *job_p, OperationStatus status);


/**
 * Set the function that a ServiceJob will use to update itself.
 *
 * @param job_p The ServiceJob to amend.
 * @param update_fn The new function that will be used for the ServiceJob to update itself.
 * @memberof ServiceJob
 */
GRASSROOTS_SERVICE_API void SetServiceJobUpdateFunction (ServiceJob *job_p, bool (*update_fn) (ServiceJob *job_p));


/**
 * Set the function that a ServiceJob will use to free itself.
 *
 * @param job_p The ServiceJob to amend.
 * @param free_fn The new function that will be used for the ServiceJob to free itself.
 * @memberof ServiceJob
 */
GRASSROOTS_SERVICE_API void SetServiceJobFreeFunction (ServiceJob *job_p, void (*free_fn) (ServiceJob *job_p));



/**
 * Set the function that a ServiceJob will use to calculate its results.
 *
 * @param job_p The ServiceJob to amend.
 * @param calculate_fn The new function that will be used for the ServiceJob tocalculate its results.
 * @memberof ServiceJob
 */
GRASSROOTS_SERVICE_API void SetServiceJobCalculateResultFunction (ServiceJob *job_p, bool (*calculate_fn) (ServiceJob *job_p));



/**
 * Update, if appropriate, a given ServiceJob.
 *
 * If the ServiceJob was previously in an unfinished state,
 * then check to see whether it has finished running.
 * If the ServiceJob had already finished, either successfully
 * or with errors, this function is a no-op.
 *
 * @param job_p The ServiceJob to update.
 * @return <code>true</code> if the ServiceJob was updated successfully,
 * <code>false</code> otherwise.
 * @memberof ServiceJob
 */
GRASSROOTS_SERVICE_API bool UpdateServiceJob (ServiceJob *job_p);


/**
 * Get the number of results stored on a ServiceJob.
 *
 * @param job_p The ServiceJob to check.
 * @return The number of results.
 * @memberof ServiceJob
 */
GRASSROOTS_SERVICE_API uint32 GetNumberOfServiceJobResults (const ServiceJob *job_p);


/**
 * Replace the set of results for a given ServiceJob.
 *
 * @param job_p The ServiceJob to amend.
 * @param results_p The new JSON fragment which will be the ServiceJob's results.
 * Any previous results will have the reference count decremented.
 * @return <code>true</code> if the ServiceJob was updated successfully, <code>false</code>
 * otherwise.
 * @memberof ServiceJob
 */
GRASSROOTS_SERVICE_API bool ReplaceServiceJobResults (ServiceJob *job_p, json_t *results_p);


/**
 * Add a LinkedService to ServiceJob.
 *
 * The ServiceJob needs to have its results set prior to calling this method
 *
 * @param job_p The ServiceJob to add the LinkedService details to.
 * @param linked_service_p The LinkedService details to update the ServiceJob with.
 * @return <code>true</code> if the ServiceJob was updated successfully,
 * <code>false</code> otherwise.
 * @memberof ServiceJob
 */
GRASSROOTS_SERVICE_API bool AddLinkedServiceToServiceJob (ServiceJob *job_p, struct LinkedService *linked_service_p);

/**
 * Attempt to extract the relevant data for all LinkedServices that the ServiceJob's Service has and
 * store them in the ServiceJob.
 *
 * @param job_p TheServiceJob to process.
 * @memberof ServiceJob
 * @see GenerateLinkedServiceResults
 */
GRASSROOTS_SERVICE_API void ProcessLinkedServices (ServiceJob *job_p);


/**
 * Calculate the results for the given ServiceJob.
 *
 * @param job_p
 * @return <code>true</code> if the results of the ServiceJob were
 * calculated successfully, <code>false</code> otherwise.
 * @memberof ServiceJob */
GRASSROOTS_SERVICE_API bool CalculateServiceJobResult (ServiceJob *job_p);


/**
 * Replace the uuid for a given ServiceJob.
 *
 * @param job_p The ServiceJob whose uuid will be changed.
 * @param new_job_id The new uuid.
 * @memberof ServiceJob
 */
GRASSROOTS_SERVICE_API void SetServiceJobUUID (ServiceJob *job_p, const uuid_t new_job_id);



GRASSROOTS_SERVICE_LOCAL int32 GetNumberOfLiveJobsFromServiceJobSet (const ServiceJobSet *jobs_p);



GRASSROOTS_SERVICE_LOCAL bool GetOperationStatusFromServiceJobJSON (const json_t *value_p, OperationStatus *status_p);


/**
 * For a given ServiceJob that is running multiple tasks, update its status to reflect
 * the given status from a task.
 *
 * @param job_p The ServiceJob to update.
 * @param status The OperationStatus from one of the tasks that the ServiceJob is running.
 */
GRASSROOTS_SERVICE_API void MergeServiceJobStatus (ServiceJob *job_p, OperationStatus status);


#ifdef __cplusplus
}
#endif



#endif /* SERVICE_JOB_H_ */
