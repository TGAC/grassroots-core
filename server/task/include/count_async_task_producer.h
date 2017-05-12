/*
 * count_async_task_resource.h
 *
 *  Created on: 9 May 2017
 *      Author: billy
 */

#ifndef CORE_SERVER_TASK_INCLUDE_COUNT_ASYNC_TASK_PRODUCER_H_
#define CORE_SERVER_TASK_INCLUDE_COUNT_ASYNC_TASK_PRODUCER_H_

#include "async_task_producer.h"


typedef struct CountAsyncTaskProducer
{
	AsyncTaskProducer catp_base_producer;
	int32 catp_current_value;
	int32 catp_limit;
} CountAsyncTaskProducer;



#ifdef __cplusplus
extern "C"
{
#endif



GRASSROOTS_TASK_API CountAsyncTaskProducer *AllocateCountAsyncTaskProducer (int32 limit);


GRASSROOTS_TASK_API void FreeCountAsyncTaskProducer (CountAsyncTaskProducer *producer_p);


GRASSROOTS_TASK_API bool IncrementCountAsyncTaskProducer (CountAsyncTaskProducer *producer_p);


GRASSROOTS_TASK_API bool HasCountAsyncTaskProducerFinished (const CountAsyncTaskProducer *producer_p);


#ifdef __cplusplus
}
#endif


#endif /* CORE_SERVER_TASK_INCLUDE_COUNT_ASYNC_TASK_PRODUCER_H_ */
