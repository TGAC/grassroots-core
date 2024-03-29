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
#ifndef GRASSROOTS_SERVICE_H
#define GRASSROOTS_SERVICE_H

#include "grassroots_service_library.h"

#include "byte_buffer.h"
#include "linked_list.h"
#include "parameter_set.h"
#include "typedefs.h"
#include "handler.h"
#include "user_details.h"
#include "operation.h"
#include "paired_service.h"
#include "linked_service.h"
#include "uuid_defs.h"
#include "service_metadata.h"
#include "sync_data.h"

#include "jansson.h"

#include "providers_state_table.h"

struct ExternalServer;

/*
 * The following preprocessor macros allow us to declare
 * and define the variables in the same place. By default,
 * they will expand to 
 * 
 * 		extern const char *SERVICE_NAME_S;
 * 
 * however if ALLOCATE_JSON_TAGS is defined then it will 
 * become
 * 
 * 		const char *SERVICE_NAME_S = "path";
 * 
 * ALLOCATE_PATH_TAGS must be defined only once prior to
 * including this header file. Currently this happens in
 * service.c.
 */
#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifdef ALLOCATE_PATH_TAGS
	#define PATH_PREFIX GRASSROOTS_SERVICE_API
	#define PATH_VAL(x)	= x
#else
	#define PATH_PREFIX extern GRASSROOTS_SERVICE_API
	#define PATH_VAL(x)
#endif

#endif

/**
 * This specifies the relative filesystem path to where the Service
 * plugins are stored.
 */
//PATH_PREFIX const char *SERVICES_PATH_S PATH_VAL("services");


#define SERVICE_GROUP_ALIAS_SEPARATOR "-"

/**
 * A datatype detailing the addon services
 * that the grassroots offers. These are the
 * equivalent to iRods microservices.
 */

/* forward declarations */
struct Plugin;
struct Service;
struct ServiceJob;
struct ServiceJobSet;


/**
 * A datatype to define how a Service runs.
 *
 * @ingroup services_group
 */
typedef enum
{
	/** A task that runs synchronously. */
	SY_SYNCHRONOUS,          //!< SY_SYNCHRONOUS

	/**
	 * A task that runs asynchronously outside of the Grassroots system
	 * such as e.g drmaa jobs.
	 */
	SY_ASYNCHRONOUS_DETACHED,//!< SY_ASYNCHRONOUS_DETACHED

	/**
	 * A task runs asynchronously within the Grassroots system
	 * such as e.g threaded jobs.
	 */
	SY_ASYNCHRONOUS_ATTACHED //!< SY_ASYNCHRONOUS_ATTACHED
} Synchronicity;


/**
 * A datatype for holding the configuration data for a Service. This is normally
 * used as a base class.
 *
 * @ingroup services_group
 */
typedef struct ServiceData
{
	/** The service that owns this data. */
	struct Service *sd_service_p;

	/**
	 * If this service has an entry in global server config it will be stored here.
	 * The value in the service config will be "services" -> \<service_name\>
	 */
	json_t *sd_config_p;

	/**
	 * If this is true, then this ServiceData will decrease the reference count
	 * on sd_config_p  when this ServiceData is freed.
	 */
	bool sd_config_flag;
} ServiceData;




/**
 * A datatype which defines an available service, its capabilities and
 * its parameters.
 *
 * @ingroup services_group
 */
