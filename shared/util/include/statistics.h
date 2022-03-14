/*
 * statistics.h
 *
 *  Created on: 13 Mar 2022
 *      Author: billy
 */

#ifndef CORE_SHARED_UTIL_INCLUDE_STATISTICS_H_
#define CORE_SHARED_UTIL_INCLUDE_STATISTICS_H_

#include "linked_list.h"

typedef struct Statistics
{
	size_t st_num_values;

	double64 st_min;

	double54 st_max;

	double64 st_mean;

	double64 st_standard_deviation;

} Statistics;

typedef struct
{
	MeasuredVariable *mvs_variable_p;

	LinkedList mvs_values;

} MeasuredVariableStats;










#endif /* CORE_SHARED_UTIL_INCLUDE_STATISTICS_H_ */
