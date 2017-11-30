/*
 * count_async_task_resource.c
 *
 *  Created on: 9 May 2017
 *      Author: billy
 */

#include "count_async_task.h"
#include "async_tasks_manager.h"

#include "streams.h"
#include "memory_allocations.h"


CountAsyncTask *AllocateCountAsyncTask (const char *name_s, AsyncTasksManager *manager_p, bool add_flag, int32 limit)
{
	AsyncTask *async_task_p = AllocateAsyncTask (name_s, manager_p, add_flag);

	if (async_task_p)
		{
			CountAsyncTask *count_task_p = (CountAsyncTask *) AllocMemory (sizeof (CountAsyncTask));

			if (count_task_p)
				{
					count_task_p -> cat_task_p = async_task_p;
					count_task_p -> cat_current_value = 0;
					count_task_p -> cat_limit = limit;

					return count_task_p;
				}		/* if (count_task_p) */

			FreeAsyncTask (async_task_p);
		}

	return NULL;
}


void FreeCountAsyncTask (CountAsyncTask *count_task_p)
{
	ClearCountAsyncTask (count_task_p);
	FreeMemory (count_task_p);
}


bool InitCountAsyncTask (CountAsyncTask *count_task_p, const char *name_s, AsyncTasksManager *manager_p, bool add_flag, int32 limit)
{
	bool success_flag = false;
	AsyncTask *async_task_p = AllocateAsyncTask (name_s, manager_p, add_flag);

	if (async_task_p)
		{
			count_task_p -> cat_task_p = async_task_p;
			count_task_p -> cat_current_value = 0;
			count_task_p -> cat_limit = limit;

			success_flag = true;
		}		/* if (count_task_p) */

	return success_flag;
}


void ClearCountAsyncTask (CountAsyncTask *count_task_p)
{
	if (count_task_p -> cat_task_p)
		{
			FreeAsyncTask (count_task_p -> cat_task_p);
			count_task_p -> cat_task_p = NULL;
		}
}


bool IncrementCountAsyncTask (CountAsyncTask *count_task_p)
{
	bool success_flag = false;
	SyncData *sync_data_p = count_task_p -> cat_task_p -> at_sync_data_p;

	if (sync_data_p)
		{
			if (AcquireSyncDataLock (sync_data_p))
				{
					/*
					 * SendSyncData () attempts to get the lock which we
					 * currently have. So if we need to call SendSyncData ()
					 * we need to release our lock first. So we'll use a flag
					 * which we'll check after the count limit check, to avoid
					 * a deadlock.
					 */
					bool notify_flag = false;

					++ (count_task_p -> cat_current_value);

					success_flag = true;

					if (count_task_p -> cat_current_value == count_task_p -> cat_limit)
						{
							notify_flag = true;
						}

					if (ReleaseSyncDataLock (sync_data_p))
						{
							/* send signal */
							if (notify_flag)
								{
									SendSyncData (sync_data_p);
								}

						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to unlock CountingAsyncTaskProducer");
						}
				}
		}

	return success_flag;
}


bool ContinueCountAsyncTask (const CountAsyncTask *count_task_p)
{
	return (count_task_p -> cat_current_value < count_task_p -> cat_limit);
}


void SetCountAsyncTaskLimit (CountAsyncTask *task_p, int32 limit)
{
	task_p -> cat_limit = limit;
	task_p -> cat_current_value = 0;
}


AsyncTask *GetAsyncTaskFromCountAsyncTask (CountAsyncTask *count_task_p)
{
	return (count_task_p -> cat_task_p);
}


