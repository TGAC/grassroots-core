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

/**
 * shared_type.h
 *
 *  Created on: 14 Feb 2017
 *      Author: billy
 *
 * @file
 * @brief
 */

#ifndef SHARED_SRC_PARAMETERS_INCLUDE_SHARED_TYPE_H_
#define SHARED_SRC_PARAMETERS_INCLUDE_SHARED_TYPE_H_

#include "grassroots_params_library.h"
#include "typedefs.h"
#include "data_resource.h"

#include "linked_list.h"

#include "parameter_type.h"

#include "time.h"


/**
 * A datatype to store values for a Parameter.
 * Use the ParameterType to access the correct
 * value.
 *
 * @ingroup parameters_group
 */
typedef struct SharedType
{
	ParameterType st_active_type;

	union
		{
			/** A Boolean value */
			bool *st_boolean_value_p;

			/** A signed integer */
			int32 *st_long_value_p;

			/** An unsigned integer */
			uint32 *st_ulong_value_p;

			/** A real number */
			double64 *st_data_value_p;

			/** A c-style string */
			char *st_string_value_s;

			/** A single character */
			char *st_char_value_p;

			/** A Resource */
			Resource *st_resource_value_p;

			/** A JSON fragment */
			json_t *st_json_p;


			/** A time and date */
			struct tm *st_time_p;
		} st_value;


} SharedType;


/**
 * A datatype allowing a SharedType to be stored
 * on a LinkedList.
 *
 * @extends ListItem
 * @ingroup parameters_group
 */
typedef struct SharedTypeNode
{
	/** The basic ListItem */
	ListItem stn_node;

	/** The SharedType value */
	SharedType stn_value;
} SharedTypeNode;



/* forward declarations */
struct ParameterBounds;


#ifdef __cplusplus
extern "C"
{
#endif


/**
 * Clear the value of a SharedType releasing any allocated memory
 * for the value.
 *
 * @param st_p The SharedType to clear.
 * @memberof SharedType
 */
GRASSROOTS_PARAMS_API void ClearSharedType (SharedType *st_p);



/**
 * Initialise a SharedType ready for use.
 *
 * @param st_p The SharedType to initialise.
 * @param pt The ParameterType for this SharedType
 * @memberof SharedType
 */
GRASSROOTS_PARAMS_API void InitSharedType (SharedType *st_p, const ParameterType pt);



/**
 * Make a deep copy of the value of one SharedType to another.
 *
 * @param src The SharedType to copy the value from.
 * @param dest_p The SharedType to copy the value to.
 * @return <code>true</code> if the value was copied successfully, <code>false</code> otherwise.
 * @memberof SharedType
 */
GRASSROOTS_PARAMS_API bool CopySharedType (const SharedType src, SharedType *dest_p);



/**
 * Set the value stored in a SharedType object.
 *
 * @param st_p The SharedType to set the value for.
 * @param value_p The value to set.
 * @return <code>true</code> if the value was set successfully, <code>false</code> otherwise.
 * @memberof SharedType
 */
GRASSROOTS_PARAMS_API bool SetSharedTypeValue (SharedType *st_p, void *value_p, const struct ParameterBounds *bounds_p);


GRASSROOTS_PARAMS_API bool SetSharedTypeBooleanValue (SharedType * value_p, const bool b);


GRASSROOTS_PARAMS_API bool SetSharedTypeCharValue (SharedType * value_p, const char c);


GRASSROOTS_PARAMS_API bool SetSharedTypeUnsignedIntValue (SharedType * value_p, const uint32 i, const struct ParameterBounds * const bounds_p);


GRASSROOTS_PARAMS_API bool SetSharedTypeSignedIntValue (SharedType * value_p, const int32 i, const struct ParameterBounds * const bounds_p);


GRASSROOTS_PARAMS_API bool SetSharedTypeRealValue (SharedType * value_p, const double64 d, const struct ParameterBounds * const bounds_p);


GRASSROOTS_PARAMS_API bool SetSharedTypeStringValue (SharedType *value_p, const char * const src_s);


GRASSROOTS_PARAMS_API bool SetSharedTypeResourceValue (SharedType *value_p, const Resource * const src_p);


GRASSROOTS_PARAMS_API bool SetSharedTypeJSONValue (SharedType *value_p, const json_t * const src_p);


GRASSROOTS_PARAMS_API bool SetSharedTypeTimeValue (SharedType *value_p, const struct tm * const src_p);


GRASSROOTS_PARAMS_API bool GetSharedTypeBooleanValue (const SharedType *value_p, bool * const b_p);


GRASSROOTS_PARAMS_API bool GetSharedTypeCharValue (const SharedType *value_p, char * const c_p);


GRASSROOTS_PARAMS_API bool GetSharedTypeUnsignedIntValue (const SharedType * value_p, uint32 * const i_p);


GRASSROOTS_PARAMS_API bool GetSharedTypeSignedIntValue (const SharedType * value_p, int32 * const i_p);


GRASSROOTS_PARAMS_API bool GetSharedTypeRealValue (const SharedType * value_p, double64 * const d_p);


GRASSROOTS_PARAMS_API bool GetSharedTypeStringValue (const SharedType *value_p, char * const src_s);


GRASSROOTS_PARAMS_API bool GetSharedTypeResourceValue (const SharedType *value_p, Resource * const res_p);


GRASSROOTS_PARAMS_API bool GetSharedTypeJSONValue (const SharedType *value_p, json_t * const json_p);


GRASSROOTS_PARAMS_API bool GetSharedTypeTimeValue (const SharedType *value_p, struct tm * const time_p);


#ifdef __cplusplus
}
#endif


#endif /* SHARED_SRC_PARAMETERS_INCLUDE_SHARED_TYPE_H_ */
