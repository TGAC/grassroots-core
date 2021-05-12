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
 * uuid_constants.h
 *
 *  Created on: 30 Apr 2021
 *      Author: billy
 */

#ifndef CORE_SHARED_UUID_INCLUDE_UUID_CONSTANTS_H_
#define CORE_SHARED_UUID_INCLUDE_UUID_CONSTANTS_H_

/**
 * The number of bytes required to store a c-style string
 * representation of a UUID.
 * @ingroup utility_group
 */
#define UUID_STRING_BUFFER_SIZE (37)

/**
 * The number of bytes required to store the raw data
 * of a UUID.
 * @ingroup utility_group
 */
#define UUID_RAW_SIZE (16)

/*
 * We want to use the Unix uuid_t as the standard
 * and wrap a Windows equivalent and since the
 * structures differ slightly we need to untypedef
 * uuid_t for Windows
 */

#ifdef _WIN32
	#include "windows_uuid.h"
#else
	#include "uuid/uuid.h"
#endif


#endif /* CORE_SHARED_UUID_INCLUDE_UUID_CONSTANTS_H_ */
