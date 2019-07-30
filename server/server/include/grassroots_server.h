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


#include "jansson.h"

#include "jobs_manager.h"
#include "servers_pool.h"


typedef struct GrassrootsServer
{
	char *gs_path_s;

	char *gs_config_filename_s;

	JobsManager *gs_jobs_manager_p;

	MEM_FLAG gs_job_manager_mem;

	ServersManager *gs_servers_manager_p;

	MEM_FLAG gs_servers_manager_mem;

	json_t *gs_config_p;

	SchemaVersion *gs_schema_version_p;

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
GRASSROOTS_SERVICE_MANAGER_API GrassrootsServer *AllocateGrassrootsServer (const char *grassroots_path_s, const char *config_filename_s, JobsManager *external_jobs_manager_p, MEM_FLAG jobs_manager_flag, ServersManager *external_servers_manager_p, MEM_FLAG servers_manager_flag);


/**
 * Free a GrassrootsServer.
 *
 * @param server_p The GrassrootsServer to free.
 * @memberof GrassrootsServer
 * @ingroup server_group
 */
GRASSROOTS_SERVICE_MANAGER_API void FreeGrassrootsServer (GrassrootsServer *server_p);




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



GRASSROOTS_SERVICE_MANAGER_API const SchemaVersion *GetSchemaVersion (GrassrootsServer *server_p);


/**
 * Get the ServersManager for a given GrassrootsServer.
 *
 * @param server_p The GrassrootsServer to get the ServersManager from.
 * @return The ServersManager.
 * @memberof GrassrootsServer
 * @ingroup server_group
 */
GRASSROOTS_SERVICE_MANAGER_API ServersManager *GetServersManager (GrassrootsServer *server_p);



#ifdef __cplusplus
}
#endif

#endif /* CORE_SERVER_SERVER_INCLUDE_GRASSROOTS_SERVER_H_ */
