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
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


#include "time_util.h"
#include "memory_allocations.h"


/* length of YYYY-MM-DD */
const size_t S_DATE_BUFFER_SIZE = 11;


/* length of YYYY-MM-DDThh:mm:ss */
const size_t S_TIME_BUFFER_SIZE = 20;


static bool ConvertNumber (const char * const buffer_s, size_t from, size_t to, int *result_p);

static bool ConvertStringToTime (const char * const time_s, time_t *time_p, bool (*conv_fn) (const char * const time_s, struct tm *time_p, int *offset_p));

static bool IsLeapYear (const int year);



static bool ParseStringToTime (struct tm * const time_p, const char *time_s,
																 const uint32 year_start, const uint32 year_end,
																 const uint32 month_start, const uint32 month_end,
																 const uint32 day_start, const uint32 day_end);

/***************************************/


void AddIntervalToTime (struct tm *time_p, const int days)
{
	int hour = time_p -> tm_hour;
	int min = time_p -> tm_min;
	int sec = time_p -> tm_sec;

	/* Take care of DST */
	time_p -> tm_isdst = 0;
	time_p -> tm_hour = 12;
	time_p -> tm_min = 0;
	time_p -> tm_sec = 0;

	/* Normalise the date */
	mktime (time_p);

	/* Add the interval */
	time_p -> tm_yday += days;

	/* Normalise the date */
	mktime (time_p);

	time_p -> tm_hour = hour;
	time_p -> tm_min = min;
	time_p -> tm_sec = sec;
}


bool SetTimeFromString (struct tm * const time_p, const char *time_s)
{
	/* YYYY-MM-DD */
	return ParseStringToTime (time_p, time_s,
																	 0, 3,
																	 5, 6,
																	 8, 9);
}


bool SetTimeFromDDMMYYYYString (struct tm * const time_p, const char *time_s)
{
	/* DD-MM-YYYY */
	return ParseStringToTime (time_p, time_s,
																	 6, 9,
																	 3, 4,
																	 0, 1);
}





char *GetTimeAsString (const struct tm * const time_p, const bool include_time_flag, const char *time_delimiter_p)
{
	char *buffer_s = NULL;
	int res = -1;
	size_t buffer_size;

	if (include_time_flag)
		{
			buffer_size = S_TIME_BUFFER_SIZE;
		}
	else
		{
			buffer_size = S_DATE_BUFFER_SIZE;
		}

	buffer_s = (char *) AllocMemory (buffer_size * sizeof (char));

	if (buffer_s)
		{
			res = sprintf (buffer_s, "%4d-%02d-%02d", 1900 + (time_p -> tm_year), 1 + (time_p -> tm_mon), time_p -> tm_mday);

			if (res > 0)
				{
					if (include_time_flag)
						{
							char time_delimiter = ':';

							if (time_delimiter_p)
								{
									time_delimiter = *time_delimiter_p;
								}

							res = sprintf (buffer_s + S_DATE_BUFFER_SIZE - 1, "T%02d%c%02d%c%02d", time_p -> tm_hour, time_delimiter, time_p -> tm_min, time_delimiter, time_p -> tm_sec);
						}

					if (res > 0)
						{
							* (buffer_s + (buffer_size - 1)) = '\0';
							return buffer_s;
						}
				}

			FreeMemory (buffer_s);
		}		/* if (buffer_s) */

	return NULL;
}


void FreeTimeString (char *time_s)
{
	FreeMemory (time_s);
}

/*
 * Tue, 17 Jun 2014 14:26:52 +0000
 * "%a, %d %b %Y %H:%M:%S %z"
 */
