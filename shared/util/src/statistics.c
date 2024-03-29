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
#include "streams.h"
#include "json_util.h"

static const char * const S_SUM_S = "sum";
static const char * const S_STD_DEV_S = CONTEXT_PREFIX_STATS_ONTOLOGY_S "0000237";
static const char * const S_VARIANCE_S = CONTEXT_PREFIX_STATS_ONTOLOGY_S "0000113";
static const char * const S_SAMPLE_MEAN_S = CONTEXT_PREFIX_STATS_ONTOLOGY_S "0000401";
static const char * const S_MIN_S = CONTEXT_PREFIX_STATS_ONTOLOGY_S "0000150";
static const char * const S_MAX_S = CONTEXT_PREFIX_STATS_ONTOLOGY_S "0000151";
static const char * const S_POP_SIZE_S = CONTEXT_PREFIX_STATS_ONTOLOGY_S "0000088";


StatisticsTool *AllocateStatisticsTool (size_t num_entries)
{
	double64 *values_p = (double64 *) AllocMemory (num_entries * sizeof (double64));

	if (values_p)
		{
			StatisticsTool *stats_tool_p = (StatisticsTool *) AllocMemory (sizeof (StatisticsTool));

			if (stats_tool_p)
				{
					stats_tool_p -> st_num_values = num_entries;
					stats_tool_p -> st_values_p = values_p;

					ResetStatisticsTool (stats_tool_p);

					return stats_tool_p;
				}

			FreeMemory (values_p);
		}

	return NULL;
}


void FreeStatisticsTool (StatisticsTool *stats_tool_p)
{
	FreeMemory (stats_tool_p -> st_values_p);
	FreeMemory (stats_tool_p);
}



void ResetStatisticsTool (StatisticsTool *stats_tool_p)
{
	memset (stats_tool_p -> st_values_p, 0, (stats_tool_p -> st_num_values) * sizeof (double64));

	stats_tool_p -> st_current_index = 0;

	ClearStatistics (& (stats_tool_p -> st_stats));
}


Statistics *AllocateStatistics (void)
{
	Statistics *stats_p = (Statistics *) AllocMemory (sizeof (Statistics));

	if (stats_p)
		{
			ClearStatistics (stats_p);

			return stats_p;
		}

	return NULL;
}

void ClearStatistics (Statistics *stats_p)
{
	stats_p -> st_population_size = 0;
	stats_p -> st_min = 0.0;
	stats_p -> st_max = 0.0;
	stats_p -> st_mean = 0.0;
	stats_p -> st_variance = 0.0;
	stats_p -> st_std_dev = 0.0;
	stats_p -> st_sum = 0.0;
}


void FreeStatistics (Statistics *stats_p)
{
	FreeMemory (stats_p);
}


bool AddStatisticsValue (StatisticsTool *stats_tool_p, const double64 value)
{
	bool success_flag = true;

	if (stats_tool_p -> st_current_index < stats_tool_p -> st_num_values)
		{
			* ((stats_tool_p -> st_values_p) + (stats_tool_p -> st_current_index)) = value;

			if (stats_tool_p -> st_current_index == 0)
				{
					stats_tool_p -> st_stats.st_min = value;
					stats_tool_p -> st_stats.st_max = value;
				}
			else
				{
					if (CompareDoubles (stats_tool_p -> st_stats.st_min, value) == 1)
						{
							stats_tool_p -> st_stats.st_min = value;
						}
					else if (CompareDoubles (stats_tool_p -> st_stats.st_max, value) == -1)
						{
							stats_tool_p -> st_stats.st_max = value;
						}

				}

			stats_tool_p -> st_stats.st_sum += value;

			++ (stats_tool_p -> st_current_index);
			++ (stats_tool_p -> st_stats.st_population_size);
		}
	else
		{
			success_flag = false;
		}

	return success_flag;
}



void CalculateStatistics (StatisticsTool *stats_tool_p)
{
	if (stats_tool_p -> st_current_index > 0)
		{
			double64 *value_p = stats_tool_p -> st_values_p;
			size_t i;
			double64 sum_of_square_diffs = 0.0;
			const size_t limit = stats_tool_p -> st_stats.st_population_size;
			const double64 mean = (stats_tool_p -> st_stats.st_sum) / ((double64) limit);

			for (i = 0; i < limit; ++ i, ++ value_p)
				{
					double64 d = *value_p - mean;

					d *= d;
					sum_of_square_diffs += d;
				}


			stats_tool_p -> st_stats.st_variance = sum_of_square_diffs / ((double) (stats_tool_p -> st_stats.st_population_size));
			stats_tool_p -> st_stats.st_std_dev = sqrt (stats_tool_p -> st_stats.st_variance);

			stats_tool_p -> st_stats.st_mean = mean;
		}
}


Statistics *CopyStatistics (const Statistics *src_p)
{
	Statistics *dest_p = (Statistics *) AllocMemory (sizeof (Statistics));

	if (dest_p)
		{
			memcpy (dest_p, src_p, sizeof (Statistics));
			return dest_p;
		}

	return NULL;
}