typedef struct Service
{
	/**
	 * The platform-specific plugin that the code for the Service is
	 * stored in.
	 */
	struct Plugin *se_plugin_p;

	/**
	 * @brief Is the service self-contained.
	 *
	 * If a Service has a specific task that it can perform then this
	 * is <code>true</code>. If it is a utility that is reused for different
	 * Services, such as a generic web search service, then this is <code>false</code>
	 */
	bool se_is_specific_service_flag;

	/**
	 * Run this Service.
	 *
	 * @param service_p The Service to run.
	 * @param param_set_p The ParameterSet to run the Service with.
	 * @param credentials_p An optional set of credentials if the Service requires it.
	 * @return A newly-allocated ServiceJobSet with the status of the ServiceJobs started
	 * by this call or <code>NULL</code> if there was an error.
	 * @see RunService
	 */
	struct ServiceJobSet *(*se_run_fn) (struct Service *service_p, ParameterSet *param_set_p, User *user_p, ProvidersStateTable *providers_p);

	/**
	 * Is the Service able to work upon the given Resource.
	 *
	 * @param service_p The Service to check.
	 * @param resource_p The Resource to check.
	 * @param handler_p A custom Handler for accessing the Resource. This can be NULL.
	 * @return If the Service can run on the given Resource, it returns a ParameterSet that
	 * is partially filled in using the value of the Resource. If the Service cannot run on
	 * the given Resource, this will return <code>NULL</code>.
	 * @memberof Service
	 */
	ParameterSet *(*se_match_fn) (struct Service *service_p, DataResource *resource_p, Handler *handler_p);

	/**
	 * Check whether the user have permissions to run the Service.
	 *
	 * @param service_p This Service.
	 * @param params_p The ParameterSet that the user wishes to use. If this is <code>NULL</code>
	 * which will check whether the user can run the Service with any ParameterSet. If this is set,
	 * it will also check whether the user can run the Service with the specific parameters e.g. access
	 * to a given input file.
	 * @param user_p The User for the user.
	 * @return <code>true</code> if the user has permissions, <code>false</code> otherwise.
	 */
	bool (*se_has_permissions_fn) (struct Service *service_p, ParameterSet *params_p, const User * const user_p);


 	/**
 	 * Function to get the user-friendly name of the service.
 	 */
	const char *(*se_get_service_name_fn) (const struct Service *service_p);

	/**
	 * Function to get the user-friendly description of the service.
	 */
	const char *(*se_get_service_description_fn) (const struct Service *service_p);


 	/**
 	 * Function to get the alias for a service.
 	 */
	const char *(*se_get_service_alias_fn) (const struct Service *service_p);

	/**
	 * Function to get a web address for more information about the service
	 */
	const char *(*se_get_service_info_uri_fn) (const struct Service *service_p);


	/**
	 * Function to get the ParameterSet for this Service.
	 */
	ParameterSet *(*se_get_params_fn) (struct Service *service_p, DataResource *resource_p, User *user_p);


	/**
	 * This function is used to customise any ServiceJob objects that this Service creates.
	 * It is often used to automatically set the <code>sj_update_fn</code> and <code>sj_free_fn</code>
	 * members of a ServiceJob.
	 *
	 * @param service_p This Service.
	 * @param job_p The ServiceJob to customise.
	 * @see ServiceJob
	 * @see SetServiceJobCustomFunctions
	 */
	void (*se_customise_service_job_fn) (struct Service *service_p, struct ServiceJob *job_p);


	/**
	 * Function to release the ParameterSet for this Service.
	 */
	void (*se_release_params_fn) (struct Service *service_p, ParameterSet *params_p);

	/**
	 * Function to close this Service
	 *
	 * @param service_p This Service.
	 * @return <code>true</code> if the Service was closed successfully,
	 * <code>false</code> otherwise.
	 */
	bool (*se_close_fn) (struct Service *service_p);


	/**
	 * Function to parse a JSON fragment from a previously serialised ServiceJob created by
	 * this Service.
	 *
	 * @param service_p A Service of the same type that the serialised ServiceJob was run by.
	 * @param service_job_json_p The JSON fragment representing the serialised ServiceJob.
	 * @return The newly-constructed ServiceJob or <code>NULL</code> upon error.
	 * @see CreateSerialisedServiceJobFromService
	 */
	struct ServiceJob *(*se_deserialise_job_json_fn) (struct Service *service_p, const json_t *service_job_json_p);


	/**
	 * Function to create a JSON fragment representing a serialised ServiceJob.
	 *
	 * @param service_p The Service that created the ServiceJob.
	 * @param service_job_p The ServiceJob to serialise.
	 * @return TThe JSON fragment representing the serialised ServiceJob or <code>NULL</code> upon error.
	 * @see CreateSerialisedJSONFromServiceJob
	 */
	json_t *(*se_serialise_job_json_fn) (struct Service *service_p, struct ServiceJob *service_job_p, bool omit_results_flag);

	/**
	 * Callback function used when processing the results from running ServiceJobs for this Service as
	 * input for any LinkedServices for this Service.
	 *
	 * @param service_p This Service.
	 * @param job_p The ServiceJob whose results will be processed.
	 * @param linked_service_p The LinkedService defining how to generate the Parameters for the Service
	 * to run based upon the given ServiceJob's results
	 * @return <code>true</code> if the LinkedService was processed successfully,
	 * <code>false</code> otherwise.
	 */
	bool (*se_process_linked_services_fn) (struct Service *service_p, struct ServiceJob *job_p, LinkedService *linked_service_p);


	/**
	 * Callback function used to get the ParameterType of a named Parameter used by the given Service.
	 *
	 * @param service_p This Service.
	 * @param param_name_s The name of the Parameter to get the ParameterType of.
	 * @param pt_p Pointer to the ParameterType where the value will be stored.
	 * @return <code>true</code> if the ParameterType was retrieved successfully,
	 * <code>false</code> otherwise.
	 */
	bool (*se_get_parameter_type_fn) (const struct Service *service_p, const char *param_name_s, ParameterType *pt_p);


	/**
	 * The synchronicity for how this Service runs.
	 */
	Synchronicity se_synchronous;

	/** Unique Id for this service */
	uuid_t se_id;

	/** The ServiceJobSet for this Service. */
	struct ServiceJobSet *se_jobs_p;

	/** Any remote Services that are paired with this Service. */
	LinkedList se_paired_services;

	/**
	 * Any LinkedServices that will use the output from running this Service
	 * as input for itself
	 */
	LinkedList se_linked_services;

	/**
	 * Any custom data that the service needs to store.
	 */
	ServiceData *se_data_p;

	/**
	 * A flag to specify whether this Service is currently running. Do not access this
	 * value directly, use the API methods IsServiceRunning() and SetServiceRunning()
	 * instead.
	 */
	bool se_running_flag;

	/**
	 * The ServiceMetadata for this Service.
	 */
	ServiceMetadata *se_metadata_p;


	/**
	 * Callback function used to create a Service's ServiceMetadata.
	 *
	 * @param service_p This Service.
	 * @return <code>true</code> if the Service's ServiceMetadata was created successfully,
	 * <code>false</code> otherwise.
	 */
	ServiceMetadata *(*se_get_metadata_fn) (struct Service *service_p);

	/**
	 * If the Service's synchronicity is set to SY_ASYNCHRONOUS_ATTACHED,
	 * this is used as mutex to control access to this Service when needed.
	 */
	SyncData *se_sync_data_p;

	/**
	 * A custom callback function to use to close the service if needed.
	 *
	 * @param service_p This Service.
	 */
	void (*se_release_service_fn) (struct Service *service_p);


	/**
	 * A custom callback function to get the data that can be be used to
	 * import into an indexing app like Lucene.
	 *
	 * @param service_p This Service.
	 */
	json_t *(*se_get_indexing_data_fn) (struct Service *service_p);


	/**
	 * If the service has to have a more complex method of
	 * creating parameters from json blobs than the standard method,
	 * use this custom callback function
	 */
	Parameter *(*se_custom_parameter_decoder_fn) (struct Service *service_p, json_t *param_json_p, const bool concise_flag);

	/**
	 * The server that this service is running on.
	 */
	GrassrootsServer *se_grassroots_p;



} Service;


