/*
 * count_async_task_resource.h
 *
 *  Created on: 9 May 2017
 *      Author: billy
 */

#ifndef CORE_SERVER_TASK_INCLUDE_COUNT_ASYNC_TASK_RESOURCE_H_
#define CORE_SERVER_TASK_INCLUDE_COUNT_ASYNC_TASK_RESOURCE_H_

#include "async_task_resource.h"


typedef struct CountingAsyncTaskResource
{
	struct AsyncTaskResource *catr_base_resource_p;
	int32 catr_current_value;
	int32 catr_limit;
} CountingAsyncTaskResource;


#endif /* CORE_SERVER_TASK_INCLUDE_COUNT_ASYNC_TASK_RESOURCE_H_ */
