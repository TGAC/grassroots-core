/*
 * async_task_resource.h
 *
 *  Created on: 9 May 2017
 *      Author: billy
 */

#ifndef CORE_SERVER_TASK_INCLUDE_ASYNC_TASK_RESOURCE_H_
#define CORE_SERVER_TASK_INCLUDE_ASYNC_TASK_RESOURCE_H_

#include "grassroots_task_library.h"


/* forward declaration, as AsyncTaskResource will be platform-specific */
/**
 * A datatype to communicate between AsyncTasks.
 */
struct AsyncTaskResource;


/**
 * Create an AsyncTaskResource.
 *
 * @return The new AsyncTaskResource or <code>NULL</code> upon error.
 * @memberof AsyncTaskResource
 */
GRASSROOTS_TASK_API	struct AsyncTaskResource *CreateAsyncTaskResource (void);


/**
 * Free an AsyncTaskResource.
 *
 * @param task_p The AsyncTaskResource to free.
 * @memberof AsyncTaskResource
 */
GRASSROOTS_TASK_API	void FreeAsyncTaskResource (struct AsyncTaskResource *resource_p);


GRASSROOTS_TASK_API bool LockAsyncTaskMonitorResource (struct AsyncTaskResource *resource_p);


GRASSROOTS_TASK_API bool UnlockAsyncTaskMonitorResource (struct AsyncTaskResource *resource_p);


#endif /* CORE_SERVER_TASK_INCLUDE_ASYNC_TASK_RESOURCE_H_ */
