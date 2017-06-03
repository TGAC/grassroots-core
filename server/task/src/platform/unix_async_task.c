/*
** Copyright 2014-2016 The Earlham Institute
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

/**
 * unix_process.c
 *
 *  Created on: 2 Feb 2017
 *      Author: billy
 *
 * @file
 * @brief
 */

#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <pthread.h>

#include "streams.h"

#include "async_task.h"
#include "memory_allocations.h"
#include "string_utils.h"


typedef struct UnixAsyncTask
{
	AsyncTask uat_base_task;
	pthread_t uat_thread;
	bool uat_valid_thread_flag;
	pthread_attr_t uat_attributes;
} UnixAsyncTask;


typedef struct AsyncTaskData
{
	AsyncTask atd_task;
	void *atd_data_p;
} AsyncTaskData;


static void *DoAsyncTaskRun (void *data_p);


AsyncTask *AllocateAsyncTask (const char *name_s)
{
	UnixAsyncTask *task_p = (UnixAsyncTask *) AllocMemory (sizeof (struct UnixAsyncTask));

	if (task_p)
		{
			memset (task_p, 0, sizeof (UnixAsyncTask));

			if (InitialiseAsyncTask (& (task_p -> uat_base_task), name_s))
				{
					task_p -> uat_thread = 0;
					task_p -> uat_valid_thread_flag = false;

					/* For portability, explicitly create threads in a joinable state */
					pthread_attr_init (& (task_p -> uat_attributes));
					pthread_attr_setdetachstate (& (task_p -> uat_attributes), PTHREAD_CREATE_JOINABLE);

					return (& (task_p -> uat_base_task));
				}

			FreeMemory (task_p);
		}

	return NULL;
}


void FreeAsyncTask (AsyncTask *task_p)
{
	CloseAsyncTask (task_p);

	FreeMemory (task_p);
}


void CloseAsyncTask (AsyncTask *task_p)
{
	UnixAsyncTask *unix_task_p = (UnixAsyncTask *) task_p;

	if (unix_task_p -> uat_valid_thread_flag)
		{
			unix_task_p -> uat_valid_thread_flag = false;
		}

	pthread_attr_destroy (& (unix_task_p -> uat_attributes));

}


void SetAsyncTaskSyncData (AsyncTask *task_p, SyncData *sync_data_p, MEM_FLAG mem)
{
	if (task_p -> at_sync_data_p)
		{
			if ((task_p -> at_sync_data_mem == MF_DEEP_COPY) || (task_p -> at_sync_data_mem == MF_SHALLOW_COPY))
				{
					FreeSyncData (task_p -> at_sync_data_p);
				}
		}

	task_p -> at_sync_data_p = sync_data_p;
	task_p -> at_sync_data_mem = mem;
}


bool IsAsyncTaskRunning (const AsyncTask *task_p)
{
	UnixAsyncTask *unix_task_p = (UnixAsyncTask *) task_p;

	return (unix_task_p -> uat_valid_thread_flag);
}


bool CloseAllAsyncTasks (void)
{
	//pthread_exit (NULL);
	return true;
}


bool RunAsyncTask (AsyncTask *task_p)
{
	bool success_flag = true;
	UnixAsyncTask *unix_task_p = (UnixAsyncTask *) task_p;
	int res = pthread_create (& (unix_task_p -> uat_thread), & (unix_task_p -> uat_attributes), DoAsyncTaskRun, task_p);

	if (res == 0)
		{
			unix_task_p -> uat_valid_thread_flag = true;
		}
	else
		{
			unix_task_p -> uat_valid_thread_flag = false;
			success_flag = false;
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create task for RunAsyncTask %d", res);
		}

	return success_flag;
}


static void *DoAsyncTaskRun (void *data_p)
{
	AsyncTask *async_task_p = (AsyncTask *) data_p;

	void *res_p = async_task_p -> at_run_fn (async_task_p -> at_data_p);

	if (async_task_p -> at_consumer_p)
		{
			RunEventConsumer (async_task_p -> at_consumer_p, async_task_p);
		}

	return res_p;
}
