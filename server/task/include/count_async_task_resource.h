/*
 * count_async_task_resource.h
 *
 *  Created on: 9 May 2017
 *      Author: billy
 */

#ifndef CORE_SERVER_TASK_INCLUDE_COUNT_ASYNC_TASK_RESOURCE_H_
#define CORE_SERVER_TASK_INCLUDE_COUNT_ASYNC_TASK_RESOURCE_H_

#include "async_task_resource.h"


typedef struct CountAsyncTaskResource
{
	AsyncTaskResource catr_base_resource;
	int32 catr_current_value;
	int32 catr_limit;
} CountAsyncTaskResource;



#ifdef __cplusplus
extern "C"
{
#endif



GRASSROOTS_TASK_API CountAsyncTaskResource *AllocateCountAsyncTaskResource (int32 limit);


GRASSROOTS_TASK_API bool IncrementCountAsyncTaskResource (CountAsyncTaskResource *resource_p);


GRASSROOTS_TASK_API bool HasCountAsyncTaskResourceFinished (const CountAsyncTaskResource *resource_p);

#ifdef __cplusplus
}
#endif


#endif /* CORE_SERVER_TASK_INCLUDE_COUNT_ASYNC_TASK_RESOURCE_H_ */
