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
 * grassroots_server.h
 *
 *  Created on: 26 Jul 2019
 *      Author: billy
 */

#ifndef CORE_SERVER_SERVER_INCLUDE_GRASSROOTS_SERVER_H_
#define CORE_SERVER_SERVER_INCLUDE_GRASSROOTS_SERVER_H_

#include "grassroots_service_manager_library.h"

#include "jansson.h"


#include "servers_manager.h"
#include "schema_version.h"
#include "user_details.h"
#include "handler.h"

/*
 * forward declarations
 */
struct JobsManager;
struct ServersManager;
struct MongoClientManager;
struct Service;


typedef struct GrassrootsServer
{
	/**
	 * The path to the root directory for this GrassrootsServer.
	 */
	char *gs_path_s;

	/**
	 * The path to the global grassroots config file
	 * for this GrassrootsServer.
	 */
	char *gs_config_filename_s;

	/**
	 * The relative path from gs_path_s to the folder
	 * containing the individual service config files
	 */
	char *gs_config_path_s;


	/**
	 * The relative path from gs_path_s to the folder
	 * containing the reference services
	 */
	char *gs_references_path_s;

	/**
   * The relative path from gs_path_s to the folder
   * containing the service modules.
   */
	char *gs_services_path_s;


	/**
   * The relative path from gs_path_s to the folder
   * containing the jobs managers modules.
   */
	char *gs_jobs_managers_path_s;


	/**
   * The relative path from gs_path_s to the folder
   * containing the servers managers modules.
   */
  char *gs_servers_managers_path_s;



	struct JobsManager *gs_jobs_manager_p;

	MEM_FLAG gs_jobs_manager_mem;

	struct ServersManager *gs_servers_manager_p;

	MEM_FLAG gs_servers_manager_mem;

	json_t *gs_config_p;

	SchemaVersion *gs_schema_version_p;

	struct MongoClientManager *gs_mongo_manager_p;

//	struct PersistentServiceData *gs_persistent_service_data_p;
} GrassrootsServer;



