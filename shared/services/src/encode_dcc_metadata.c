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
 * encode_dcc_metadata.c
 *
 *  Created on: 14 Apr 2016
 *      Author: tyrrells
 */

#include "../../../shared/services/include/encode_dcc_metadata.h"




bool AddEncodeDDCContext (json_t *service_json_p, json_t *contexts_p)
{
	if (json_object_set_new (contexts_p, "encode-dcc", json_string ("https://github.com/ENCODE-DCC/encoded")) == 0)
		{
			return true;

		}		/* if (json_object_set_new (contexts_p, "encode-dcc", json_string ("https://github.com/ENCODE-DCC/encoded")) == 0) */

	return false;
}