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


struct AsyncTask
{
	pthread_t at_thread;
	bool at_valid_thread_flag;
	bool at_detach_flag;
};


typedef struct SystemAsyncTaskData
{
	struct AsyncTask *task_p;
} SystemAsyncTaskData;


struct AsyncTask *CreateAsyncTask (bool detach_flag)
{
	struct AsyncTask *task_p = (struct AsyncTask *) AllocMemory (sizeof (struct AsyncTask));

	if (task_p)
		{
			task_p -> at_thread = 0;
			task_p -> at_valid_thread_flag = false;
			task_p -> at_detach_flag = detach_flag;
		}

	return task_p;
}


void FreeAsyncTask (struct AsyncTask *task_p)
{
	CloseAsyncTask (task_p);

	FreeMemory (task_p);
}


void CloseAsyncTask (struct AsyncTask *task_p)
{
	if (task_p -> at_valid_thread_flag)
		{
			if (! (task_p -> at_detach_flag))
				{
					pthread_join (task_p -> at_thread, NULL);
				}

			task_p -> at_valid_thread_flag = false;
		}
}


bool IsAsyncTaskRunning (const struct AsyncTask *task_p)
{
	return task_p -> at_valid_thread_flag;
}


bool CloseAllAsyncTasks (void)
{
	pthread_exit (NULL);
	return true;
}


bool RunAsyncTask (struct AsyncTask *task_p, void * (*run_fn) (void *data_p), void *task_data_p)
{
	bool success_flag = true;
	int res = pthread_create (& (task_p -> at_thread), NULL, run_fn, task_data_p);

	if (res == 0)
		{
			task_p -> at_valid_thread_flag = true;

			if (task_p -> at_detach_flag)
				{
					res = pthread_detach (task_p -> at_thread);

					if (res != 0)
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to detach task for RunThreadedSystemTask %d", res);
							task_p -> at_detach_flag = false;
						}
				}
		}
	else
		{
			task_p -> at_valid_thread_flag = false;
			success_flag = false;
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create task for RunThreadedSystemTask %d", res);
		}

	return success_flag;
}
