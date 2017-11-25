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
#include "grassroots_config.h"
#include "json_util.h"
#include "streams.h"


#undef mongoc_client_t
#include "mongoc.h"



typedef struct MongoClientManager
{
	mongoc_client_pool_t *mcm_clients_p;
	mongoc_uri_t *mcm_uri_p;
} MongoClientManager;


static MongoClientManager *s_manager_p = NULL;

static MongoClientManager *AllocateMongoClientManager (const char *uri_s);

static void FreeMongoClientManager (MongoClientManager *manager_p);




static MongoClientManager *AllocateMongoClientManager (const char *uri_s)
{
	mongoc_uri_t *uri_p = mongoc_uri_new (uri_s);

	if (uri_p)
		{
			MongoClientManager *manager_p = (MongoClientManager *) AllocMemory (sizeof (MongoClientManager));

			if (manager_p)
				{
					mongoc_client_pool_t *clients_p = mongoc_client_pool_new (uri_p);

					if (clients_p)
						{
							manager_p -> mcm_clients_p = clients_p;
							manager_p -> mcm_uri_p = uri_p;

							return manager_p;
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

	return NULL;
}


static void FreeMongoClientManager (MongoClientManager *manager_p)
{
	mongoc_client_pool_destroy (manager_p -> mcm_clients_p);
	mongoc_uri_destroy (manager_p -> mcm_uri_p);

	FreeMemory (manager_p);
}


bool InitMongoClientManager (void)
{
	bool success_flag = false;

	if (!s_manager_p)
		{
			const json_t *mongo_config_p = GetGlobalConfigValue ("mongodb");

			if (mongo_config_p)
				{
					const char *uri_s = GetJSONString (mongo_config_p, "uri");

					if (uri_s)
						{
							MongoClientManager *manager_p = AllocateMongoClientManager (uri_s);

							if (manager_p)
								{
									s_manager_p = manager_p;
									success_flag = true;
								}
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create MongoClientManager for %s", uri_s);
								}
						}
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, mongo_config_p, "No uri in config");
						}

				}
			else
				{
					PrintLog (STM_LEVEL_INFO, __FILE__, __LINE__, "No mongodb in config");
					success_flag = true;
				}
		}
	else
		{
			success_flag = true;
		}

	return success_flag;
}


void ExitMongoClientManager (void)
{
	if (s_manager_p)
		{
			FreeMongoClientManager (s_manager_p);
			s_manager_p = NULL;
		}
}


mongoc_client_t *GetMongoClientFromMongoClientManager (void)
{
	mongoc_client_t *client_p = NULL;

	if (s_manager_p)
		{
			client_p = mongoc_client_pool_try_pop (s_manager_p -> mcm_clients_p);
		}

	return client_p;
}


void ReleaseMongoClientFromMongoClientManager (mongoc_client_t *client_p)
{
	if (s_manager_p)
		{
			mongoc_client_pool_push (s_manager_p -> mcm_clients_p, client_p);
		}
}