bool ConvertDropboxStringToTime (const char * const time_s, struct tm *time_p, int *offset_p)
{
	bool success_flag = false;
	
	if (time_s)
		{
			char month [4];

			int res = sscanf (time_s, "%*s %d %3s %d %2d%*c%2d%*c%2d %d", 
				& (time_p -> tm_mday),
				month,
				& (time_p -> tm_year),				
				& (time_p -> tm_hour),				
				& (time_p -> tm_min),				
				& (time_p -> tm_sec),				
				offset_p);
			
			/* Did we match all seven arguments? */
			if (res == 7)
				{
					const char *months_ss [12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
					int j;
					
					for (j = 11; j >= 0; -- j)
						{
							if (strcmp (month, months_ss [j]) == 0)	
								{
									time_p -> tm_mon = j;
									
									success_flag = true;
									j = -1;
								}
						}
										
				}		/* if (res == 7) */			
			
		}		/* if (time_s) */

	return success_flag;
}


bool ConvertCompactStringToTime (const char * const time_s, struct tm *time_p, int * UNUSED_PARAM (offset_p))
{
	bool success_flag = false;
	
	if (time_s)
		{
			const size_t l = strlen (time_s);
			
			if (l >= 8)
				{
					int year;
				
					if (ConvertNumber (time_s, 0, 3, &year))
						{
							int month;

							if (ConvertNumber (time_s, 4, 5, &month))
								{
									int day;

									if (ConvertNumber (time_s, 6, 7, &day))
										{
											int hour = 0;
											int min = 0;
											int sec = 0;
											
											time_p -> tm_year = year - 1900;
											time_p -> tm_mon = month - 1;
											time_p -> tm_mday = day; 

											success_flag = true;
											
											if (l == 14)
												{
													if (ConvertNumber (time_s, 8, 9, &hour))
														{
															if (ConvertNumber (time_s, 10, 11, &min))
																{
																	if (ConvertNumber (time_s, 12, 13, &sec))
																		{

																		}		/* if (ConvertNumber (time_s, 12, 13, &sec)) */
		
																}		/* if (ConvertNumber (time_s, 10, 11, &min)) */
															
														}		/* if (ConvertNumber (time_s, 8, 9, &hour)) */
													
												}		/* if (l == 14) */

											time_p -> tm_hour = hour;
											time_p -> tm_min = min;
											time_p -> tm_sec = sec;

										}		/* if (ConvertNumber (time_s, 0, 3, &year)) */
									
								}		/* if (ConvertNumber (time_s, 0, 3, &year)) */
	
						}		/* if (ConvertNumber (time_s, 0, 3, &year)) */
				
				}		/* if (l >= 8) */

		}		/* if (time_s) */

	return success_flag;
}



bool ConvertCompactStringToEpochTime (const char * const time_s, time_t *time_p)
{
	return ConvertStringToTime (time_s, time_p, ConvertCompactStringToTime);
}


bool ConvertDropboxStringToEpochTime (const char * const time_s, time_t *time_p)
{
	return ConvertStringToTime (time_s, time_p, ConvertDropboxStringToTime);
}


bool GetPresentTime (struct tm *tm_p)
{
	bool success_flag = false;
	time_t current_time = time (NULL);

	if (current_time != (time_t) -1)
		{
#ifdef WINDOWS
		success_flag = (localtime_s (tm_p, &current_time) != 0);
#else
		success_flag = (localtime_r (&current_time, tm_p) != NULL);
#endif
		}

	return success_flag;
}



void SetDateValuesForTime (struct tm *time_p, const int year, const int month, const int day)
{
	time_p -> tm_year = year - 1900;
	time_p -> tm_mon = month + 1;
	time_p -> tm_mday = day;
}


struct tm *AllocateTime (void)
{
	struct tm *time_p = AllocMemory (sizeof (struct tm));

	if (time_p)
		{
			ClearTime (time_p);
		}

	return time_p;
}


void ClearTime (struct tm *time_p)
{
	memset (time_p, 0, sizeof (struct tm));
}


void FreeTime (struct tm *time_p)
{
	FreeMemory (time_p);
}


struct tm **AllocateTimeArray (const size_t num_entries)
{
	struct tm **times_pp = (struct tm **) AllocMemoryArray (num_entries + 1, sizeof (struct tm *));

	if (times_pp)
		{
			return times_pp;
		}

	return NULL;
}


void FreeTimeArray (struct tm **values_pp, const size_t num_values)
{
	size_t i;
	struct tm **value_pp = values_pp;

	for (i = num_values; i > 0; -- i, ++ value_pp)
		{
			if (*value_pp)
				{
					FreeTime (*value_pp);
				}
		}

	FreeMemory (values_pp);
}


struct tm *DuplicateTime (const struct tm *src_p)
{
	struct tm *dest_p = AllocateTime ();

	if (dest_p)
		{
			CopyTime (src_p, dest_p);
		}

	return dest_p;
}


void CopyTime (const struct tm *src_p, struct tm *dest_p)
{
	memcpy (dest_p, src_p, sizeof (struct tm));
}


bool CopyTimesArray (const struct tm **src_array_pp, struct tm **dest_array_pp, const size_t num_entries)
{
	const struct tm **src_pp = src_array_pp;
	struct tm **dest_pp = dest_array_pp;
	size_t i = 0;
	bool success_flag = true;

	while ((i < num_entries) && success_flag)
		{
			if (*src_pp)
				{
					if (*dest_pp)
						{
							CopyTime (*src_pp, *dest_pp);
						}
					else
						{
							struct tm *dest_p = DuplicateTime (*src_pp);

							if (dest_p)
								{
									*dest_pp = dest_p;
								}
							else
								{
									success_flag = false;
								}
						}
				}
			else
				{
					*dest_pp = NULL;
				}

			++ src_pp;
			++ dest_pp;
		}

	if (!success_flag)
		{
			while (dest_pp >= dest_array_pp)
				{
					if (*dest_pp)
						{
							FreeTime (*dest_pp);
							*dest_pp = NULL;
						}

					-- dest_pp;
				}
		}

	return success_flag;
}


struct tm *GetTimeFromString (const char *time_s)
{
	struct tm *time_p = AllocateTime ();

	if (time_p)
		{
			if (SetTimeFromString (time_p, time_s))
				{
					return time_p;
				}		/* if (SetTimeFromString (time_p, time_s)) */

			FreeTime (time_p);
		}		/* if (time_p) */

	return NULL;
}


int CompareDates (const struct tm *time_0_p, const struct tm *time_1_p, const bool dates_only_flag)
{
	int res = (time_0_p -> tm_year) - (time_1_p -> tm_year);

	if (res == 0)
		{
			res = (time_0_p -> tm_mon) - (time_1_p -> tm_mon);

			if (res == 0)
				{
					res = (time_0_p -> tm_mday) - (time_1_p -> tm_mday);

					if (res == 0)
						{
							if (!dates_only_flag)
								{
									res = (time_0_p -> tm_hour) - (time_1_p -> tm_hour);

									if (res == 0)
										{
											res = (time_0_p -> tm_min) - (time_1_p -> tm_min);

											if (res == 0)
												{
													res = (time_0_p -> tm_sec) - (time_1_p -> tm_sec);
												}
										}
								}
						}
				}
		}

	return res;
}


bool MayStringIncludeTime (const char * const time_s)
{
	bool time_flag = false;

	if (time_s)
		{
			const size_t l = strlen (time_s);

			if (l == S_TIME_BUFFER_SIZE - 1)
				{
					time_flag = true;
				}
		}

	return time_flag;
}


/****************************************/

static bool ConvertNumber (const char * const buffer_s, size_t from, size_t to, int *result_p)
{
	bool success_flag = true;
	const char *digit_p = buffer_s + from;
	size_t i;
	int res = 0;
	
	/* check that it's a valid positive integer */
	for (i = from; i <= to; ++ i, ++ digit_p)
		{
			if (isdigit (*digit_p))
				{
					int j = *digit_p - '0';

					res *= 10;
					res += j;
				}
			else
				{
					success_flag = false;
					i = to + 1;		/* force exit from loop */
				}
		}
	
	if (success_flag)
		{
			*result_p = res;
		}
	
	return success_flag;
}



static bool ConvertStringToTime (const char * const time_s, time_t *time_p, bool (*conv_fn) (const char * const time_s, struct tm *time_p, int *offset_p))
{
	struct tm t;
	bool success_flag = false;
	int offset = 0;
	
	memset (&t, 0, sizeof (struct tm));

	if (conv_fn (time_s, &t, &offset))
		{			
			*time_p = mktime (&t);
		
			if (offset != 0)
				{
					double d = floor (offset * 0.010);
					int h = (int) d; 
					int m = offset - (h * 100);
					time_t offset_time = (3600 * (time_t) h) + (60 * (time_t) m);
					
					if (offset > 0)
						{
							time_p += offset_time; 
						}
					else
						{
							time_p -= offset_time; 							
						}
				}	
		}
		
	return success_flag;
}


static bool IsLeapYear (const int year)
{
	bool result = false;

	/* is it divisible by 4? */
	if ((year >> 2) == 0)
		{
			/* if it is divisible by 100 it is not a leap year... */
			if (year % 100 == 0)
				{
					/* ... unless it is divisible by by 400 */
					if (year % 400 == 0)
						{
							result = true;
						}

				}		/* if (year % 100 == 0) */
			else
				{
					result = true;
				}

		}		/* if ((year >> 2) == 0) */

	return result;
}


static bool ParseStringToTime (struct tm * const time_p, const char *time_s,
																 const uint32 year_start, const uint32 year_end,
																 const uint32 month_start, const uint32 month_end,
																 const uint32 day_start, const uint32 day_end)
{
	bool success_flag = false;
	const size_t l = strlen (time_s);

	/*
	 * String is of the format DD-MM-YYYY or DD-MM-YYYYThh:mm:ss
	 */

	if (l >= S_DATE_BUFFER_SIZE - 1)
		{
			int year;

			if (ConvertNumber (time_s, year_start, year_end, &year))
				{
					int month;

					if (ConvertNumber (time_s, month_start, month_end, &month))
						{
							int day;

							if (ConvertNumber (time_s, day_start, day_end, &day))
								{
									int hour = 0;
									int min = 0;
									int sec = 0;

									if (l == S_TIME_BUFFER_SIZE - 1)
										{
											if (ConvertNumber (time_s, 11, 12, &hour))
												{
													if (ConvertNumber (time_s, 14, 15, &min))
														{
															if (ConvertNumber (time_s, 17, 18, &sec))
																{
																	success_flag = true;
																}
														}
												}
										}
									else
										{
											success_flag = true;
										}

									if (success_flag)
										{
											time_p -> tm_year = year - 1900;
											time_p -> tm_mon = month - 1;
											time_p -> tm_mday = day;

											time_p -> tm_hour = hour;
											time_p -> tm_min = min;
											time_p -> tm_sec = sec;
										}

								}		/* if (ConvertNumber (time_s, 6, 7, &day)) */

						}		/* if (ConvertNumber (time_s, 4, 5, &month)) */

				}		/* if (ConvertNumber (time_s, 0, 3, &year)) */
		}

	return success_flag;
}


bool IsValidDate (struct tm *time_p)
{
	bool valid_flag = false;

	if (time_p)
		{
			struct tm t;

			ClearTime (&t);

			if (memcmp (time_p, &t, sizeof (struct tm)) != 0)
				{
					valid_flag = true;
				}
		}

	return valid_flag;
}