#ifdef __cplusplus
extern "C"
{
#endif


/**
 * Allocate a GrassrootsServer
 *
 * @return The newly-created GrassrootsServer or <code>NULL</code> upon error.
 * @memberof GrassrootsServer
 * @ingroup server_group
 */
GRASSROOTS_SERVICE_MANAGER_API GrassrootsServer *AllocateGrassrootsServer (const char *grassroots_path_s, const char *config_filename_s, const char *service_config_path_s, const char *services_path_s, const char *references_path_s,
	const char *jobs_managers_path_s, const char *servers_managers_path_s, struct JobsManager *external_jobs_manager_p, MEM_FLAG jobs_manager_flag, struct ServersManager *external_servers_manager_p, MEM_FLAG servers_manager_flag);


/**
 * Free a GrassrootsServer.
 *
 * @param server_p The GrassrootsServer to free.
 * @memberof GrassrootsServer
 * @ingroup server_group
 */
GRASSROOTS_SERVICE_MANAGER_API void FreeGrassrootsServer (GrassrootsServer *server_p);



/**
 * Get the current Server-wide JobsManager.
 *
 * @return The JobsManager.
 * @memberof JobsManager
 */
GRASSROOTS_SERVICE_MANAGER_API struct JobsManager *GetJobsManager (GrassrootsServer *grassroots_p);



GRASSROOTS_SERVICE_MANAGER_API bool SetGrassrootsServerConfig (GrassrootsServer *server_p, const char *config_filename_s);



GRASSROOTS_SERVICE_MANAGER_API bool InitGrassrootsServer (GrassrootsServer *server_p);


/**
 * Connect to any defined separate Grassroots servers.
 * @ingroup server_group
 */
GRASSROOTS_SERVICE_MANAGER_API void ConnectToExternalServers (GrassrootsServer *server_p);


/**
 * Disconnect to any defined separate Grassroots servers.
 * @ingroup server_group
 */
GRASSROOTS_SERVICE_MANAGER_LOCAL void DisconnectFromExternalServers (GrassrootsServer *server_p);



/**
 * Load the Service that matches a given service name
 *
 * @param services_p The List of Services that the named Services will get appended to if it is found successfully.
 * @param services_path_s The directory where the Service modules are stored.
 * @param service_name_s The name of the Service to find.
 * @param user_p Any user configuration details, this can be <code>NULL</code>.
  * @memberof Service
 */
GRASSROOTS_SERVICE_MANAGER_API void LoadMatchingServicesByName (GrassrootsServer *grassroots_p, LinkedList *services_p, const char *service_name_s, const char *service_alias_s, User *user_p);


/**
 * Load the Services that are able to act upon a given Resource.
 *
 * @param services_p The List of Services that the named Services will get appended to if it is found successfully.
 * @param services_path_s The directory where the Service modules are stored.
 * @param resource_p The Resource to check for matching Services for.
 * @param handler_p The Handler that is appropriate for the given Resource.
 * @param user_p Any user configuration details, this can be <code>NULL</code>.
 * @memberof Service
 */
GRASSROOTS_SERVICE_MANAGER_API void LoadMatchingServices (GrassrootsServer *grassroots_p, LinkedList *services_p, DataResource *resource_p, Handler *handler_p, User *user_p);


/**
 * Load all Services that can be run upon a keyword parameter.
 *
 * @param services_p The List of Services that any keyword-aware Services will get appended to.
 * @param user_p Any user configuration details, this can be <code>NULL</code>.
 * @memberof Service
 */
GRASSROOTS_SERVICE_MANAGER_API void LoadKeywordServices (GrassrootsServer *grassroots_p, LinkedList *services_p, User *user_p);


/**
 * Process a given JSON request and produce the server response.
 *
 * @param server_p The GrassrootsServer that will process the request.
 * @param req_p The incoming JSON request.
 * @param user_p The user accessing Grassroots. This can be <code>NULL</code> for a public or non-logged in user.
 * @param error_ss A pointer to a variable where any error messages can be stored.
 * @return The response from the server or <code>NULL</code> upon error.
 * @memberof GrassrootsServer
 * @ingroup server_group
 */
GRASSROOTS_SERVICE_MANAGER_API json_t *ProcessServerJSONMessage (GrassrootsServer *grassroots_p, json_t *json_req_p, User *user_p, const char **error_ss);


/**
 * Create a response object with a valid header and a given key and value.
 *
 * @param server_p The GrassrootsServer that will process the request.
 * @param req_p If this object is not <code>NULL</code> and contains a "verbose" key set to true,
 * then the request will be added to a "request" key within the "header" section of this response.
 * This is to allow the tracking of requests to responses if needed.
 * @param key_s The key to use to add the associated value to the generated response.
 * @param value_p The value to add to the generated response.
 * @return The response or <code>NULL</code> upon error.
 * @see GetInitialisedMessage
 * @memberof GrassrootsServer
 * @ingroup server_group
 */
GRASSROOTS_SERVICE_MANAGER_API json_t *GetInitialisedResponseOnServer (GrassrootsServer *server_p, User *user_p, const json_t *req_p, const char *key_s, json_t *value_p);



/**
 * Get a configuration value from the global Grassroots configuration file.
 *
 * @param key_s The key to get the associated value for.
 * @return The JSON fragment containing the configuration data or <code>
 * NULL</code> if it could not be found.
 * @memberof GrassrootsServer
 * @ingroup server_group
 */
GRASSROOTS_SERVICE_MANAGER_API json_t *GetGlobalConfigValue (const GrassrootsServer *grassroots_p, const char *key_s);


/**
 * Get the Provider name for this Grassroots Server.
 *
 * @return The Provider name.
 * @ingroup server_group
 */
GRASSROOTS_SERVICE_MANAGER_API const char *GetServerProviderName (const GrassrootsServer *grassroots_p);


/**
 * Get the Provider description for this Grassroots Server.
 *
 * @return The Provider description.
 * @ingroup server_group
 */
GRASSROOTS_SERVICE_MANAGER_API const char *GetServerProviderDescription (const GrassrootsServer *grassroots_p);


/**
 * Get the Provider URI for this Grassroots Server.
 *
 * @return The Provider URI.
 * @ingroup server_group
 */
GRASSROOTS_SERVICE_MANAGER_API const char *GetServerProviderURI (const GrassrootsServer *grassroots_p);


/**
 * Get the JSON fragment with all of the details of the Provider
 * for this Grassroots Server.
 *
 * @return A read-only JSON representation of the Provider
 * or <code>NULL</code> upon error.
 * @ingroup server_group
 */
GRASSROOTS_SERVICE_MANAGER_API const json_t *GetProviderAsJSON (const GrassrootsServer *grassroots_p);


/**
 * Check if a named Service is enabled upon this Grassroots server.
 *
 * @param service_name_s The name of the Service to check.
 * @return <code>true</code> if the named Service is enabled, <code>false</code> otherwise.
 * @ingroup server_group
 */
GRASSROOTS_SERVICE_MANAGER_API bool IsServiceEnabled (const GrassrootsServer *grassroots_p, const char *service_name_s);



/**
 * Add any reference Services to the list of available Services.
 *
 * @param services_p The LinkedList of ServiceNodes to add any referred Services
 * to.
 * @param references_path_s The directory containing the JSON definitions of
 * reference services.
 * @param services_path_s The directory containing the Service plugins.
 * @param operation_name_s If this value is set, only referred Services that
 * have an Operation with this name will be added to Services list. If this is
 * <code>NULL</code> then all possible reference Services will be added.
 * @param user_p Any user configuration details, this can be <code>NULL</code>.
 * @memberof Service
 */
GRASSROOTS_SERVICE_MANAGER_API void AddReferenceServices (GrassrootsServer *grassroots_p, LinkedList *services_p,
                     const char *operation_name_s, User *user_p);


/**
 * Get the job logging URI for this Grassroots Server.
 *
 * @return The job logging URI.
 * @ingroup server_group
 */
GRASSROOTS_SERVICE_MANAGER_API const char *GetJobLoggingURI (const GrassrootsServer *grassroots_p);


GRASSROOTS_SERVICE_MANAGER_API const SchemaVersion *GetSchemaVersion (GrassrootsServer *server_p);


/**
 * Get the ServersManager for a given GrassrootsServer.
 *
 * @param server_p The GrassrootsServer to get the ServersManager from.
 * @return The ServersManager.
 * @memberof GrassrootsServer
 * @ingroup server_group
 */
GRASSROOTS_SERVICE_MANAGER_API struct ServersManager *GetServersManager (GrassrootsServer *server_p);






GRASSROOTS_SERVICE_MANAGER_API const char *GetServerRootDirectory (const GrassrootsServer * const grassroots_p);


/**
 * Get a configuration value for a named Service.
 *
 * The system will initially check the "config/<service_name>" folder
 * within the Grassroots directory e.g. "config/BlastN Service" for the
 * Service called "BlastN Service". If this file exists it will be loaded
 * and produce the configuration value. If it does not exist or cannot be
 * loaded then, the system will attempt to use an object specified at
 * "services.<service_name>" in the global configuration file instead.
 *
 * @param service_name_s The name of the Service to get the configuration data for.
 * @param alloc_flag_p If the returned JSON fragment has been newly-allocated then
 * the value that this points to will be set to <code>true</code> to indicate that
 * the returned value will need to have json_decref called upon it to avoid a
 * memory leak.
 * @return The JSON fragment containing the configuration data or <code>
 * NULL</code> if it could not be loaded.
 * @memberof GrassrootsServer
 * @ingroup server_group
 */
GRASSROOTS_SERVICE_MANAGER_API json_t *GetGlobalServiceConfig (GrassrootsServer *grassroots_p, const char * const service_name_s, bool *alloc_flag_p);



/**
 * Get an existing User by the email address.
 *
 * @param grassroots_p The GrassrootsServer to search.
 * @param email_s The email address to find the user for.
 * @return The User or <code>NULL</code> upon error.
 * @memberof GrassrootsServer
 */
GRASSROOTS_SERVICE_MANAGER_API User *GetUserByEmailAddress (const GrassrootsServer *grassroots_p, const char *email_s);




/**
 * Get an existing User by the id.
 *
 * @param grassroots_p The GrassrootsServer to search.
 * @param id_s The bson id, as a string, of the User to find.
 * @return The User or <code>NULL</code> upon error.
 * @memberof GrassrootsServer
 */
GRASSROOTS_SERVICE_MANAGER_API User *GetUserByIdString (const GrassrootsServer *grassroots_p, const char *id_s);



/**
 * Get an existing User by the id.
 *
 * @param grassroots_p The GrassrootsServer to search.
 * @param id_p The bson id of the User to find.
 * @return The User or <code>NULL</code> upon error.
 * @memberof GrassrootsServer
 */
GRASSROOTS_SERVICE_MANAGER_API User *GetUserById (const GrassrootsServer *grassroots_p, const bson_oid_t *id_p);


/**
 * Get a list of all Users
 *
 * @param grassroots_p The GrassrootsServer to search.
 * @return The list of Users or <code>NULL</code> upon error.
 * @memberof GrassrootsServer
 */
GRASSROOTS_SERVICE_MANAGER_API LinkedList *GetAllUsers (const GrassrootsServer *grassroots_p);


/**
 * Get a Service by its name.
 *
 * This will match a Service with its response from getting GetServiceName.
 *
 * @param service_name_s The name of the Service to find.
 * @return The matching Service or <code>NULL</code> if it could not be found.
 * @memberof Service
 */
GRASSROOTS_SERVICE_MANAGER_API struct Service *GetServiceByName (GrassrootsServer *grassroots_p, const char *const service_name_s, const char *const service_alias_s);



#ifdef __cplusplus
}
#endif

#endif /* CORE_SERVER_SERVER_INCLUDE_GRASSROOTS_SERVER_H_ */
