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
 * mongo_db_util.h
 *
 *  Created on: 20 Jul 2015
 *      Author: billy
 */

#ifndef MONGO_DB_UTIL_H_
#define MONGO_DB_UTIL_H_

#include "mongodb_library.h"
#include "grassroots_server.h"
#include "typedefs.h"


#ifdef __cplusplus
	extern "C" {
#endif

/**
 * When the Grassroots system starts up, iniitialise
 * and global MongoDB resources that it needs.
 *
 * @return <code>true</code> if the global MongoDB resources
 * were acquired successfully,
 * <code>false</code> otherwise.
 * @ingroup server_group
 */
GRASSROOTS_MONGODB_API bool InitMongoDB (void);


/**
 * Free any MongoDB resources that the Grassroots system
 * obtained when it was started.
 * @ingroup server_group
 */
GRASSROOTS_MONGODB_API void ExitMongoDB (void);


#ifdef __cplusplus
}
#endif


#endif /* MONGO_DB_UTIL_H_ */
