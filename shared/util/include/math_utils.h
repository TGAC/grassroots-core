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
 * @file
 * @brief
 */
/**@file math_utils.h
*/ 

#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include "grassroots_util_library.h"
#include "typedefs.h"

#ifdef __cplusplus
	extern "C" {
#endif


/**
 * Compare two doubles taking the epsilon
 * of this computer into account.
 *
 * @param d1 The first double to compare.
 * @param d2 The second double to compare.
 * @return -1 if d1 is less than d2, +1 if d1 is
 * greater than d2 and 0 if they are identical.
 *
 * @ingroup utility_group
 */
GRASSROOTS_UTIL_API int CompareDoubles (const double d1, const double d2);


/**
 * Compare two floats taking the epsilon
 * of this computer into account.
 *
 * @param f1 The first float to compare.
 * @param f2 The second float to compare.
 * @return -1 if f1 is less than f2, +1 if f1 is
 * greater than f2 and 0 if they are identical.
 *
 * @ingroup utility_group
 */
GRASSROOTS_UTIL_API int CompareFloats (const float f1, const float f2);


/**
 * Since atof, atoi, etc. return 0 on error we need to do our own
 * "number getting" routine that notifies us of any errors.
 *
 * @param str_pp Pointer to char * where we start getting the number from.
 * @param data_p Pointer where the number will be stored if the conversion was successful.
 * @param alternative_decimal_points_s An array of characters to consider as decimal points
 * which may be the case in different locales. If this is <code>NULL</code> then a "." will be used.
 * @return <code>true</code> if the conversion was successful, <code>false</code> otherwise.
 *
 * @ingroup utility_group
 */
GRASSROOTS_UTIL_API bool GetValidRealNumber (const char **str_pp, double64 *data_p, const char * const alternative_decimal_points_s);


/**
 * Since atof, atoi, etc. return 0 on error we need to do our own
 * "number getting" routine that notifies us of any errors.
 *
 * @param str_pp Pointer to char * where we start getting the number from.
 * @param answer_p Pointer where the number will be stored if the conversion was successful.
 * @return <code>true</code> if the conversion was successful, <code>false</code> otherwise.
 *
 * @ingroup utility_group
 */
GRASSROOTS_UTIL_API bool GetValidInteger (const char **str_pp, int *answer_p);


/**
 * Since atof, atoi, etc. return 0 on error we need to do our own
 * "number getting" routine that notifies us of any errors.
 *
 * @param str_pp Pointer to char * where we start getting the number from.
 * @param answer_p Pointer where the number will be stored if the conversion was successful.
 * @return TRUE if the conversion was successful, FALSE otherwise.
 *
 * @ingroup utility_group
 */
GRASSROOTS_UTIL_API bool GetValidLong (const char **str_pp, long *answer_p);


/**
 * Compare two doubles taking the epsilon
 * of this computer into account.
 *
 * @param v1_p Pointer to the first double to compare.
 * @param v2_p Pointer to the second double to compare.
 * @return -1 if the first double is less than the second,
 * +1 if the first double is greater than the second
 * and 0 if they are identical.
 *
 * @ingroup utility_group
 */
GRASSROOTS_UTIL_API int SortDoubles (const void *v1_p, const void *v2_p);


/**
 * Get an integer as a string, e.g.
 *
 *  1 would be converted to "1"
 *
 * @param value The value to convert to a string.
 * @return The c-style string or <code>NULL</code> upon error.
 * This needs to be freed using FreeCopiedString().
 * @ingroup utility_group
 */
GRASSROOTS_UTIL_API char *ConvertIntegerToString (const int32 value);


/**
 * Get an unsigned integer as a string, e.g.
 *
 *  1 would be converted to "1"
 *
 * @param value The value to convert to a string.
 * @return The c-style string or <code>NULL</code> upon error.
 * This needs to be freed using FreeCopiedString().
 * @ingroup utility_group
 */
GRASSROOTS_UTIL_API char *ConvertUnsignedIntegerToString (const uint32 value);



/**
 * Get an int64 as a string, e.g.
 *
 *  1 would be converted to "1"
 *
 * @param value The value to convert to a string.
 * @return The c-style string or <code>NULL</code> upon error.
 * This needs to be freed using FreeCopiedString().
 * @ingroup utility_group
 */
GRASSROOTS_UTIL_API char *ConvertLongToString (const int64 value);


/**
 * Get an double as a string, e.g.
 *
 *  1.03 would be converted to "1.03"
 *
 * @param value The value to convert to a string.
 * @return The c-style string or <code>NULL</code> upon error.
 * This needs to be freed using FreeCopiedString().
 * @ingroup utility_group
 */
GRASSROOTS_UTIL_API char *ConvertDoubleToString (const double64 value);



/**
 * Get an size_t as a string, e.g.
 *
 *  1 would be converted to "1"
 *
 * @param value The value to convert to a string.
 * @return The c-style string or <code>NULL</code> upon error.
 * This needs to be freed using FreeCopiedString().
 * @ingroup utility_group
 */
GRASSROOTS_UTIL_API char *ConvertSizeTToString (const size_t value);



#ifdef __cplusplus
}
#endif


#endif	/* #ifndef MATH_UTILS_H */
