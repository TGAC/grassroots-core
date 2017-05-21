/*
 * count_async_task_resource.h
 *
 *  Created on: 9 May 2017
 *      Author: billy
 */

#ifndef CORE_SERVER_TASK_INCLUDE_COUNT_ASYNC_TASK_PRODUCER_H_
#define CORE_SERVER_TASK_INCLUDE_COUNT_ASYNC_TASK_PRODUCER_H_

#include "async_task_producer.h"


typedef struct CountAsyncTask
{
	AsyncTask *cat_task_p;
	int32 cat_current_value;
	int32 cat_limit;
} CountAsyncTask;



#ifdef __cplusplus
extern "C"
{
#endif



GRASSROOTS_TASK_API CountAsyncTask *AllocateCountAsyncTask (int32 limit);


GRASSROOTS_TASK_API void FreeCountAsyncTask (CountAsyncTask *count_task_p);


GRASSROOTS_TASK_API bool IncrementCountAsyncTask (CountAsyncTask *count_task_p);


GRASSROOTS_TASK_API bool HasCountAsyncTaskFinished (const CountAsyncTask *count_task_p);


#ifdef __cplusplus
}
#endif


#endif /* CORE_SERVER_TASK_INCLUDE_COUNT_ASYNC_TASK_PRODUCER_H_ */
