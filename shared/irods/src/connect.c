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

#include "rcConnect.h"

#include "connect.h"
#include "json_util.h"
#include "streams.h"
#include "irods_connection.h"



#ifdef _DEBUG
	#define CONNECT_DEBUG	(STM_LEVEL_FINE)
#else
	#define CONNECT_DEBUG	(STM_LEVEL_NONE)
#endif


/***********************************************/

static IRodsConnection *AllocateIRodsConnection (rcComm_t *irods_comm_p);

static bool ReleaseIRodsConnection (rcComm_t *connection_p);


/***********************************************/


void FreeIRodsConnection (struct IRodsConnection *connection_p)
{
	ReleaseIRodsConnection (connection_p -> ic_connection_p);

	FreeMemory (connection_p);
}


static bool ReleaseIRodsConnection (rcComm_t *connection_p)
{
	int status = rcDisconnect (connection_p);

	if (status == 0)
		{
			return true;
		}
	else
		{
			PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to disconnect from iRODS: %d", status);
			return false;
		}
}


IRodsConnection *CreateIRodsConnectionFromUserDetails (const UserDetails *user_p)
{
	IRodsConnection *connection_p = NULL;

	UserAuthentication *auth_p = GetUserAuthenticationForSystem (user_p, "irods");

	if (auth_p)
		{
			connection_p = CreateIRodsConnection (auth_p);
		}

	return connection_p;
}


IRodsConnection *CreateIRodsConnection (UserAuthentication *auth_p)
{
	IRodsConnection *connection_p = NULL;
	rodsEnv env;
	rErrMsg_t err;

	int status = getRodsEnv (&env);

	if (status == 0) 
		{
			rcComm_t *comm_p = rcConnect (env.rodsHost, env.rodsPort, auth_p -> ua_username_s, env.rodsZone, 0, &err);
			
			if (comm_p)
				{
					status = clientLoginWithPassword (comm_p, auth_p -> ua_password_s);
					
					if (status == 0)
						{
							connection_p = AllocateIRodsConnection (comm_p);
						}
					else
						{
							ReleaseIRodsConnection (comm_p);
						}
				}
		}
		
	return connection_p;
}


/*******************************/

static IRodsConnection *AllocateIRodsConnection (rcComm_t *irods_comm_p)
{
	IRodsConnection *connection_p = (IRodsConnection *) AllocMemory (sizeof (IRodsConnection));

	if (connection_p)
		{
			connection_p -> ic_connection_p = irods_comm_p;
		}

	return connection_p;
}

                                                                                                      
