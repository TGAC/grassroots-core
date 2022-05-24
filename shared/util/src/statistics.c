/*
 * statistics.c
 *
 *  Created on: 24 May 2022
 *      Author: billy
 */

#include <math.h>
#include <string.h>

#include "statistics.h"
#include "memory_allocations.h"
#include "math_utils.h"




Statistics *AllocateStatistics (size_t num_entries)
{
	double64 *values_p = (double64 *) AllocMemory (num_entries * sizeof (double64));

	if (values_p)
		{
			Statistics *stats_p = (Statistics *) AllocMemory (sizeof (Statistics));

			if (stats_p)
				{
					stats_p -> st_num_values = num_entries;
					stats_p -> st_values_p = values_p;

					ResetStatistics (stats_p);

					return stats_p;
				}

			FreeMemory (values_p);
		}

	return NULL;
}


void FreeStatistics (Statistics *stats_p)
{
	FreeMemory (stats_p -> st_values_p);
	FreeMemory (stats_p);
}



void ResetStatistics (Statistics *stats_p)
{
	memset (stats_p -> st_values_p, 0, (stats_p -> st_num_values) * sizeof (double64));

	stats_p -> st_current_index = 0;
	stats_p -> st_min = 0.0;
	stats_p -> st_max = 0.0;
	stats_p -> st_mean = 0.0;
	stats_p -> st_variance = 0.0;
	stats_p -> st_std_dev = 0.0;
	stats_p -> st_sum = 0.0;
}

bool AddStatisticsValue (Statistics *stats_p, const double64 value)
{
	bool success_flag = true;

	if (stats_p -> st_current_index < stats_p -> st_num_values)
		{
			* ((stats_p -> st_values_p) + (stats_p -> st_current_index)) = value;

			if (stats_p -> st_current_index == 0)
				{
					stats_p -> st_min = value;
					stats_p -> st_max = value;
				}
			else
				{
					if (CompareDoubles (stats_p -> st_min, value) == -1)
						{
							stats_p -> st_min = value;
						}
					else if (CompareDoubles (stats_p -> st_max, value) == 1)
						{
							stats_p -> st_max = value;
						}

				}

			stats_p -> st_sum += value;

			++ (stats_p -> st_current_index);
		}
	else
		{
			success_flag = false;
		}

	return success_flag;
}



void CalculateStatistics (Statistics *stats_p)
{
	if (stats_p -> st_current_index > 0)
		{
			double64 *value_p = stats_p -> st_values_p;
			size_t i = 0;
			double64 variance = 0.0;
			const size_t limit = 1 + (stats_p -> st_current_index);
			const double64 mean = (stats_p -> st_sum) / ((double64) limit);

			for ( ; i < limit; ++ i, ++ value_p)
				{
					double64 d = *value_p - mean;

					d *= d;
					variance += d;
				}


			stats_p -> st_variance = variance;
			stats_p -> st_std_dev = sqrt (variance);

			stats_p -> st_mean = mean;
		}
}
