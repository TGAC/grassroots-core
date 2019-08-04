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
 * mongo_client_manager.c
 *
 *  Created on: 30 Mar 2017
 *      Author: billy
 *
 * @file
 * @brief
 */



#include "mongo_client_manager.h"
#include "memory_allocations.h"
#include "json_tools.h"
#include "grassroots_server.h"
#include "json_util.h"
#include "streams.h"


#include "mongoc.h"

typedef struct MongoClientManager
{
	mongoc_client_pool_t *mcm_clients_p;
	mongoc_uri_t *mcm_uri_p;
} MongoClientManager;




MongoClientManager *AllocateMongoClientManager (const char *uri_s)
{
	mongoc_uri_t *uri_p = NULL;

	uri_p = mongoc_uri_new (uri_s);

	if (uri_p)
		{
			MongoClientManager *manager_p = (MongoClientManager *) AllocMemory (sizeof (MongoClientManager));

			if (manager_p)
				{
					mongoc_client_pool_t *clients_p = mongoc_client_pool_new (uri_p);

					if (clients_p)
						{
							const int ERROR_API_LEVEL = 2;

							if (mongoc_client_pool_set_error_api (clients_p, ERROR_API_LEVEL))
								{
									const char * const APP_NAME_S = "grassroots";

									manager_p -> mcm_clients_p = clients_p;
									manager_p -> mcm_uri_p = uri_p;

									if (!mongoc_client_pool_set_appname  (clients_p, APP_NAME_S))
										{
											PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to set mongodb client pool app name to %s", APP_NAME_S);
										}

									return manager_p;
								}
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to set mongodb client pool error api to %d", ERROR_API_LEVEL);
								}

							mongoc_client_pool_destroy (clients_p);
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create mongodb client pool for %s", uri_s);
						}
				}

			mongoc_uri_destroy (uri_p);
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get mongodb uri for %s", uri_s);
		}


	mongoc_cleanup ();
	return NULL;
}


void FreeMongoClientManager (MongoClientManager *manager_p)
{
	mongoc_client_pool_destroy (manager_p -> mcm_clients_p);
	mongoc_uri_destroy (manager_p -> mcm_uri_p);

	mongoc_cleanup ();
	FreeMemory (manager_p);
}



mongoc_client_t *GetMongoClientFromMongoClientManager (MongoClientManager *manager_p)
{
	mongoc_client_t *client_p = mongoc_client_pool_try_pop (manager_p -> mcm_clients_p);

	return client_p;
}


void ReleaseMongoClientFromMongoClientManager (MongoClientManager *manager_p, mongoc_client_t *client_p)
{
	mongoc_client_pool_push (manager_p -> mcm_clients_p, client_p);
}