/**
 * A datatype for storing Services on a LinkedList.
 *
 * @extends ListItem
 *
 * @ingroup services_group
 */
typedef struct
{
	/** The List Node */
	ListItem sn_node;

	/** The Service */
	Service *sn_service_p;
} ServiceNode;


/**
 * A datatype for having a set of Services.
 *
 * @ingroup services_group
 */
typedef struct ServicesArray
{
	/** An array of pointers to Services */
	Service **sa_services_pp;

	/** The number of Services in the array */
	uint32 sa_num_services;	
} ServicesArray;





#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Get the ServicesArray from a given Plugin.
 *
 * @param plugin_p The Plugin to get the ServicesArray from.
 * @param user_p Any user configuration details. This can be NULL.
 * @param service_config_p Any service configuration details.
 * @return The ServicesArray or <code>NULL</code> upon error.
 */
GRASSROOTS_SERVICE_API ServicesArray *GetReferrableServicesFromPlugin (Plugin * const plugin_p, User *user_p, const json_t *service_config_p);

/**
 * Get the ServicesArray from a given Plugin.
 *
 * @param plugin_p The Plugin to get the ServicesArray from.
 * @param user_p Any user configuration details. This can be NULL.
 * @return The ServicesArray or <code>NULL</code> upon error.
 */
GRASSROOTS_SERVICE_API ServicesArray *GetServicesFromPlugin (Plugin * const plugin_p, User *user_p);


/**
 * Initialise the basic structure of a Service.
 *
 * @param service_p The Service to initialise.
 * @param get_service_name_fn The callback function that the Service will call to get its name.
 * @param get_service_description_fn The callback function that the Service will call to get its description.
 * @param get_service_alias_fn The callback function that the Service will call to get its alias.
 * @param get_service_info_uri_fn The callback function that the Service will call to get a web address for more information about the
 * Service. This can be <code>NULL</code>.
 * @param run_fn The callback function that the Service will call to run itself.
 * @param match_fn The callback function that the Service will call to check whether it is an appropriate Service for
 * a given Resource.
 * @param get_parameters_fn The callback function that the Service will call to get its ParameterSet.
 * @param release_parameters_fn The callback function that the Service will call to free a ParameterSet returned from a previous call
 * to get its ParameterSet.
 * @param close_fn The callback function that the Service will call when the Service is closed.
 * @param customise_service_job_fn If the ServiceJobs that this Service uses are extended from the Base ServiceJob datatype,
 * this function is used to set them up. This can be <code>NULL</code>.
 * @param specific_flag <code>true</code> if this Service performs a specific analysis. For Services used by scripted reference Services
 * detailed by JSON configuration files, then this should be <code>false</code>.
 * @param synchronous The synchronicity for how this Service runs.
 * @param data_p The ServiceData for this Service.
 * @param get_metadata_fn The callback function that the Service will call to create its ServiceMetadata.
 * @param get_indexing_data_fn The callback function that the Service will call to get its JSON data for indexing..
 * @param grassroots_p The GrassrootsServer that this Service will run on.
 * @return <code>true</code> if the Service was set up successfully, <code>false</code> otherwise.
 * @memberof Service
 */
