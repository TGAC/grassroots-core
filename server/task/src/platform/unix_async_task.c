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



AsyncTask *CreateAsyncTask (const char *name_s)
{
	UnixAsyncTask *task_p = (UnixAsyncTask *) AllocMemory (sizeof (struct UnixAsyncTask));

	if (task_p)
		{
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


bool RunAsyncTask (AsyncTask *task_p, void * (*run_fn) (void *data_p), void *task_data_p)
{
	bool success_flag = true;
	UnixAsyncTask *unix_task_p = (UnixAsyncTask *) task_p;
	int res = pthread_create (& (unix_task_p -> uat_thread), & (unix_task_p -> uat_attributes), run_fn, task_data_p);

	if (res == 0)
		{
			unix_task_p -> uat_valid_thread_flag = true;
		}
	else
		{
			unix_task_p -> uat_valid_thread_flag = false;
			success_flag = false;
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create task for RunThreadedSystemTask %d", res);
		}

	return success_flag;
}
