/*
 * system_async_task.c
 *
 *  Created on: 3 May 2017
 *      Author: billy
 */

#include "system_async_task.h"
#include "async_tasks_manager.h"
#include "memory_allocations.h"
#include "string_utils.h"
#include "uuid_util.h"


#ifdef _DEBUG
	#define ASYNC_SYSTEM_BLAST_TOOL_DEBUG (STM_LEVEL_FINEST)
#else
	#define ASYNC_SYSTEM_BLAST_TOOL_DEBUG (STM_LEVEL_NONE)
#endif


static void *RunAsyncSystemTaskHook (void *data_p);


SystemAsyncTask *AllocateSystemAsyncTask (ServiceJob *job_p, const char *name_s, AsyncTasksManager *manager_p, bool add_flag, const char *command_s, void (*on_success_callback_fn) (ServiceJob *job_p))
{
	AsyncTask *async_task_p = AllocateAsyncTask (name_s, manager_p, add_flag);

	if (async_task_p)
		{
			SystemAsyncTask *system_task_p = (SystemAsyncTask *) AllocMemory (sizeof (SystemAsyncTask));

			if (system_task_p)
				{
					system_task_p -> std_command_line_s = NULL;

					if (SetSystemAsyncTaskCommand (system_task_p, command_s))
						{
							system_task_p -> std_async_task_p = async_task_p;
							system_task_p -> std_service_job_p = job_p;
							system_task_p -> std_on_success_callback_fn = on_success_callback_fn;


							if (manager_p && add_flag)
								{
									system_task_p -> std_async_task_mem = MF_SHADOW_USE;
								}
							else
								{
									system_task_p -> std_async_task_mem = MF_SHALLOW_COPY;
								}

							#if ASYNC_SYSTEM_BLAST_TOOL_DEBUG >= STM_LEVEL_FINE
							PrintLog (STM_LEVEL_FINE, __FILE__, __LINE__, "AllocateSystemAsyncTask at 0x%16x with task at 0x%16x for %s", system_task_p, system_task_p -> std_async_task_p, name_s);
							#endif


							return system_task_p;
						}


					FreeMemory (system_task_p);
				}

			FreeAsyncTask (async_task_p);
		}

	return NULL;
}



bool SetSystemAsyncTaskCommand (SystemAsyncTask *task_p, const char *command_s)
{
	bool success_flag = true;

	if (command_s)
		{
			char *copied_command_s = EasyCopyToNewString (command_s);

			if (copied_command_s)
				{
					if (task_p -> std_command_line_s)
						{
							FreeCopiedString (task_p -> std_command_line_s);
						}

					task_p -> std_command_line_s = copied_command_s;
				}
			else
				{
					success_flag = false;
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "SetSystemAsyncTaskCommand failed to set command \"%s\"", command_s);
				}
		}
	else
		{
			if (task_p -> std_command_line_s)
				{
					FreeCopiedString (task_p -> std_command_line_s);
					task_p -> std_command_line_s = NULL;
				}
		}

	return success_flag;
}


void FreeSystemAsyncTask (SystemAsyncTask *system_task_p)
{
	#if ASYNC_SYSTEM_BLAST_TOOL_DEBUG >= STM_LEVEL_FINE
	PrintLog (STM_LEVEL_FINE, __FILE__, __LINE__, "FreeSystemAsyncTask at 0x%16x with task at 0x%16x with name %s", system_task_p, system_task_p -> std_async_task_p, system_task_p -> std_async_task_p -> at_name_s);
	#endif

	/*
	 * If the underlying task is not within an AsyncTasksManager,
	 * then we'll delete it. Otherwise the AsyncTasksManager has
	 * the responsibility for deleting it.
	 */
	switch (system_task_p -> std_async_task_mem)
		{
			case MF_DEEP_COPY:
			case MF_SHALLOW_COPY:
				FreeAsyncTask (system_task_p -> std_async_task_p);
				break;

			default:
				break;
		}


	if (system_task_p -> std_command_line_s)
		{
			FreeCopiedString (system_task_p -> std_command_line_s);
		}


	FreeMemory (system_task_p);
}


bool RunSystemAsyncTask (SystemAsyncTask *task_p)
{
	SetAsyncTaskRunData (task_p -> std_async_task_p, RunAsyncSystemTaskHook, task_p);

	return RunAsyncTask (task_p -> std_async_task_p);
}


static void *RunAsyncSystemTaskHook (void *data_p)
{
	SystemAsyncTask *task_p = ((SystemAsyncTask *) data_p);
	ServiceJob *job_p = task_p -> std_service_job_p;

	/* Windows complains that it can't find the function as it's a cicular dependency*/
	GrassrootsServer *grassroots_p = job_p -> sj_service_p -> se_plugin_p -> pl_server_p; // GetGrassrootsServerFromService (job_p -> sj_service_p);
	JobsManager *jobs_manager_p = GetJobsManager (grassroots_p);
	OperationStatus status = OS_STARTED;
	char uuid_s [UUID_STRING_BUFFER_SIZE];

	ConvertUUIDToString (job_p -> sj_id, uuid_s);

	/* Set the job to having started */
  /* Windows complains that it can't find the function as it's a cicular
    * dependency*/
	// SetServiceJobStatus (job_p, status);
  job_p -> sj_status = status;


	if (AddServiceJobToJobsManager (jobs_manager_p, job_p -> sj_id, job_p))
		{
			status = ActualRunSystemAsyncTask (task_p);
		}
	else
		{
			status = OS_FAILED_TO_START;
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add job %s with status %d to jobs manager", uuid_s, status);
		}

	/* has the job status changed? */
	if (status != OS_STARTED)
		{
      /* Windows complains that it can't find the function as
        * it's a cicular dependency*/
      // SetServiceJobStatus (job_p, status);
      job_p -> sj_status = status;

			/* If the job ran successfully, run any specified callbacks to update the results */
			if ((status == OS_SUCCEEDED) || (status == OS_PARTIALLY_SUCCEEDED))
				{
					if (task_p -> std_on_success_callback_fn)
						{
							task_p -> std_on_success_callback_fn (job_p);
						}
				}

			if (! (AddServiceJobToJobsManager (jobs_manager_p, job_p -> sj_id, job_p)))
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add job %s with status %d to jobs manager", uuid_s, status);
				}

/*
			if (task_p -> std_async_task_p -> at_consumer_p)
				{
					#if ASYNC_SYSTEM_BLAST_TOOL_DEBUG >= STM_LEVEL_FINE
					PrintLog (STM_LEVEL_FINE, __FILE__, __LINE__, "Sending message to EventConsumer for %s", uuid_s);
					#endif

					RunEventConsumerFromAsyncTask (task_p -> std_async_task_p);
					//SendSyncData (task_data_p -> std_async_task_p -> at_sync_data_p);
				}
*/
		}

	#if ASYNC_SYSTEM_BLAST_TOOL_DEBUG >= STM_LEVEL_FINE
	PrintLog (STM_LEVEL_FINE, __FILE__, __LINE__, "Leaving RunAsyncSystemTaskHook for %s with status %d", uuid_s, status);
	#endif

	return NULL;
}


void RunSystemAsyncTaskSuccess (SystemAsyncTask *task_p, ServiceJob *job_p)
{
	if (task_p -> std_on_success_callback_fn)
		{
			task_p -> std_on_success_callback_fn (job_p);
		}
}
