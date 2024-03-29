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
#ifndef IRODS_CONNECT_H
#define IRODS_CONNECT_H


#include "jansson.h"
#include "irods_util_library.h"
#include "user_details.h"

/* forward declaration */
struct IRodsConnection;

#ifdef __cplusplus
extern "C" 
{
#endif


/**
 * Create a connection to a local iRODS server.
 *
 * @param user_p The UserDetails to get the required credentials from to create
 * this IRodsConnection.
 * @return The connection to the iRODS server or <code>NULL</code> upon error.
 * @memberof IRodsConnection
 */
IRODS_UTIL_API struct IRodsConnection *CreateIRodsConnectionFromUserDetails (const User *user_p);


/**
 * Create a connection to a local iRODS server.
 *
 * @param auth_p The UserAuthentication to use to get the required credentials from to create
 * this IRodsConnection.
 * @return The connection to the iRODS server or <code>NULL</code> upon error.
 * @memberof IRodsConnection
 */
IRODS_UTIL_API struct IRodsConnection *CreateIRodsConnection (UserAuthentication *auth_p);


/**
 * Close a connection to an iRODS server.
 *
 * @param connection_p The connection to close.
 * @memberof IRodsConnection
 */
IRODS_UTIL_API void FreeIRodsConnection (struct IRodsConnection *connection_p);




#ifdef __cplusplus
}
#endif

#endif		/* #ifndef IRODS_CONNECT_H */