GRASSROOTS_SERVICE_API bool InitialiseService (Service * const service_p,
	const char *(*get_service_name_fn) (const Service *service_p),
	const char *(*get_service_description_fn) (const Service *service_p),
	const char *(*get_service_alias_fn) (const Service *service_p),
	const char *(*get_service_info_uri_fn) (const Service *service_p),
	struct ServiceJobSet *(*run_fn) (Service *service_p, ParameterSet *param_set_p, User *user_p, ProvidersStateTable *providers_p),
	ParameterSet *(*match_fn) (Service *service_p, DataResource *resource_p, Handler *handler_p),
	ParameterSet *(*get_parameters_fn) (Service *service_p, DataResource *resource_p, User *user_p),
	bool (*get_parameter_type_fn) (const Service *service_p, const char *param_name_s, ParameterType *pt_p),
	void (*release_parameters_fn) (Service *service_p, ParameterSet *params_p),
	bool (*close_fn) (Service *service_p),
	void (*customise_service_job_fn) (Service *service_p, struct ServiceJob *job_p),
 	bool specific_flag,
	Synchronicity synchronous,
	ServiceData *data_p,
	ServiceMetadata *(*get_metadata_fn) (Service *service_p),
	json_t *(*get_indexing_data_fn) (Service *service_p),
	GrassrootsServer *grassroots_p
);


/**
 * @brief Run a Service.
 *
 * This is a convenience wrapper around se_run_fn.
 *
 * @param service_p The Service to run.
 * @param param_set_p The ParameterSet to run the Service with.
 * @param user_p An optional set of User as json.
 * @param providers_p The ProvidersStateTable to be used by any RemoteServices.
 * @return A newly-allocated ServiceJobSet containing the details for the new jobs or
 * <code>NULL</code> upon error.
 * @memberof Service
 * @see se_run_fn
 */
GRASSROOTS_SERVICE_API struct ServiceJobSet *RunService (Service *service_p, ParameterSet *param_set_p, User *user_p, ProvidersStateTable *providers_p);


/**
 * Does the Service use the default ServiceJob serialisation/deserialisation when storing ServiceJobs in the
 * JobsManager or does it use its own specialised routines?
 *
 * @param service_p The Service to check.
 * @return <code>true</code> if the Service use custom routines, <code>false</code> if it uses the standard ones.
 * @memberof Service
 */
GRASSROOTS_SERVICE_API bool DoesServiceHaveCustomServiceJobSerialisation (struct Service *service_p);


/**
 * Function to parse a JSON fragment from a previously serialised ServiceJob.
 *
 * @param service_p A Service of the same type that the serialised ServiceJob was run by.
 * @param service_job_json_p The JSON fragment representing the serialised ServiceJob.
 * @return The newly-constructed ServiceJob or <code>NULL</code> upon error.
 * @see se_deserialise_job_fn
 * @memberof Service
 */
GRASSROOTS_SERVICE_API struct ServiceJob *CreateSerialisedServiceJobFromService (struct Service *service_p, const json_t *service_job_json_p);


/**
 * Function to create a JSON fragment representing a serialised ServiceJob.
 *
 * @param service_p A Service of the same type that the serialised ServiceJob was run by.
 * @param service_job_p The ServiceJob to serialise.
 * @param omit_results_flag If this is <code>true</code> then just the minimal status information for
 * the ServiceJob will be returned. If it is <code>false</code> then the job results will be included too if possible.
 * @return The JSON fragment representing the serialised ServiceJob or <code>NULL</code> upon error.
 * @see se_serialise_job_fn
 * @memberof Service
 */
GRASSROOTS_SERVICE_API json_t *CreateSerialisedJSONForServiceJobFromService (struct Service *service_p, struct ServiceJob *service_job_p, bool omit_results_flag);



/**
 * Is the Service able to work upon the given Resource.
 *
 * @param service_p The Service to check.
 * @param resource_p The Resource to check.
 * @param handler_p A custom Handler for accessing the Resource. This can be NULL.
 * @return If the Service can run on the given Resource, it returns a ParameterSet that
 * is partially filled in using the value of the Resource. If the Service cannot run on
 * the given Resource, this will return <code>NULL</code>.
 * @memberof Service
 * @see se_match_fn
 */
GRASSROOTS_SERVICE_API ParameterSet *IsServiceMatch (Service *service_p, DataResource *resource_p, Handler *handler_p);


/**
 * Get the user-friendly name of the service.
 *
 * @param service_p The Service to get the name for.
 * @return The name of Service.
 * @memberof Service
 */
GRASSROOTS_SERVICE_API const char *GetServiceName (const Service *service_p);


/**
 * Get the user-friendly description of the service.
 *
 * @param service_p The Service to get the description for.
 * @return The description of Service.
 * @memberof Service
 */
GRASSROOTS_SERVICE_API const char *GetServiceDescription (const Service *service_p);


/**
 * Get the alias of the service.
 *
 * @param service_p The Service to get the alias for.
 * @return The alias of Service.
 * @memberof Service
 */
GRASSROOTS_SERVICE_API const char *GetServiceAlias (const Service *service_p);


