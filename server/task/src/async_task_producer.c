/*
 * async_task_resource.c
 *
 *  Created on: 9 May 2017
 *      Author: billy
 */

#include "async_task_producer.h"

#include <stddef.h>

#include "memory_allocations.h"


void SetAsyncTaskResourceData (AsyncTaskProducer *producer_p, void *data_p, void (*free_data_fn) (void *data_p))
{
	//producer_p -> atp_data_p = data_p;
	//producer_p -> atp_free_data_fn = free_data_fn;
}


bool InitAsyncTaskResource (AsyncTaskProducer *producer_p)
{
//	producer_p -> atp_continue_fn = NULL;
//	producer_p -> atp_data_p = NULL;
//	producer_p -> atp_free_data_fn = NULL;
	//producer_p -> atp_signal_fn = NULL;

	return true;
}


void ClearAsyncTaskResource (AsyncTaskProducer *producer_p)
{
/*
	if (producer_p -> atp_data_p)
		{
			if (producer_p -> atp_free_data_fn)
				{
					producer_p -> atp_free_data_fn (producer_p -> atp_data_p);
				}
			else
				{
					FreeMemory (producer_p -> atp_data_p);
				}
		}
*/
}
