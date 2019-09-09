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


/*
 * forward declarations
 */
struct JobsManager;
struct ServersManager;
struct MongoClientManager;



typedef struct GrassrootsServer
{
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


	struct JobsManager *gs_jobs_manager_p;

	MEM_FLAG gs_jobs_manager_mem;

	struct ServersManager *gs_servers_manager_p;

	MEM_FLAG gs_servers_manager_mem;

	json_t *gs_config_p;

	SchemaVersion *gs_schema_version_p;

	struct MongoClientManager *gs_mongo_manager_p;
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
GRASSROOTS_SERVICE_MANAGER_API GrassrootsServer *AllocateGrassrootsServer (const char *grassroots_path_s, const char *config_filename_s, const char *service_config_path_s, const char *references_path_s, struct JobsManager *external_jobs_manager_p, MEM_FLAG jobs_manager_flag, struct ServersManager *external_servers_manager_p, MEM_FLAG servers_manager_flag);


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
 * Construct a response message based upon a client's message.
 *
 * This will attempt to create a JSON object from the incoming message
 * and call ProcessServerJSONMessage.
 *
 * @param server_p The GrassrootsServer that will process the request.
 * @param request_s The message from the client.
 * @param error_ss A pointer to a variable where any error messages can be stored.
 * @return The response from the server or <code>NULL</code> upon error.
 * @see ProcessServerJSONMessage
 * @memberof GrassrootsServer
 * @ingroup server_group
 */
GRASSROOTS_SERVICE_MANAGER_API json_t *ProcessServerRawMessage (GrassrootsServer *server_p, const char * const request_s, const char **error_ss);




/**
 * Process a given JSON request and produce the server response.
 *
 * @param server_p The GrassrootsServer that will process the request.
 * @param req_p The incoming JSON request.
 * @param error_ss A pointer to a variable where any error messages can be stored.
 * @return The response from the server or <code>NULL</code> upon error.
 * @memberof GrassrootsServer
 * @ingroup server_group
 */
GRASSROOTS_SERVICE_MANAGER_API json_t *ProcessServerJSONMessage (GrassrootsServer *server_p, json_t *req_p, const char **error_ss);




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
GRASSROOTS_SERVICE_MANAGER_API json_t *GetInitialisedResponseOnServer (GrassrootsServer *server_p, const json_t *req_p, const char *key_s, json_t *value_p);



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



#ifdef __cplusplus
}
#endif

#endif /* CORE_SERVER_SERVER_INCLUDE_GRASSROOTS_SERVER_H_ */