/**
 * Get a JSON document containing the data that can be parsed by an
 * indexing app such as Lucene.
 *
 * @param service_p The Service to get the data for.
 * @return The JSON document.
 * @memberof Service
 */
GRASSROOTS_SERVICE_API json_t *GetServiceIndexingData (Service *service_p);


/**
 * Get the address of a web page about the service.
 *
 * @param service_p The Service to get the description for.
 * @return The address of the page or NULL if there isn't one.
 * @memberof Service
 */
GRASSROOTS_SERVICE_API const char *GetServiceInformationURI (const Service *service_p);

/**
 * Get a newly-created ParameterSet describing the parameters for a given Service.
 *
 * @param service_p The Service to get the ParameterSet for.
 * @param resource_p This is the input to the Service and can be NULL.
 * @param user_p Any user authorisation details that the Service might need. This can be NULL.
 * @return The newly-created ParameterSet or <code>NULL</code> upon error. This
 * ParameterSet will need to be freed once it is no longer needed by calling FreeParameterSet.
 * @see FreeParameterSet.
 * @memberof Service
 */
GRASSROOTS_SERVICE_API ParameterSet *GetServiceParameters (Service *service_p, DataResource *resource_p,  User *user_p);



/**
 * Get the ParameterType of a named Parameter used by the given Service.
 *
 * @param service_p The Service to get the ParameterType from.
 * @param param_name_s The name of the Parameter to get the ParameterType of.
 * @param pt_p Pointer to the ParameterType where the value will be stored.
 * @return <code>true</code> if the ParameterType was retrieved successfully,
 * <code>false</code> otherwise.
 * @memberof Service
 */
GRASSROOTS_SERVICE_API bool GetParameterTypeForNamedParameter (const struct Service *service_p, const char *param_name_s, ParameterType *pt_p);


/**
 * Get the unique id of a service object.
 *
 * @param service_p The Service to get the id for.
 * @return The string of the id.
 * @memberof Service
 */
GRASSROOTS_SERVICE_API char *GetServiceUUIDAsString (Service *service_p);


/**
 * Free a Service and its associated Parameters and ServiceData.
 *
 * @param service_p The Service to free.
 * @memberof Service
 */
GRASSROOTS_SERVICE_API void FreeService (Service *service_p);


/**
 * For an asynchronous Service, tell it that it is no longer needed
 * by the Grassroots system.
 *
 * @param service_p The Service to release.
 * @memberof Service
 */
GRASSROOTS_SERVICE_API void ReleaseService (Service *service_p);



/**
 * For an asynchronous Service, set the function that will be called
 * if ReleaseService() is called.
 *
 * @param service_p The Service to set the release function for.
 * @param release_fn The release function to set for the given Service.
 * @memberof Service
 */

GRASSROOTS_SERVICE_API void SetServiceReleaseFunction (Service *service_p, void (*release_fn) (Service *service_p));


/**
 * Free and service-specific configuration data.
 *
 * If a Service has a separate configuration file this function will ensure
 * there isn't a memory leak when the Service is freed.
 *
 * @param data_p
 * @memberof ServiceData
 */
GRASSROOTS_SERVICE_LOCAL void ReleaseServiceData (ServiceData *data_p);

/**
 * Allocate a ServiceNode pointing to the given Service.
 *
 * @param service_p The Service to store on the ServiceNode.
 * @return A newly-allocated ServiceNode or <code>NULL</code> upon error.
 * @memberof ServiceNode
 */
GRASSROOTS_SERVICE_API ServiceNode *AllocateServiceNode (Service *service_p);


/**
 * @brief Free a ServiceNode.
 *
 * @param node_p The ServiceNode to free.
 * @memberof ServiceNode
 */
GRASSROOTS_SERVICE_API void FreeServiceNode (ListItem *node_p);


/**
 * @brief Close a Service
 *
 * @param service_p The Service to close.
 * @return <code>true</code> if the Service was closed successfully, <code>false</code> otherwise.
 * @memberof Service
 */
GRASSROOTS_SERVICE_API bool CloseService (Service *service_p);




/**
 * Generate a json-based description of a Service.
 *
 * @param service_p The Service to generate the description for.
 * @param resource_p An optional Resource for the Service to run on. This can be <code>
 * NULL</code>.
 * @param user_p Optional user configuration details. This can be NULL.
 * @param add_id_flag If this is <code>true</code> then the UUID of the Service will be added
 * to the returned JSON. If this is <code>false</code> then it will not.
 * @return The json-based representation of the Service or <code>NULL</code> if there was
 * an error.
 * @memberof Service
 */
GRASSROOTS_SERVICE_API json_t *GetServiceAsJSON (Service * const service_p, DataResource *resource_p, User *user_p, const bool add_id_flag);


/**
 * Get the URL of an icon image to use for a given Service.
 *
 * @param service_p The Service to get the icon for.
 * @return The URL of the image to use for the Service. This can be <code>NULL</code>.
 * @memberof Service
 */
GRASSROOTS_SERVICE_API const char *GetServiceIcon (const Service *service_p);