json_t *GetStatisticsAsJSON (const Statistics *stats_p)
{
	json_t *stats_json_p = json_object ();

	if (stats_json_p)
		{
			if (SetJSONInteger (stats_json_p, S_POP_SIZE_S, stats_p -> st_population_size))
				{
					if (SetJSONReal (stats_json_p, S_MIN_S, stats_p -> st_min))
						{
							if (SetJSONReal (stats_json_p, S_MAX_S, stats_p -> st_max))
								{
									if (SetJSONReal (stats_json_p, S_SAMPLE_MEAN_S, stats_p -> st_mean))
										{
											if (SetJSONReal (stats_json_p, S_VARIANCE_S, stats_p -> st_variance))
												{
													if (SetJSONReal (stats_json_p, S_STD_DEV_S, stats_p -> st_std_dev))
														{
															if (SetJSONReal (stats_json_p, S_SUM_S, stats_p -> st_sum))
																{
																	return stats_json_p;
																}		/* if (SetJSONReal (stats_json_p, S_SUM_S, stats_p -> st_sum)) */
															else
																{
																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, stats_json_p, "Failed to add \"%s\": \"%s\"", S_SUM_S, stats_p -> st_sum);
																}
														}		/* if (SetJSONReal (stats_json_p, S_STD_DEV_S, stats_p -> st_std_dev)) */
													else
														{
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, stats_json_p, "Failed to add \"%s\": \"%s\"", S_STD_DEV_S, stats_p -> st_std_dev);
														}

												}		/* if (SetJSONReal (stats_json_p, S_VARIANCE_S, stats_p -> st_variance)) */
											else
												{
													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, stats_json_p, "Failed to add \"%s\": \"%s\"", S_VARIANCE_S, stats_p -> st_variance);
												}

										}		/* if (SetJSONReal (stats_json_p, S_SAMPLE_MEAN_S, stats_p -> st_mean)) */
									else
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, stats_json_p, "Failed to add \"%s\": \"%s\"", S_SAMPLE_MEAN_S, stats_p -> st_mean);
										}

								}		/* if (SetJSONReal (stats_json_p, S_MAX_S, stats_p -> st_max)) */
							else
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, stats_json_p, "Failed to add \"%s\": \"%s\"", S_MAX_S, stats_p -> st_max);
								}

						}		/* if (SetJSONReal (stats_json_p, S_MIN_S, stats_p -> st_min)) */
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, stats_json_p, "Failed to add \"%s\": \"%s\"", S_MIN_S, stats_p -> st_min);
						}

				}		/* if (SetJSONInteger (stats_json_p, S_POP_SIZE_S, stats_p -> st_population_size)) */
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, stats_json_p, "Failed to add \"%s\": \"%s\"", S_POP_SIZE_S, stats_p -> st_population_size);
				}

			json_decref (stats_json_p);
		}		/* if (stats_json_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate statistics json object");
		}

	return NULL;
}


Statistics *GetStatisticsFromJSON (const json_t *stats_json_p)
{
	Statistics *stats_p = AllocateStatistics ();

	if (stats_p)
		{
			if (GetJSONLong (stats_json_p, S_POP_SIZE_S, & (stats_p -> st_population_size)))
				{
					if (GetJSONReal (stats_json_p, S_MIN_S, & (stats_p -> st_min)))
						{
							if (GetJSONReal (stats_json_p, S_MAX_S, & (stats_p -> st_max)))
								{
									if (GetJSONReal (stats_json_p, S_SAMPLE_MEAN_S, & (stats_p -> st_mean)))
										{
											if (GetJSONReal (stats_json_p, S_VARIANCE_S, & (stats_p -> st_variance)))
												{
													if (GetJSONReal (stats_json_p, S_STD_DEV_S, & (stats_p -> st_std_dev)))
														{
															if (GetJSONReal (stats_json_p, S_SUM_S, & (stats_p -> st_sum)))
																{
																	return stats_p;
																}		/* if (GetJSONReal (stats_json_p, S_SUM_S, stats_p -> st_sum)) */
															else
																{
																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, stats_json_p, "Failed to get \"%s\"", S_SUM_S);
																}
														}
													else
														{
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, stats_json_p, "Failed to get \"%s\"", S_STD_DEV_S);
														}
												}
											else
												{
													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, stats_json_p, "Failed to get \"%s\"", S_VARIANCE_S);
												}
										}
									else
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, stats_json_p, "Failed to get \"%s\"", S_SAMPLE_MEAN_S);
										}
								}
							else
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, stats_json_p, "Failed to get \"%s\"", S_MAX_S);
								}
						}
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, stats_json_p, "Failed to get \"%s\"", S_MIN_S);
						}
				}
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, stats_json_p, "Failed to get \"%s\"", S_POP_SIZE_S);
				}

			FreeStatistics (stats_p);
		}		/* if (stats_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate statistics");
		}

	return NULL;
}

