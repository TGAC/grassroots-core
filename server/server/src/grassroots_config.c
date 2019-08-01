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
#include "grassroots_config.h"
#include "servers_pool.h"
#include "streams.h"
#include "json_util.h"
#include "service_config.h"
#include "string_utils.h"
#include "filesystem_utils.h"
#include "json_tools.h"
#include "provider.h"


static json_t *s_config_p = NULL;
static bool s_load_config_tried_flag = false;


static const char *GetProviderElement (const char * const element_s);



bool InitConfig (void)
{
	InitSchemaVersionDetails ();

	return (GetConfig () != NULL);
}


bool DestroyConfig (void)
{
	bool success_flag = true;

	if (s_config_p)
		{
			json_decref (s_config_p);
		}

	ClearDefaultSchemaVersionDetails ();

	return success_flag;
}


json_t *GetGlobalConfigValue (const char *key_s)
{
	json_t *value_p = NULL;
	const json_t *config_p = GetConfig ();

	if (config_p)
		{
			value_p = json_object_get (config_p, key_s);
		}

	return value_p;
}



const char *GetServerProviderName (void)
{
	return GetProviderElement (PROVIDER_NAME_S);
}


const char *GetServerProviderDescription (void)
{
	return GetProviderElement (PROVIDER_DESCRIPTION_S);
}


const char *GetServerProviderURI (void)
{
	return GetProviderElement (PROVIDER_URI_S);
}


const json_t *GetProviderAsJSON (void)
{
	const json_t *provider_p = NULL;
	const json_t *config_p = GetConfig ();

	if (config_p)
		{
			provider_p = GetProviderDetails (config_p);
		}

	return provider_p;
}


const char *GetJobLoggingURI (void)
{
	const char *uri_s = NULL;
	const json_t *config_p = GetConfig ();

	if (config_p)
		{
			const json_t *jobs_p = GetCompoundJSONObject (config_p, "admin.jobs");

			if (jobs_p)
				{
					uri_s = GetJSONString (jobs_p, "uri");
				}
		}

	return uri_s;
}


bool IsServiceEnabled (const char *service_name_s)
{
	bool enabled_flag = true;
	const json_t *services_config_p = GetGlobalConfigValue (SERVICES_NAME_S);

	if (services_config_p)
		{
			const json_t *service_statuses_p = json_object_get (services_config_p, SERVICES_STATUS_S);

			if (service_statuses_p)
				{
					const json_t *service_p = json_object_get (service_statuses_p, service_name_s);

					GetJSONBoolean (service_statuses_p, SERVICES_STATUS_DEFAULT_S, &enabled_flag);

					if (service_p)
						{
							if (json_is_true (service_p))
								{
									enabled_flag = true;
								}
							else if (json_is_false (service_p))
								{
									enabled_flag = false;
								}

						}		/* if (service_p) */

				}		/* if (service_statuses_p) */

		}		/* if (services_config_p) */

	return enabled_flag;
}







static const char *GetProviderElement (const char * const element_s)
{
	const char *result_s = NULL;
	const json_t *provider_p = GetProviderAsJSON ();

	if (provider_p)
		{
			result_s = GetJSONString (provider_p, element_s);
		}

	return result_s;
}




