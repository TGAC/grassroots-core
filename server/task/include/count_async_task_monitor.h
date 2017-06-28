/*
 * count_async_task_monitor.h
 *
 *  Created on: 12 May 2017
 *      Author: billy
 */

#ifndef CORE_SERVER_TASK_INCLUDE_COUNT_ASYNC_TASK_MONITOR_H_
#define CORE_SERVER_TASK_INCLUDE_COUNT_ASYNC_TASK_MONITOR_H_



typedef struct CountingAsyncTaskMonitor
{
	AsyncTaskResource catr_base_resource;
	int32 catr_current_value;
	int32 catr_limit;
} CountingAsyncTaskResource;



#ifdef __cplusplus
extern "C"
{
#endif



GRASSROOTS_TASK_API CountingAsyncTaskResource *AllocateCountingAsyncTaskResource (int32 limit);


GRASSROOTS_TASK_API bool IncrementCountingAsyncTaskResource (struct CountingAsyncTaskResource *resource_p);


#ifdef __cplusplus
}
#endif

#endif /* CORE_SERVER_TASK_INCLUDE_COUNT_ASYNC_TASK_MONITOR_H_ */
