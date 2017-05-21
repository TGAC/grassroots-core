/*
 * count_async_task_monitor.h
 *
 *  Created on: 12 May 2017
 *      Author: billy
 */

#ifndef CORE_SERVER_TASK_INCLUDE_COUNT_ASYNC_TASK_CONSUMER_H_
#define CORE_SERVER_TASK_INCLUDE_COUNT_ASYNC_TASK_CONSUMER_H_

#include "async_task_consumer.h"


typedef struct CountAsyncTaskConsumer
{
	AsyncTaskConsumer catc_base_consumer;
	int32 catc_current_value;
	int32 catc_limit;

} CountAsyncTaskConsumer;


#ifdef __cplusplus
extern "C"
{
#endif


GRASSROOTS_TASK_API CountAsyncTaskConsumer *AllocateCountAsyncTaskConsumer (int32 limit);


GRASSROOTS_TASK_API void FreeCountAsyncTaskConsumer (CountAsyncTaskConsumer *consumer_p);


GRASSROOTS_TASK_API bool WatchCountAsyncTaskConsumer (CountAsyncTaskConsumer *consumer_p);



#ifdef __cplusplus
}
#endif


#endif /* CORE_SERVER_TASK_INCLUDE_COUNT_ASYNC_TASK_CONSUMER_H_ */
