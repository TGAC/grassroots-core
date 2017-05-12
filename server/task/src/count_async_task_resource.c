/*
 * count_async_task_resource.c
 *
 *  Created on: 9 May 2017
 *      Author: billy
 */

#include "count_async_task_resource.h"
#include "streams.h"
#include "memory_allocations.h"


CountAsyncTaskResource *AllocateCountAsyncTaskResource (int32 limit)
{
	CountAsyncTaskResource *resource_p = (CountAsyncTaskResource *) AllocMemory (sizeof (CountAsyncTaskResource));

	if (resource_p)
		{
			if (InitAsyncTaskResource (& (resource_p -> catr_base_resource)))
				{
					resource_p -> catr_current_value = 0;
					resource_p -> catr_limit = limit;

					return resource_p;
				}		/* if (InitAsyncTaskResource (& (resource_p -> catr_base_resource))) */

			FreeMemory (resource_p);
		}		/* if (resource_p) */

	return NULL;
}


void FreeCountAsyncTaskResource (CountAsyncTaskResource *resource_p)
{
	ClearAsyncTaskResource (& (resource_p -> catr_base_resource));
	FreeMemory (resource_p);
}



bool IncrementCountAsyncTaskResource (CountAsyncTaskResource *resource_p)
{
	bool success_flag = false;

	if (LockAsyncTaskResource (& (resource_p -> catr_base_resource)))
		{
			++ (resource_p -> catr_current_value);

			success_flag = true;

			if (resource_p -> catr_current_value == resource_p -> catr_limit)
				{
					/* send signal */
					FireAsyncTaskResource (& (resource_p -> catr_base_resource));
				}

			if (!UnlockAsyncTaskResource (& (resource_p -> catr_base_resource)))
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to unlock CountingAsyncTaskResource");
				}
		}

	return success_flag;
}


bool HasCountAsyncTaskResourceFinished (const CountAsyncTaskResource *resource_p)
{
	return (resource_p -> catr_current_value == resource_p -> catr_limit);
}
