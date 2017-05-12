/*
 * count_async_task_resource.c
 *
 *  Created on: 9 May 2017
 *      Author: billy
 */

#include "count_async_task_producer.h"
#include "streams.h"
#include "memory_allocations.h"


CountAsyncTaskProducer *AllocateCountAsyncTaskProducer (int32 limit)
{
	CountAsyncTaskProducer *producer_p = (CountAsyncTaskProducer *) AllocMemory (sizeof (CountAsyncTaskProducer));

	if (producer_p)
		{
			if (InitAsyncTaskProducer (& (producer_p -> catp_base_producer)))
				{
					producer_p -> catp_current_value = 0;
					producer_p -> catp_limit = limit;

					return producer_p;
				}		/* if (InitAsyncTaskProducer (& (producer_p -> catp_base_producer))) */

			FreeMemory (producer_p);
		}		/* if (producer_p) */

	return NULL;
}


void FreeCountAsyncTaskProducer (CountAsyncTaskProducer *producer_p)
{
	ClearAsyncTaskProducer (& (producer_p -> catp_base_producer));
	FreeMemory (producer_p);
}



bool IncrementCountAsyncTaskProducer (CountAsyncTaskProducer *producer_p)
{
	bool success_flag = false;

	if (LockAsyncTaskProducer (& (producer_p -> catp_base_producer)))
		{
			++ (producer_p -> catp_current_value);

			success_flag = true;

			if (producer_p -> catp_current_value == producer_p -> catp_limit)
				{
					/* send signal */
					FireAsyncTaskProducer (& (producer_p -> catp_base_producer));
				}

			if (!UnlockAsyncTaskProducer (& (producer_p -> catp_base_producer)))
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to unlock CountingAsyncTaskProducer");
				}
		}

	return success_flag;
}


bool HasCountAsyncTaskProducerFinished (const CountAsyncTaskProducer *producer_p)
{
	return (producer_p -> catp_current_value == producer_p -> catp_limit);
}
