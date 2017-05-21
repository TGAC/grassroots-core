/*
 * count_async_task_resource.c
 *
 *  Created on: 9 May 2017
 *      Author: billy
 */

#include "count_async_task_producer.h"
#include "streams.h"
#include "memory_allocations.h"


CountAsyncTask *AllocateCountAsyncTaskProducer (int32 limit)
{
	CountAsyncTask *count_task_p = (CountAsyncTask *) AllocMemory (sizeof (CountAsyncTask));

	if (count_task_p)
		{
			if (InitAsyncTaskProducer (count_task_p -> catp_producer_p))
				{
					count_task_p -> cat_current_value = 0;
					count_task_p -> cat_limit = limit;

					return count_task_p;
				}		/* if (InitAsyncTaskProducer (& (producer_p -> catp_base_producer))) */

			FreeMemory (count_task_p);
		}		/* if (count_task_p) */

	return NULL;
}


void FreeCountAsyncTask (CountAsyncTask *count_task_p)
{
	FreeAsyncTaskProducer (producer_p -> catp_producer_p);
	FreeMemory (count_task_p);
}



bool IncrementCountAsyncTask (CountAsyncTask *count_task_p)
{
	bool success_flag = false;
	SyncData *sync_data_p = count_task_p -> cat_task_p -> at_sync_data_p;

	if (sync_data_p)
		{
			if (AcquireSyncDataLock (sync_data_p))
				{
					++ (count_task_p -> cat_current_value);

					success_flag = true;

					if (count_task_p -> cat_current_value == count_task_p -> cat_limit)
						{
							/* send signal */
							SendSyncData (sync_data_p);
						}

					if (!ReleaseSyncDataLock (sync_data_p))
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to unlock CountingAsyncTaskProducer");
						}
				}
		}

	return success_flag;
}


bool HasCountAsyncTaskProducerFinished (const CountAsyncTask *count_task_p)
{
	return (count_task_p -> cat_current_value == count_task_p -> cat_limit);
}
