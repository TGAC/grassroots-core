/*
** Copyright 2014-2021 The Earlham Institute
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
 * windows_uuid.h
 *
 *  Created on: 30 Jan 2020
 *      Author: billy
 */

#ifndef GRASSROOTS_WINDOWS_UUID_H
#define GRASSROOTS_WINDOWS_UUID_H

#include <rpcdce.h>


/*
 * We want to use the Unix uuid_t as the standard
 * and wrap a Windows equivalent and since the
 * structures differ slightly we need to untypedef
 * uuid_t for Windows
 */

#ifdef uuid_t
	#undef uuid_t
#endif

typedef struct
{
	unsigned char uu_data [16];
} uuid_t;


#ifdef __cplusplus
extern "C"
{
#endif

int uuid_parse (char *in, uuid_t uu);

#ifdef __cplusplus
}
#endif

#endif
