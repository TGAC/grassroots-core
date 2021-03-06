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
 * irods_connection.h
 *
 *  Created on: 18 Feb 2016
 *      Author: tyrrells
 */

#ifndef SERVER_LIB_INCLUDE_IRODS_CONNECTION_H_
#define SERVER_LIB_INCLUDE_IRODS_CONNECTION_H_


#include "rcConnect.h"

/**
 * This is a wrapper which hides the internals
 * of the code and objects needed to access an
 * iRODS system. This means that you can interact
 * with the iRODS server more easily.
 *
 * @ingroup irods_group
 */
typedef struct IRodsConnection
{
	/**
	 * The underlying connection to and
	 * iRODS server.
	 */
	rcComm_t *ic_connection_p;
} IRodsConnection;


#endif /* SERVER_LIB_INCLUDE_IRODS_CONNECTION_H_ */
