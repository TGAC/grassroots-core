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

#endif /* CORE_SERVER_TASK_INCLUDE_COUNT_ASYNC_TASK_CONSUMER_H_ */
