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
/*
 * mongodb_util.c
 *
 *  Created on: 20 Jul 2015
 *      Author: billy
 */

#include "mongodb_util.h"
#include "memory_allocations.h"
#include "json_tools.h"
#include "grassroots_server.h"
#include "json_util.h"
#include "streams.h"
#include "mongo_client_manager.h"
#include "mongoc.h"
#include "mongodb_tool.h"


bool InitMongoDB (void)
{
	mongoc_init ();

	return true;
}


void ExitMongoDB (void)
{
	mongoc_cleanup ();
}




bool PrepareSaveData (bson_oid_t **id_pp, bson_t **selector_pp)
{
	bool success_flag = false;

	if (*id_pp)
		{
			*selector_pp = BCON_NEW (MONGO_ID_S, BCON_OID (*id_pp));

			if (*selector_pp)
				{
					success_flag = true;
				}
		}
	else
		{
			if ((*id_pp = GetNewBSONOid ()) != NULL)
				{
					success_flag = true;
				}
		}

	return success_flag;
}