/**
 * Get the URL of a web page describing  a given Service.
 *
 * @param service_p The Service to get the page for.
 * @return The URL of the description page for the Service. This can be <code>NULL</code>.
 * @memberof Service
 */
GRASSROOTS_SERVICE_API const char *GetServiceInformationPage (const Service *service_p);


/**
 * Close a Services-based Plugin and free all of the Services.
 *
 * @param plugin_p The Plugin to free the Services from.
 * @return <code>true</code> if the Services were successfully released, <code>false</code> otherwise.
 * @memberof Service
 */
GRASSROOTS_SERVICE_API bool DeallocatePluginService (Plugin * const plugin_p);


/**
 * Get the JSON fragment for exposing a list of Services to any interested Clients
 * or ExternalServcers.
 *
 * @param services_list_p A LinkedList of ServiceNodes detailing the Services to generate the fragment for
 * @param resource_p The Resource of interest to run the Services with. This can be <code>NULL</code>.
 * @param user_p Any User for access o potentially restricted Services. This can be <code>NULL</code> for only
 * publicly accessible Services.
 * @param add_service_ids_flag If this is <code>true</code> then the UUID of the Service will be added
 * to the returned JSON. If this is <code>false</code> then it will not.
 * @param providers_p This is used to keep track of which ExternalServers and their Services have already been processed
 * when generating the JSON fragment.
 * @return The json-based representation of the Service or <code>NULL</code> if there was
 * an error.
 * @see GetServiceAsJSON
 * @memberof Service
 */
GRASSROOTS_SERVICE_API json_t *GetServicesListAsJSON (LinkedList *services_list_p, DataResource *resource_p, User *user_p, const bool add_service_ids_flag, ProvidersStateTable *providers_p);


/**
 * @brief Free a ParameterSet that was got from a call to <code>GetServiceParameters</code>.
 *
 * @param service_p The Service used for the previous call to <code>GetServiceParameters</code>
 * @param params_p The ParameterSet to free.s
 * @memberof Service
 * @see GetServiceParameters
 */
GRASSROOTS_SERVICE_API void ReleaseServiceParameters (Service *service_p, ParameterSet *params_p);


/**
 * Free a ServicesArray and each of its Services.
 *
 * @param services_p The ServicesArray to free.
 * @memberof ServicesArray
 */
GRASSROOTS_SERVICE_API void FreeServicesArray (ServicesArray *services_p);


/**
 * Allocate an empty ServicesArray.
 *
 * @param num_services The number of potential Services that the ServicesArray will hold.
 * @return A newly-allocated ServicesArray with space for the given number of Services or
 * <code>NULL</code> upon error.
 * @memberof ServicesArray
 */
GRASSROOTS_SERVICE_API ServicesArray *AllocateServicesArray (const uint32 num_services);


/**
 * Assign a given Plugin as the code that generated each Service within a ServicesArray.
 *
 * @param services_p The ServicesArray containing the Services to be amended.
 * @param plugin_p The Plugin to be set for each Service.
 * @memberof ServicesArray
 */
GRASSROOTS_SERVICE_LOCAL void AssignPluginForServicesArray (ServicesArray *services_p, Plugin *plugin_p);


/**
 * Add a JSON fragment to the response for a given Service that has been run.
 *
 * @param service_p The Service which will have its response amended.
 * @param result_json_p The JSON fragment to add.
 * @return <code>true</code> if the sService response was successfully updated, <code>false</code> otherwise.
 * @memberof Service
 */
GRASSROOTS_SERVICE_API bool AddServiceResponseHeader (Service *service_p, json_t *result_json_p);


/**
 * Get the ServicesArray generated from a Services reference file.
 *
 * @param config_p The JSON fragment loaded from a referemce file.
 * @param plugin_name_s The name of the Plugin to use.
 * @param get_service_fn The function used to generate each Service that will be placed in the resultant
 * ServicesArray.
 * @return The ServicesArray containing the generated Services or <code>NULL</code> upon error.
 * @memberof ServicesArray
 */
GRASSROOTS_SERVICE_API ServicesArray *GetReferenceServicesFromJSON (json_t *config_p, const char *plugin_name_s, Service *(*get_service_fn) (json_t *config_p, size_t i, GrassrootsServer *grassroots_p), GrassrootsServer *grassroots_p);


/**
 * @brief Get the OperationStatus for an operation in a Service.
 *
 * @param service_p The Service to query.
 * @param service_id The uuid_t for the Operation whose OperationStatus is wanted.
 * @return The OperationStatus for the given Operation
 * @memberof Service
 */
GRASSROOTS_SERVICE_API OperationStatus GetCurrentServiceStatus (Service *service_p, const uuid_t service_id);


/**
 * The default function for getting the status of a ServiceJob with the given service id.
 *
 * @param service_p The Service to query the status for.
 * @param service_id The UUID of the ServiceJob that the status is wanted.
 * @return The OperationStatus for the
 */
