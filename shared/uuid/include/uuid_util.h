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
 * uuid_util.h
 *
 *  Created on: 30 Apr 2021
 *      Author: billy
 */

#ifndef CORE_SHARED_UUID_INCLUDE_UUID_UTIL_H_
#define CORE_SHARED_UUID_INCLUDE_UUID_UTIL_H_


#include "uuid/uuid.h"


#include "uuid_util_library.h"
#include "typedefs.h"
#include "string_linked_list.h"

#include "uuid_defs.h"


#ifdef __cplusplus
extern "C"
{
#endif


/**
 * Get the string representation of a uuid_t.
 *
 * @param id The uuid_t to get the string representation of.
 * @return A newly-allocated string of the id or <code>NULL</code> upon error.
 * @see FreeUUIDString.
 * @ingroup utility_group
 */
GRASSROOTS_UUID_API char *GetUUIDAsString (const uuid_t id);


/**
 * Convert a uuid_t into a given string buffer.
 *
 * @param id The uuid_t to get the string representation of.
 * @param uuid_s The buffer to write the representation to. This must be at least
 * UUID_STRING_BUFFER_SIZE bytes long.
 * @see UUID_STRING_BUFFER_SIZE
 * @ingroup utility_group
 */
GRASSROOTS_UUID_API void ConvertUUIDToString (const uuid_t id, char *uuid_s);



/**
 * Convert the string representation of a uuid_t to a uuid_t.
 *
 * @param id_s The buffer containing the representation.
 * @param id The uuid_t to set.
 * @return <code>true</code> if the uuid_t was set successfully, <code>false</code> otherwise.
 * @ingroup utility_group
 */
GRASSROOTS_UUID_API bool ConvertStringToUUID (const char *id_s, uuid_t id);


/**
 * Free a string representation of a uuid_t.
 *
 * @param uuid_s The string representation to free.
 * @see GetUUIDAsString.
 * @ingroup utility_group
 */
GRASSROOTS_UUID_API void FreeUUIDString (char *uuid_s);



/**
 * Test whether a uuid_t contains a valid value.
 *
 * @param id The uuid_t to test.
 * @return <code>true</code> if the uuid_t is valid, <code>false</code> otherwise.
 * @ingroup utility_group
 */
GRASSROOTS_UUID_API bool IsUUIDSet (uuid_t id);


/**
 * Create a LinkedList of StringListNodes with each entry being
 * a uuid.
 *
 * @param ids_s The string to parse from which the LinkedList will be
 * generated. The values in this string need to be separated by
 * whitespace.
 * @return The newly-allocated LinkedList of StringListNodes where each
 * entry is a uuid or <code>NULL</code> upon error.
 * @memberof StringListNode
 */
GRASSROOTS_UUID_API LinkedList *GetUUIDSList (const char *ids_s);


#ifdef __cplusplus
}
#endif


#endif /* CORE_SHARED_UUID_INCLUDE_UUID_UTIL_H_ */
