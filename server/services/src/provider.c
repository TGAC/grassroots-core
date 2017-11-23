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
 * provider.c
 *
 *  Created on: 15 Sep 2015
 *      Author: tyrrells
 */

#include "provider.h"

#include "json_tools.h"
#include "json_util.h"


const char *GetProviderName (const json_t * const data_p)
{
	return GetJSONString (data_p, PROVIDER_NAME_S);
}


const char *GetProviderURI (const json_t * const data_p)
{
	return GetJSONString (data_p, PROVIDER_URI_S);
}


const char *GetProviderDescription (const json_t * const data_p)
{
	return GetJSONString (data_p, PROVIDER_DESCRIPTION_S);
}


const char *GetProviderLogo (const json_t * const data_p)
{
	return GetJSONString (data_p, PROVIDER_LOGO_S);
}



const json_t *GetProviderDetails (const json_t *root_json_p)
{
	json_t *provider_p = json_object_get (root_json_p, SERVER_PROVIDER_S);

	if (!provider_p)
		{
			provider_p = json_object_get (root_json_p, SERVER_MULTIPLE_PROVIDERS_S);
		}

	return provider_p;
}


bool SetProviderType (json_t *provider_p)
{
	bool success_flag = true;
	const char * const TYPE_KEY_S = "@type";
	const char * const TYPE_VALUE_S = "so:Organization";

	const char *type_s = GetJSONString (provider_p, TYPE_KEY_S);

	if ((!type_s) || (stcrmp (TYPE_VALUE_S, type_s) != 0))
		{
			if (json_object_set_new (provider_p, TYPE_KEY_S, json_string (TYPE_VALUE_S)) != 0)
				{
					success_flag = false;
				}
		}

	return success_flag;
}

