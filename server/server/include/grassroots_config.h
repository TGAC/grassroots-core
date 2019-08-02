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
 * grassroots_config.h
 *
 *  Created on: 3 May 2015
 *      Author: billy
 */

#ifndef GRASSROOTS_CONFIG_H_
#define GRASSROOTS_CONFIG_H_

#include "grassroots_service_manager_library.h"
#include "typedefs.h"
#include "jansson.h"
#include "schema_version.h"

#ifdef __cplusplus
	extern "C" {
#endif


/**
 * Get a configuration value from the global Grassroots configuration file.
 *
 * @param key_s The key to get the associated value for.
 * @return The JSON fragment containing the configuration data or <code>
 * NULL</code> if it could not be found.
 * @ingroup server_group
 */
GRASSROOTS_SERVICE_MANAGER_API json_t *GetGlobalConfigValue (const char *key_s);


/**
 * Get the Provider name for this Grassroots Server.
 *
 * @return The Provider name.
 * @ingroup server_group
 */
GRASSROOTS_SERVICE_MANAGER_API const char *GetServerProviderName (void);


/**
 * Get the Provider description for this Grassroots Server.
 *
 * @return The Provider description.
 * @ingroup server_group
 */
GRASSROOTS_SERVICE_MANAGER_API const char *GetServerProviderDescription (void);


/**
 * Get the Provider URI for this Grassroots Server.
 *
 * @return The Provider URI.
 * @ingroup server_group
 */
GRASSROOTS_SERVICE_MANAGER_API const char *GetServerProviderURI (void);


/**
 * Get the JSON fragment with all of the details of the Provider
 * for this Grassroots Server.
 *
 * @return A read-only JSON representation of the Provider
 * or <code>NULL</code> upon error.
 * @ingroup server_group
 */
GRASSROOTS_SERVICE_MANAGER_API const json_t *GetProviderAsJSON (void);


/**
 * Check if a named Service is enabled upon this Grassroots server.
 *
 * @param service_name_s The name of the Service to check.
 * @return <code>true</code> if the named Service is enabled, <code>false</code> otherwise.
 * @ingroup server_group
 */
GRASSROOTS_SERVICE_MANAGER_API bool IsServiceEnabled (const char *service_name_s);



/**
 * Get the job logging URI for this Grassroots Server.
 *
 * @return The job logging URI.
 * @ingroup server_group
 */
GRASSROOTS_SERVICE_MANAGER_API const char *GetJobLoggingURI (void);



#ifdef __cplusplus
}
#endif



#endif /* GRASSROOTS_CONFIG_H_ */
