/*
 * system_async_task.c
 *
 *  Created on: 3 May 2017
 *      Author: billy
 */

#include "system_async_task.h"
#include "memory_allocations.h"

SystemTaskData *CreateSystemTaskData (JobsManager *jobs_manager_p, ServiceJob *job_p, const char *command_s)
{
	struct AsyncTask *async_task_p = CreateAsyncTask ();

	if (async_task_p)
		{
			SystemTaskData *task_data_p = (SystemTaskData *) AllocMemory (sizeof (SystemTaskData));

			if (task_data_p)
				{
					task_data_p -> std_async_task_p = async_task_p;
					task_data_p -> std_command_line_s = command_s;
					task_data_p -> std_jobs_manager_p = jobs_manager_p;
					task_data_p -> std_service_job_p = job_p;

					return task_data_p;
				}

			FreeAsyncTask (async_task_p);
		}

	return NULL;
}


void FreeSystemTaskData (SystemTaskData *task_data_p)
{
	FreeAsyncTask (task_data_p -> std_async_task_p);
	FreeMemory (task_data_p);
}


void *RunAsyncSystemTask (void *data_p)
{
	SystemTaskData *task_data_p = (SystemTaskData *) data_p;
	OperationStatus status = OS_STARTED;

	SetServiceJobStatus (task_data_p -> std_service_job_p, status);

	if (AddServiceJobToJobsManager (task_data_p -> std_jobs_manager_p, task_data_p -> std_service_job_p -> sj_id, task_data_p -> std_service_job_p))
		{
			int res = system (task_data_p -> std_command_line_s);

			if (res != -1)
				{
					int process_exit_code = WEXITSTATUS (res);

					if (process_exit_code == 0)
						{
							status = OS_SUCCEEDED;
						}
					else
						{
							status = OS_ERROR;
						}
				}
			else
				{
					status = OS_ERROR;
				}
		}
	else
		{
			status = OS_FAILED_TO_START;
		}

	SetServiceJobStatus (task_data_p -> std_service_job_p, status);

	return (void *) status;
}
