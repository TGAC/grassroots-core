/*
 * async_task_monitor.h
 *
 *  Created on: 9 May 2017
 *      Author: billy
 */

#ifndef CORE_SERVER_TASK_INCLUDE_ASYNC_TASK_CONSUMER_H_
#define CORE_SERVER_TASK_INCLUDE_ASYNC_TASK_CONSUMER_H_


#include "grassroots_task_library.h"
#include "typedefs.h"

#include "async_task.h"

/**
 * A datatype to communicate between AsyncTasks.
 */

typedef struct AsyncTaskConsumer
{
	AsyncTask *atc_task_p;
	bool (*atr_continue_fn) (struct AsyncTaskResource *resource_p);
	void *atr_data_p;
	void (*atr_free_data_fn) (void *data_p);
} AsyncTaskConsumer;



#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Create an AsyncTaskConsumer.
 *
 * @return The new AsyncTaskConsumer or <code>NULL</code> upon error.
 * @memberof AsyncTaskConsumer
 */
GRASSROOTS_TASK_API	AsyncTaskConsumer *CreateAsyncTaskConsumer (void);


/**
 * Free an AsyncTaskConsumer.
 *
 * @param task_p The AsyncTaskConsumer to free.
 * @memberof AsyncTaskConsumer
 */
GRASSROOTS_TASK_API	void FreeAsyncTaskConsumer (AsyncTaskConsumer *consumer_p);


GRASSROOTS_TASK_API bool InitAsyncTaskConsumer (AsyncTaskConsumer *consumer_p);


GRASSROOTS_TASK_API void ClearAsyncTaskConsumer (AsyncTaskConsumer *consumer_p);



#ifdef __cplusplus
}
#endif




#endif /* CORE_SERVER_TASK_INCLUDE_ASYNC_TASK_CONSUMER_H_ */
