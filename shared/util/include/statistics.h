/*
 * statistics.h
 *
 *  Created on: 13 Mar 2022
 *      Author: billy
 */

#ifndef CORE_SHARED_UTIL_INCLUDE_STATISTICS_H_
#define CORE_SHARED_UTIL_INCLUDE_STATISTICS_H_

#include <stddef.h>

#include "jansson.h"


#include "typedefs.h"
#include "grassroots_util_library.h"



typedef struct Statistics
{
	size_t st_population_size;

	double64 st_min;

	double64 st_max;

	double64 st_mean;

	double64 st_variance;

	double64 st_std_dev;

	double64 st_sum;

} Statistics;



typedef struct StatisticsTool
{
	Statistics st_stats;

	size_t st_num_values;

	double64 *st_values_p;

	size_t st_current_index;

} StatisticsTool;




/** @publicsection */

#ifdef __cplusplus
extern "C"
{
#endif


GRASSROOTS_UTIL_API StatisticsTool *AllocateStatisticsTool (size_t num_entries);


GRASSROOTS_UTIL_API void FreeStatisticsTool (StatisticsTool *stats_tool_p);


GRASSROOTS_UTIL_API bool AddStatisticsValue (StatisticsTool *stats_tool_p, const double64 value);


GRASSROOTS_UTIL_API void ResetStatisticsTool (StatisticsTool *stats_tool_p);


GRASSROOTS_UTIL_API void CalculateStatistics (StatisticsTool *stats_tool_p);


GRASSROOTS_UTIL_API void ClearStatistics (Statistics *stats_p);


GRASSROOTS_UTIL_API void FreeStatistics (Statistics *stats_p);


GRASSROOTS_UTIL_API Statistics *CopyStatistics (const Statistics *src_p);


GRASSROOTS_UTIL_API json_t *GetStatisticsAsJSON (const Statistics *stats_p);


#ifdef __cplusplus
}
#endif




#endif /* CORE_SHARED_UTIL_INCLUDE_STATISTICS_H_ */
