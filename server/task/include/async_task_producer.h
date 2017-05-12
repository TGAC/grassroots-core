/*
 * async_task_resource.h
 *
 *  Created on: 9 May 2017
 *      Author: billy
 */

#ifndef CORE_SERVER_TASK_INCLUDE_ASYNC_TASK_PRODUCER_H_
#define CORE_SERVER_TASK_INCLUDE_ASYNC_TASK_PRODUCER_H_

#include "grassroots_task_library.h"
#include "typedefs.h"

#include "async_task.h"

/**
 * A datatype to communicate between AsyncTasks.
 */

typedef struct AsyncTaskProducer
{
	AsyncTask *atp_task_p;
	bool (*atp_continue_fn) (struct AsyncTaskProducer *producer_p);
	void *atp_data_p;
	void (*atp_free_data_fn) (void *data_p);
} AsyncTaskProducer;



#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Create an AsyncTaskProducer.
 *
 * @return The new AsyncTaskProducer or <code>NULL</code> upon error.
 * @memberof AsyncTaskProducer
 */
GRASSROOTS_TASK_API	AsyncTaskProducer *CreateAsyncTaskProducer (void);


/**
 * Free an AsyncTaskProducer.
 *
 * @param task_p The AsyncTaskProducer to free.
 * @memberof AsyncTaskProducer
 */
GRASSROOTS_TASK_API	void FreeAsyncTaskProducer (AsyncTaskProducer *producer_p);


GRASSROOTS_TASK_API bool InitAsyncTaskProducer (AsyncTaskProducer *producer_p);


GRASSROOTS_TASK_API void ClearAsyncTaskProducer (AsyncTaskProducer *producer_p);


GRASSROOTS_TASK_API bool LockAsyncTaskProducer (AsyncTaskProducer *producer_p);


GRASSROOTS_TASK_API bool UnlockAsyncTaskProducer (AsyncTaskProducer *producer_p);


GRASSROOTS_TASK_API void FireAsyncTaskProducer (AsyncTaskProducer *producer_p);


GRASSROOTS_TASK_API	void SetAsyncTaskProducerData (AsyncTaskProducer *producer_p, void *data_p, void (*free_data_fn) (void *data_p));



#ifdef __cplusplus
}
#endif


#endif /* CORE_SERVER_TASK_INCLUDE_ASYNC_TASK_PRODUCER_H_ */
