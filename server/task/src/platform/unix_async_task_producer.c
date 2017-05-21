/*
 * unix_async_task_resource.c
 *
 *  Created on: 9 May 2017
 *      Author: billy
 */


#include <errno.h>

#include "pthread.h"
#include "unix_async_task_producer.h"

#include "typedefs.h"
#include "streams.h"
#include "memory_allocations.h"




void FreeAsyncTaskProducer (struct AsyncTaskProducer *producer_p)
{
	UnixAsyncTaskProducer *unix_producer_p = (UnixAsyncTaskProducer *) producer_p;

	ClearAsyncTaskProducer (producer_p);

	FreeMemory (unix_producer_p);
}



void FireAsyncTaskProducer (AsyncTaskProducer *producer_p)
{
	UnixAsyncTaskProducer *unix_producer_p = (UnixAsyncTaskProducer *) producer_p;

	pthread_cond_signal (& (unix_producer_p -> uatp_task_status_cond));
}


/*
void WaitOnAsyncTaskProducer (AsyncTaskProducer *producer_p)
{
	UnixAsyncTaskProducer *unix_producer_p = (UnixAsyncTaskProducer *) producer_p;

	if (LockAsyncTaskProducer (producer_p))
		{
			while (producer_p -> atp_continue_fn (producer_p))
				{
					pthread_cond_wait (& (unix_producer_p -> uatp_task_status_cond), & (unix_producer_p -> uatp_task_status_mutex));
				}

			if (!UnlockAsyncTaskProducer (producer_p))
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to unlock CountingAsyncTaskProducer mutex");
				}
		}
}
*/


