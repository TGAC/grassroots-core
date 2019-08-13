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
#include "system_util.h"

#include "curl/curl.h"

#include "handler_utils.h"
#include "streams.h"
#include "jobs_manager.h"
#include "string_utils.h"
#include "mongodb_util.h"
#include "servers_pool.h"

#ifdef DRMAA_ENABLED
#include "drmaa_util.h"
#endif


#ifdef IRODS_ENABLED
#include "irods_util.h"
#endif





bool InitInformationSystem (void)
{
	bool res_flag = false;

	if (InitHandlerUtil ())
		{
			if (InitDefaultOutputStream ())
				{
					CURLcode c = curl_global_init (CURL_GLOBAL_DEFAULT);

					if (c == 0)
						{
							if (InitMongoDB ())
								{
									res_flag = true;

									#ifdef DRMAA_ENABLED
									if (res_flag)
										{
											res_flag = InitDrmaaEnvironment ();
										}
									#endif

								}		/* if (InitMongoDB ()) */
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "InitMongoDB failed");
								}

						}		/* if (c == 0) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "curl_global_init failed with code %d", c);
						}

				}		/* if (InitDefaultOutputStream ()) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "InitDefaultOutputStream failed");
				}

		}		/* if (InitHandlerUtil ()) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "InitHandlerUtil failed");
		}

	return res_flag;
}


bool DestroyInformationSystem (void)
{
	bool res_flag = true;

	#ifdef DRMAA_ENABLED
	ExitDrmaaEnvironment ();
	#endif

	//FreeExternalServers ();
	ExitMongoDB ();


	curl_global_cleanup ();
	
//	FreeDefaultOutputStream ();
	DestroyHandlerUtil ();



	return res_flag;
}