GRASSROOTS_SERVICE_LOCAL OperationStatus DefaultGetServiceStatus (Service *service_p, const uuid_t service_id);


/**
 * Create a new PairedService and add it to a Service.
 *
 * @param service_p The Service to add the PairedService to.
 * @param external_server_p The ExternalServer that the PairedService runs on.
 * @param remote_service_name_s The name of the PairedService on the ExternalServer.
 * @param op_p The JSON fragment to create the PairedService's ParameterSet from.
 * @param provider_p The JSON fragment representing the Provider.
 * @return <code>true</code> if the PairedService was created and added successfully, <code>false</code> otherwise.
 * @see AddPairedService.
 * @see AllocatePairedService.
 * @see CreateParameterSetFromJSON
 * @memberof Service
 */
GRASSROOTS_SERVICE_API bool CreateAndAddPairedService (Service *service_p, struct ExternalServer *external_server_p, const char *remote_service_name_s, const json_t *op_p, const json_t *provider_p);


/**
 * Add a PairedService to a Service.
 *
 * @param service_p The Service to add the PairedService to.
 * @param paired_service_p The PairedService to add.
 * @return <code>true</code> if the PairedService was added successfully, <code>false</code> otherwise.
 * @memberof Service
 */
GRASSROOTS_SERVICE_API bool AddPairedService (Service *service_p, PairedService *paired_service_p);


/**
 * Create and add a LinkedService to a Service.
 *
 * @param service_p The Service to add the LinkedService to.
 * @param service_config_p The json fragement defining the LinkedService to add.
 * @param grassroots_p The GrassrootsServer that this Service will run on.
 * @return <code>true</code> if the LinkedService was added successfully, <code>false</code> otherwise.
 * @memberof Service
 */
GRASSROOTS_SERVICE_API bool CreateAndAddLinkedService (Service *service_p, const json_t *service_config_p, GrassrootsServer *grassroots_p);


/**
 * Add a LinkedService to a Service.
 *
 * @param service_p The Service to add the LinkedService to.
 * @param paired_service_p The LinkedService to add.
 * @return <code>true</code> if the LinkedService was added successfully, <code>false</code> otherwise.
 * @memberof Service
 */
GRASSROOTS_SERVICE_API bool AddLinkedService (Service *service_p, LinkedService *paired_service_p);



/**
 * Get the JSON fragment for sending to a Grassroots Server detailing whether and how a user
 * wants a Service to run. This object needs to be within a JSON array which is what the Server
 * requires.
 *
 * @param service_name_s The name of the Service to get the JSON fragment for.
 * @param params_p The ParameterSet to use. If run_flag is false, then this can be <code>NULL</code>.
 * @param sv_p If you wish to create a JSON fragment for a different version of the Grassroots system,
 * then you can set this value to the version that you require. If this is <code>NULL</code>, then the
 * current version of the running Grassroots system will be used.
 * @param run_flag Whether the Service should be run or not.
 * @return The JSON fragment to be added to an array to send to the Server or <code>NULL
 * </code> upon error.
 * @memberof Service
 */
GRASSROOTS_SERVICE_API json_t *GetServiceRunRequest (const char * const service_name_s, const ParameterSet *params_p, const SchemaVersion *sv_p, const bool run_flag, const ParameterLevel level);


/**
 * Get the JSON fragment for sending to a Grassroots Server detailing how to update the
 * parameters for a Service
 *
 * @param service_name_s The name of the Service to get the JSON fragment for.
 * @param params_p The ParameterSet to use. If run_flag is false, then this can be <code>NULL</code>.
 * @param sv_p If you wish to create a JSON fragment for a different version of the Grassroots system,
 * then you can set this value to the version that you require. If this is <code>NULL</code>, then the
 * current version of the running Grassroots system will be used.
 * @param run_flag Whether the Service should be run or not.
 * @return The JSON fragment to be added to an array to send to the Server or <code>NULL
 * </code> upon error.
 * @memberof Service
 */
GRASSROOTS_SERVICE_API json_t *GetServiceRefreshRequest (const char * const service_name_s, const ParameterSet *params_p, const SchemaVersion *sv_p, const bool run_flag, const ParameterLevel level);


/**
 * Get the JSON fragment for sending to a Grassroots Server asking for a service to be refreshed.
 * This object needs to be within a JSON array which is what the Server
 * requires.
 *
 * @param service_name_s The name of the Service to get the JSON fragment for.
 * @param params_p The ParameterSet to use. If run_flag is false, then this can be <code>NULL</code>.
 * @param sv_p If you wish to create a JSON fragment for a different version of the Grassroots system,
 * then you can set this value to the version that you require. If this is <code>NULL</code>, then the
 * current version of the running Grassroots system will be used.
 * @param run_flag Whether the Service should be run or not.
 * @return The JSON fragment to be added to an array to send to the Server or <code>NULL
 * </code> upon error.
 * @memberof Service
 */
