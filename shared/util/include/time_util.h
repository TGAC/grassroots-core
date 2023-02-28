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
#ifndef TIME_UTIL_H
#define TIME_UTIL_H

#include <time.h>

#include "typedefs.h"
#include "grassroots_util_library.h"


#ifdef __cplusplus
extern "C" 
{
#endif


/**
 * Convert a string that is of the form YYYYMMDD or YYYYMMDDhhmmss 
 * into a struct tm.
 * 
 * @param time_s The string to convert.
 * @param time_p The struct tm to store the converted time in.
 * @param offset_p Where the timezone adjustment will be stored. It's in the form (+/-)hhmm.
 * @return <code>true</code> if the value was converted successfully, <code>false</code> otherwise.
 * @ingroup utility_group
 */
GRASSROOTS_UTIL_API bool ConvertCompactStringToTime (const char * const time_s, struct tm *time_p, int *offset_p);


/**
 * Convert a string that is of the form YYYYMMDD or YYYYMMDDhhmmss 
 * into a time_t.
 * 
 * @param time_s The string to convert.
 * @param time_p The time_t to store the converted time in.
 * @return <code>true</code> if the value was converted successfully, <code>false</code> otherwise.
 * @ingroup utility_group
 */
GRASSROOTS_UTIL_API bool ConvertCompactStringToEpochTime (const char * const time_s, time_t *time_p);



/**
 * Convert a string that is of the form "Sat, 21 Aug 2010 22:31:20 +0000" ("%a, %d %b %Y %H:%M:%S %z")
 * into a struct tm.
 * 
 * @param time_s The string to convert.
 * @param time_p The struct tm to store the converted time in.
 * @param offset_p Where the timezone adjustment will be stored. It's in the form (+/-)hhmm.
 * @return <code>true</code> if the value was converted successfully, <code>false</code> otherwise.
 * @ingroup utility_group
 */
GRASSROOTS_UTIL_API bool ConvertDropboxStringToTime (const char * const time_s, struct tm *time_p, int *offset_p);


/**
 * Convert a string that is of the form "Sat, 21 Aug 2010 22:31:20 +0000" ("%a, %d %b %Y %H:%M:%S %z")
 * into a time_t.
 * 
 * @param time_s The string to convert.
 * @param time_p The time_t to store the converted time in.
 * @return <code>true</code> if the value was converted successfully, <code>false</code> otherwise.
 * @ingroup utility_group
 */
GRASSROOTS_UTIL_API bool ConvertDropboxStringToEpochTime (const char * const time_s, time_t *time_p);



/**
 * Get the current time
 *
 * @param tm_p Where the current time will be stored.
 * @return <code>true</code> if the time was set successfully, <code>false</code> otherwise.
 * @ingroup utility_group
 */
GRASSROOTS_UTIL_API bool GetPresentTime (struct tm *tm_p);



/**
 * Get a time as a string in the ISO 8601 format
 *
 * @param time_p The time to get as a c-style string
 * @param include_time_flag If this is <code>true</code> then both the date and time
 * will be included in the string in ISO 8601 format. If this is <code>false</code>,
 * then just the date part of the string will be returned.
 * @param time_delimiter_p By default, if the time is requested as part of the string,
 * the hours, minutes and seconds are separated by a ':'. If this value is not <code>NULL</code>
 * then the character it points to will be used instead.
 * @return The time as a c-style string or <code>NULL</code> upon error.
 * @ingroup utility_group
 * @see FreeTimeString()
 */
GRASSROOTS_UTIL_API char *GetTimeAsString (const struct tm * const time_p, const bool include_time_flag, const char *time_delimiter_p);

/**
 * @fn void FreeTimeString(char*)
 * @brief Free a string previsouly-generated from GetTimeAsString().
 *
 * @param time_s The string to free.
 * @ingroup utility_group
 * @see GetTimeAsString()
 */
GRASSROOTS_UTIL_API void FreeTimeString (char *time_s);


/**
 * Set the time from a string in the ISO 8601 format
 *
 * @param time_p The time to be set from the string
 * @param time_s The time to be set as a c-style string or <code>NULL</code> upon error.
 * @return <code>true</code> if the time was set successfully, <code>false</code> otherwise.
 * @ingroup utility_group
 */
GRASSROOTS_UTIL_API bool SetTimeFromString (struct tm * const time_p, const char *time_s);



/**
 * Set the time from a string of the form DD-MM-YYYY
 *
 * @param time_p The time to be set from the string
 * @param time_s The time to be set as a c-style string or <code>NULL</code> upon error.
 * @return <code>true</code> if the time was set successfully, <code>false</code> otherwise.
 * @ingroup utility_group
 */
GRASSROOTS_UTIL_API bool SetTimeFromDDMMYYYYString (struct tm * const time_p, const char *time_s);



GRASSROOTS_UTIL_API struct tm *GetTimeFromString (const char *time_s);


/**
 * Add a number of days to a time.
 *
 * @param time_p The time to be adjusted.
 * @param days The number of days to add.
 * @ingroup utility_group
 */
GRASSROOTS_UTIL_API void AddIntervalToTime (struct tm *time_p, const int days);


/**
 * Allocate a struct tm variable and set all of its values to 0.
 *
 * This value should be freed using FreeTime() to avoid a memory leak.
 *
 * @return The new struct tm or <code>NULL</code> upon error.
 * @ingroup utility_group
 */
GRASSROOTS_UTIL_API struct tm *AllocateTime (void);


/**
 * Free a struct tm variable allocated by AllocateTime().
 *
 * @param time_p The struct tm to free.
 * @ingroup utility_group
 */
GRASSROOTS_UTIL_API void FreeTime (struct tm *time_p);


/**
 * Copy the values from one struct tm to another.
 *
 * @param src_p The struct tm to copy the values from.
 * @param dest_p The struct tm to copy the values to.
 * @ingroup utility_group
 */
GRASSROOTS_UTIL_API void CopyTime (const struct tm *src_p, struct tm *dest_p);

/**
 * Make a deep copy of a struct tm.
 *
 * @param src_p The struct tm to copy.
 * @return The newly-allocated struct tm with the copied values or <code>NULL</code>
 * upon error. This will need to be freed with FreeTime().
 * @ingroup utility_group
 */
GRASSROOTS_UTIL_API struct tm *DuplicateTime (const struct tm *src_p);


/**
 * Set the year, month and day values for a struct tm.
 *
 * @param year The 4 digit year value.
 * @param month The month value ranging from 0 for January to 11 for December.
 * @param month The day value starting from 1.
 * @ingroup utility_group
 */
GRASSROOTS_UTIL_API void SetDateValuesForTime (struct tm *time_p, const int year, const int month, const int day);


GRASSROOTS_UTIL_API void ClearTime (struct tm *time_p);


GRASSROOTS_UTIL_API bool IsValidDate (struct tm *time_p);


/**
 * Compare two dates chronologically.
 *
 * @param time_0_p The first time.
 * @param time_1_p The second time.
 * @param dates_only_flag If this is <code>true</code> then only the date, month and year of the two times will be
 * used for comparison. If this is <code>false</code> then the hours, minutes and seconds will be used too.
 * @return Less than zero if the first date is chronologically before the second one, greater than zero if the first date
 * is after the second and zero if the dates are the same.
 * @ingroup utility_group
 */
GRASSROOTS_UTIL_API int CompareDates (const struct tm *time_0_p, const struct tm *time_1_p, const bool dates_only_flag);



GRASSROOTS_UTIL_API bool MayStringIncludeTime (const char * const time_s);


GRASSROOTS_UTIL_API void FreeTimeArray (struct tm **values_pp, const size_t num_values);

#ifdef __cplusplus
}
#endif


#endif		/* #ifndef TIME_UTIL_H */
 
