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

#include <string.h>

#include "server.h"
#include "linked_list.h"
#include "jansson.h"
#include "json_tools.h"
#include "json_util.h"
#include "service.h"
#include "time_util.h"
#include "operation.h"
#include "query.h"
#include "connect.h"
#include "user.h"
#include "key_value_pair.h"
#include "service.h"
#include "service_matcher.h"
#include "service_config.h"

#include "handler.h"
#include "handler_utils.h"

#include "math_utils.h"
#include "string_utils.h"
#include "jobs_manager.h"
#include "parameter_set.h"

#include "servers_pool.h"
#include "uuid/uuid.h"

//#include "irods_handle.h"
#include "providers_state_table.h"
#include "grassroots_config.h"
#include "provider.h"


#ifdef _DEBUG
#define SERVER_DEBUG	(STM_LEVEL_FINE)
#else
#define SERVER_DEBUG	(STM_LEVEL_NONE)
#endif


/*****************************/
/***** STATIC PROTOTYPES *****/
/*****************************/



#if IRODS_ENABLED == 1
static json_t *GetAllModifiedData (const json_t * const req_p, UserDetails *user_p);
#endif



static json_t *GenerateServiceIndexingData (LinkedList *services_p, const json_t * const req_p, UserDetails *user_p, ProvidersStateTable *providers_p);

static json_t *GenerateNamedServices (LinkedList *services_p, const json_t * const req_p, UserDetails *user_p, ProvidersStateTable *providers_p);

static int32 AddPairedServices (Service *internal_service_p, UserDetails *user_p, ProvidersStateTable *providers_p);

static int32 AddAllPairedServices (LinkedList *internal_services_p, UserDetails *user_p, ProvidersStateTable *providers_p);




/***************************/
/***** API DEFINITIONS *****/
/***************************/








/******************************/
/***** STATIC DEFINITIONS *****/
/******************************/










#if IRODS_ENABLED == 1
static json_t *GetAllModifiedData (const json_t * const req_p, UserDetails *user_p)
{
	json_t *res_p = NULL;
	const char *username_s = NULL;
	const char *password_s = NULL;
	const char *token_s = NULL;
	const UserAuthentication *user_auth_p = GetUserAuthenticationForSystem (user_p, PROTOCOL_IRODS_S);


	if (user_auth_p)
		{
			const char *from_s = NULL;
			const char *to_s = NULL;
			/* "from" defaults to the start of time */
			time_t from = 0;

			/* "to" defaults to now */
			time_t to = time (NULL);

			json_t *group_p = json_object_get (req_p, "");

			if (group_p)
				{
					json_t *interval_p = json_object_get (group_p, "");

					if (interval_p)
						{
							from_s = GetJSONString (interval_p, "from");
							to_s = GetJSONString (interval_p, "to");
						}
				}

			if (from_s)
				{
					if (!ConvertCompactStringToEpochTime (from_s, &from))
						{
							// error
						}
				}

			if (to_s)
				{
					if (!ConvertCompactStringToEpochTime (to_s, &to))
						{
							// error
						}
				}

			res_p = GetModifiedIRodsFiles (username_s, password_s, from, to);
		}

	return res_p;
}
#endif