GRASSROOTS_SERVICE_API json_t *GetServiceRefreshRequest (const char * const service_name_s, const ParameterSet *params_p, const SchemaVersion *sv_p, const bool run_flag, const ParameterLevel level);



/**
 * Get the JSON fragment for a job result list registering that a Service
 * could be run against the user's parameters.
 *
 * This is of use when running a keyword search and specifying that a Service
 * knows of this keyword but cannot run on it directly. For example, a Blast
 * Service might know that the keyword refers to some if its databases.
 *
 * @param service_p The Service that has registered an interest in the given keyword.
 * @param keyword_s The keyword.
 * @param params_p The parameters to encode within the JSON fragment. This will be set up based upon the
 * keyword used to check whether the Service was interested.
 * @param full_definition_flag This should be set to <code>true</code> when a Server is exposing its Services
 * and <code>false</code> when a request to run a Service is being generated. See GetParameterAsJSON for more
 * information.
 * @return The JSON fragment to send to the Server or <code>NULL</code> upon error.
 * @see IsServiceMatch
 * @see GetParameterAsJSON
 * @memberof Service
 */
GRASSROOTS_SERVICE_API json_t *GetInterestedServiceJSON (Service *service_p, const char *keyword_s, const ParameterSet * const params_p, const bool full_definition_flag);


/**
 * This function is used to customise any ServiceJob objects that a given Service creates.
 * It is often used to automatically set the <code>sj_update_fn</code> and <code>sj_free_fn</code>
 * members of a ServiceJob.
 *
 * @param service_p The Service to set the custom ServiceJob functions for.
 * @param job_p The ServiceJob to customise.
 * @see ServiceJob
 * @see se_customise_service_job_fn
 * @memberof Service
 */
GRASSROOTS_SERVICE_API void SetServiceJobCustomFunctions (Service *service_p, struct ServiceJob *job_p);


/**
 * Check if a Service is currently running any ServiceJobs.
 *
 * @param service_p The Service to check.
 * @return <code>true</code> if the Service is running, <code>false</code> if it is not.
 * @memberof Service
 */
GRASSROOTS_SERVICE_API bool IsServiceRunning (Service *service_p);


/**
 * Set if a Service is currently running any ServiceJobs or not.
 *
 * @param service_p The Service to set.
 * @param b <code>true</code> if the Service is running, <code>false</code> if it is not.
 * @memberof Service
 */
GRASSROOTS_SERVICE_API void SetServiceRunning (Service *service_p, bool b);


/**
 * Set the values of a Service's ServiceMetadata object.
 *
 * @param service_p The Service to set.
 * @param category_p The top-level application category to use.
 * @param subcategory_p The application subcategory to use.
 * @return <code>true</code> if the ServiceMetadata was set successfully, <code>false</code> if it was not.
 * @memberof Service
 */
GRASSROOTS_SERVICE_API void SetMetadataForService (Service *service_p, SchemaTerm *category_p, SchemaTerm *subcategory_p);







GRASSROOTS_SERVICE_API bool SortServicesListByName (LinkedList *services_list_p);


GRASSROOTS_SERVICE_API json_t *GetBaseServiceDataAsJSON (Service * const service_p, User *user_p);


/**
 * Get the GrassrootsServer that the Service is running on.
 *
 * @param service_p The Service to query
 * @return The GrassrootsServer
 * @memberof Service
 */
GRASSROOTS_SERVICE_API GrassrootsServer *GetGrassrootsServerFromService (const Service * const service_p);


GRASSROOTS_SERVICE_API json_t *GetServiceRefreshRequestFromJSON (const char * const service_name_s, json_t *param_set_json_p, const SchemaVersion *sv_p, const bool run_flag, const ParameterLevel level);


GRASSROOTS_SERVICE_API json_t *GetServiceRunRequestFromJSON (const char * const service_name_s, json_t *param_set_json_p, const SchemaVersion *sv_p, const bool run_flag, const ParameterLevel level);


GRASSROOTS_SERVICE_API bool DefaultGetParameterTypeForNamedParameter (const char *param_name_s, ParameterType *pt_p, const NamedParameterType *params_p);


/**
 * Add a ServiceJob to a Service.
 *
 * @param service_p The Service to add the ServiceJob to.
 * @param job_p The ServiceJob to add.
 * @return <code>true</code> if the ServiceJob was added to the Service successfully,
 * <code>false</code> otherwise.
 */
GRASSROOTS_SERVICE_API bool AddServiceJobToService (Service *service_p, ServiceJob *job_p);


/**
 * Remove a ServiceJob from a Service.
 *
 * @param service_p The Service to remove the ServiceJob from.
 * @param job_p The ServiceJob to remove.
 * @return <code>true</code> if the ServiceJob was removed to the ServiceJob successfully,
 * <code>false</code> if could not as the ServiceJob is not a member of the Service
 */
GRASSROOTS_SERVICE_API bool RemoveServiceJobFromService (Service *service_p, ServiceJob *job_p);


#ifdef __cplusplus
}
#endif


#endif		/* #ifndef GRASSROOTS_SERVICE_H */
