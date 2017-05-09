/*
 * async_task_resource.h
 *
 *  Created on: 9 May 2017
 *      Author: billy
 */

#ifndef CORE_SERVER_TASK_INCLUDE_ASYNC_TASK_RESOURCE_H_
#define CORE_SERVER_TASK_INCLUDE_ASYNC_TASK_RESOURCE_H_

#include "grassroots_task_library.h"
#include "typedefs.h"



/**
 * A datatype to communicate between AsyncTasks.
 */

typedef struct AsyncTaskResource
{
	bool (*atr_continue_fn) (struct AsyncTaskResource *resource_p);
	void *atr_data_p;
	void (*atr_free_data_fn) (void *data_p);
} AsyncTaskResource;


/**
 * Create an AsyncTaskResource.
 *
 * @return The new AsyncTaskResource or <code>NULL</code> upon error.
 * @memberof AsyncTaskResource
 */
GRASSROOTS_TASK_API	AsyncTaskResource *CreateAsyncTaskResource (void);


/**
 * Free an AsyncTaskResource.
 *
 * @param task_p The AsyncTaskResource to free.
 * @memberof AsyncTaskResource
 */
GRASSROOTS_TASK_API	void FreeAsyncTaskResource (AsyncTaskResource *resource_p);


GRASSROOTS_TASK_API bool InitAsyncTaskResource (AsyncTaskResource *resource_p);


GRASSROOTS_TASK_API void ClearAsyncTaskResource (AsyncTaskResource *resource_p);


GRASSROOTS_TASK_API bool LockAsyncTaskResource (AsyncTaskResource *resource_p);


GRASSROOTS_TASK_API bool UnlockAsyncTaskResource (AsyncTaskResource *resource_p);


GRASSROOTS_TASK_API void FireAsyncTaskResource (AsyncTaskResource *resource_p);


GRASSROOTS_TASK_API	void SetAsyncTaskResourceData (AsyncTaskResource *resource_p, void *data_p, void (*free_data_fn) (void *data_p));


#endif /* CORE_SERVER_TASK_INCLUDE_ASYNC_TASK_RESOURCE_H_ */
