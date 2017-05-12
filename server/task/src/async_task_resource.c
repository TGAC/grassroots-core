/*
 * async_task_resource.c
 *
 *  Created on: 9 May 2017
 *      Author: billy
 */

#include <stddef.h>

#include "async_task_resource.h"
#include "memory_allocations.h"


void SetAsyncTaskResourceData (AsyncTaskResource *resource_p, void *data_p, void (*free_data_fn) (void *data_p))
{
	resource_p -> atr_data_p = data_p;
	resource_p -> atr_free_data_fn = free_data_fn;
}


bool InitAsyncTaskResource (AsyncTaskResource *resource_p)
{
	resource_p -> atr_continue_fn = NULL;
	resource_p -> atr_data_p = NULL;
	resource_p -> atr_free_data_fn = NULL;
	//resource_p -> atr_signal_fn = NULL;

	return true;
}


void ClearAsyncTaskResource (AsyncTaskResource *resource_p)
{
	if (resource_p -> atr_data_p)
		{
			if (resource_p -> atr_free_data_fn)
				{
					resource_p -> atr_free_data_fn (resource_p -> atr_data_p);
				}
			else
				{
					FreeMemory (resource_p -> atr_data_p);
				}
		}
}
