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
 * mongo_client_manager.h
 *
 *  Created on: 30 Mar 2017
 *      Author: billy
 *
 * @file
 * @brief
 */

#ifndef CORE_SERVER_MONGODB_INCLUDE_MONGO_CLIENT_MANAGER_H_
#define CORE_SERVER_MONGODB_INCLUDE_MONGO_CLIENT_MANAGER_H_

#include "typedefs.h"
#include "mongodb_library.h"
#include "grassroots_server.h"

/* forward declarations */
struct MongoClientManager;
struct _mongoc_client_t;





#ifdef __cplusplus
extern "C"
{
#endif



GRASSROOTS_MONGODB_API struct MongoClientManager *AllocateMongoClientManager (const char *uri_s);


GRASSROOTS_MONGODB_API void FreeMongoClientManager (struct MongoClientManager *manager_p);


GRASSROOTS_MONGODB_API struct _mongoc_client_t *GetMongoClientFromMongoClientManager (struct MongoClientManager *manager_p);


GRASSROOTS_MONGODB_API void ReleaseMongoClientFromMongoClientManager (struct MongoClientManager *manager_p, struct _mongoc_client_t *client_p);


#ifdef __cplusplus
}
#endif


#endif /* CORE_SERVER_MONGODB_INCLUDE_MONGO_CLIENT_MANAGER_H_ */
