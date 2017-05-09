/*
 * count_async_task_resource.c
 *
 *  Created on: 9 May 2017
 *      Author: billy
 */

#include "count_async_task_resource.h"
#include "streams.h"


bool IncrementCountingAsyncTaskResource (struct CountingAsyncTaskResource *resource_p)
{
	bool success_flag = false;

	if (LockAsyncTaskResource (resource_p -> catr_base_resource_p))
		{
			++ (resource_p -> catr_current_value);

			success_flag = true;

			if (resource_p -> catr_current_value == resource_p -> catr_limit)
				{
					/* send signal */
					FireAsyncTaskResource (resource_p -> catr_base_resource_p);
				}

			if (!UnlockAsyncTaskResource (resource_p -> catr_base_resource_p))
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to unlock CountingAsyncTaskResource");
				}
		}

	return success_flag;
}

