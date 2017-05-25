/*
 * system_async_task.c
 *
 *  Created on: 3 May 2017
 *      Author: billy
 */

#include "system_async_task.h"
#include "memory_allocations.h"
#include "string_utils.h"


#ifdef _DEBUG
	#define ASYNC_SYSTEM_BLAST_TOOL_DEBUG (STM_LEVEL_FINE)
#else
	#define ASYNC_SYSTEM_BLAST_TOOL_DEBUG (STM_LEVEL_NONE)
#endif


static void *RunAsyncSystemTaskHook (void *data_p);


SystemAsyncTask *AllocateSystemAsyncTask (ServiceJob *job_p, const char *name_s, const char *command_s)
{
	AsyncTask *async_task_p = AllocateAsyncTask (name_s);

	if (async_task_p)
		{
			char *copied_command_s = CopyToNewString (command_s, 0, false);

			if (copied_command_s)
				{
					SystemAsyncTask *task_data_p = (SystemAsyncTask *) AllocMemory (sizeof (SystemAsyncTask));

					if (task_data_p)
						{
							task_data_p -> std_async_task_p = async_task_p;
							task_data_p -> std_command_line_s = copied_command_s;
							task_data_p -> std_service_job_p = job_p;

							return task_data_p;
						}

					FreeCopiedString (copied_command_s);
				}

			FreeAsyncTask (async_task_p);
		}

	return NULL;
}


void FreeSystemAsyncTask (SystemAsyncTask *task_data_p)
{
	FreeAsyncTask (task_data_p -> std_async_task_p);

	if (task_data_p -> std_command_line_s)
		{
			FreeCopiedString (task_data_p -> std_command_line_s);
		}

	DecrementServiceJobReferenceCount (task_data_p -> std_service_job_p);

	FreeMemory (task_data_p);
}


bool RunSystemAsyncTask (SystemAsyncTask *task_data_p)
{
	SetAsyncTaskRunData (task_data_p -> std_async_task_p, RunAsyncSystemTaskHook, task_data_p);

	return RunAsyncTask (task_data_p -> std_async_task_p);
}


static void *RunAsyncSystemTaskHook (void *data_p)
{
	SystemAsyncTask *task_data_p = ((SystemAsyncTask *) data_p);
	JobsManager *jobs_manager_p = GetJobsManager ();
	OperationStatus status = OS_STARTED;
	ServiceJob *job_p = task_data_p -> std_service_job_p;
	char uuid_s [UUID_STRING_BUFFER_SIZE];

	ConvertUUIDToString (job_p -> sj_id, uuid_s);

	SetServiceJobStatus (job_p, status);


	if (AddServiceJobToJobsManager (jobs_manager_p, job_p -> sj_id, job_p))
		{
			#if ASYNC_SYSTEM_BLAST_TOOL_DEBUG >= STM_LEVEL_FINE
			PrintLog (STM_LEVEL_FINE, __FILE__, __LINE__, "About to run RunAsyncSystemTaskHook for %s with \"%s\"", uuid_s, task_data_p -> std_command_line_s);
			#endif

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
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add job %s with status %d to jobs manager", uuid_s, status);
		}

	SetServiceJobStatus (job_p, status);

	/* has the job status changed? */
	if (status != OS_STARTED)
		{
			if (! (AddServiceJobToJobsManager (jobs_manager_p, job_p -> sj_id, job_p)))
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add job %s with status %d to jobs manager", uuid_s, status);
				}

			if (task_data_p -> std_async_task_p -> at_consumer_p)
				{
					RunEventConsumerFromAsyncTask (task_data_p -> std_async_task_p);
					//SendSyncData (task_data_p -> std_async_task_p -> at_sync_data_p);
				}
		}

	#if ASYNC_SYSTEM_BLAST_TOOL_DEBUG >= STM_LEVEL_FINE
	PrintLog (STM_LEVEL_FINE, __FILE__, __LINE__, "About to call FreeSystemTaskData for %s in RunAsyncSystemTaskHook with \"%s\"", uuid_s, task_data_p -> std_command_line_s);
	#endif



	FreeSystemAsyncTask (task_data_p);

	#if ASYNC_SYSTEM_BLAST_TOOL_DEBUG >= STM_LEVEL_FINE
	PrintLog (STM_LEVEL_FINE, __FILE__, __LINE__, "Leaving RunAsyncSystemTaskHook for %s with status %d", uuid_s, status);
	#endif

	return NULL;
}
