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

#include <windows.h>

#include "streams.h"

#include "async_task.h"
#include "async_tasks_manager.h"
#include "memory_allocations.h"
#include "string_utils.h"




typedef struct WindowsAsyncTask
{
	AsyncTask wat_base_task;
	HANDLE wat_thread_handle;
	DWORD wat_thread_id;
	bool wat_valid_thread_flag;
} WindowsAsyncTask;



static DWORD WINAPI DoAsyncTaskRun (LPVOID data_p);


#ifdef _DEBUG
	#define WINDOWS_ASYNC_TASK_DEBUG	(STM_LEVEL_FINEST)
#else
	#define WINDOWS_ASYNC_TASK_DEBUG	(STM_LEVEL_NONE)
#endif



AsyncTask *AllocateAsyncTask (const char *name_s, AsyncTasksManager *manager_p, bool add_flag)
{
	WindowsAsyncTask *task_p = (WindowsAsyncTask*) AllocMemory (sizeof (struct WindowsAsyncTask));

	if (task_p)
		{
			memset (task_p, 0, sizeof (WindowsAsyncTask));

			if (InitialiseAsyncTask (& (task_p -> wat_base_task), name_s, manager_p, add_flag))
				{
					task_p -> wat_thread_handle = NULL;
					task_p -> wat_thread_id = 0;

					/* For portability, explicitly create threads in a joinable state */


					#if WINDOWS_ASYNC_TASK_DEBUG >= STM_LEVEL_FINEST
					PrintLog (STM_LEVEL_FINE, __FILE__, __LINE__, "AllocateAsyncTask for \"%s\" at %.16X", task_p -> uat_base_task.at_name_s, task_p);
					#endif

					return (& (task_p -> wat_base_task));
				}

			FreeMemory (task_p);
		}

	return NULL;
}


void FreeAsyncTask (AsyncTask *task_p)
{
	#if WINDOWS_ASYNC_TASK_DEBUG >= STM_LEVEL_FINEST
	PrintLog (STM_LEVEL_FINE, __FILE__, __LINE__, "FreeAsyncTask for \"%s\" at %.16X", task_p -> at_name_s, task_p);
	#endif

	CloseAsyncTask (task_p);

	ClearAsyncTask (task_p);

	FreeMemory (task_p);
}


void CloseAsyncTask (AsyncTask *task_p)
{
	WindowsAsyncTask *win_task_p = (WindowsAsyncTask *) task_p;

	if (win_task_p-> wat_valid_thread_flag)
		{
			win_task_p-> wat_valid_thread_flag = false;
		}
		
		ExitThread (0);
	}


bool SetAsyncTaskSyncData (AsyncTask *task_p, SyncData *sync_data_p, MEM_FLAG mem)
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

	return true;
}


bool IsAsyncTaskRunning (const AsyncTask *task_p)
{
	WindowsAsyncTask *win_task_p = (WindowsAsyncTask *) task_p;

	return (win_task_p -> wat_valid_thread_flag);
}


bool CloseAllAsyncTasks (void)
{
	
	return true;
}


bool RunAsyncTask (AsyncTask *task_p)
{
	bool success_flag = true;
	WindowsAsyncTask *win_task_p = (WindowsAsyncTask *) task_p;

	win_task_p -> wat_thread_handle = CreateThread (
		NULL,																// default security attributes
		0,																	// use default stack size  
		DoAsyncTaskRun,											// thread function name
		task_p -> at_data_p,								// argument to thread function 
		0,																	// use default creation flags 
		& (win_task_p -> wat_thread_id)			// returns the thread identifier 
	);

	 
	if (win_task_p -> wat_thread_handle)
		{
			win_task_p -> wat_valid_thread_flag = true;
		}
	else
		{
			win_task_p -> wat_valid_thread_flag = false;
			success_flag = false;
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create task for RunAsyncTask %d", GetLastError ());
		}

	return success_flag;
}


OperationStatus RunProcess (const char * const command_line_s)
{
	OperationStatus status = OS_STARTED;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory (&si, sizeof (si));
	si.cb = sizeof (si);
	ZeroMemory (&pi, sizeof (pi));

#if ASYNC_SYSTEM_BLAST_TOOL_DEBUG >= STM_LEVEL_FINE
	PrintLog (STM_LEVEL_FINE, __FILE__, __LINE__, "About to run RunAsyncSystemTaskHook for %s with \"%s\"", uuid_s, command_line_s);
#endif

	// Start the child process.
	if (CreateProcess (
		NULL,   // No module name (use command line)
		command_line_s,        // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory
		&si,            // Pointer to STARTUPINFO structure
		&pi)           // Pointer to PROCESS_INFORMATION structure
		)
		{
		DWORD process_exit_code;

		// Wait until child process exits.
		WaitForSingleObject (pi.hProcess, INFINITE);

		if (GetExitCodeProcess (pi.hProcess, &process_exit_code))
			{
			if (process_exit_code == 0)
				{
				status = OS_SUCCEEDED;
				PrintLog (STM_LEVEL_FINE, __FILE__, __LINE__, "\"%s\" ran successfully", command_line_s);
				}
			else
				{
				status = OS_ERROR;
				PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "\"%s\" failed with return code %d", command_line_s, process_exit_code);
				}
			}
		else
			{
			status = OS_ERROR;
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "\"%s\" failed to get process exit code %d", command_line_s, GetLastError ());

			}
		}
	else
		{
		status = OS_ERROR;
		PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "CreateProcess () failed running \"%s\" with return code %d", command_line_s, GetLastError ());
		}

	return status;

}


OperationStatus ActualRunSystemAsyncTask (SystemAsyncTask *task_p)
{
	return RunProcess (task_p -> std_command_line_s);
}

static DWORD WINAPI DoAsyncTaskRun (LPVOID data_p)
{
	WindowsAsyncTask *win_task_p = (WindowsAsyncTask *) data_p;
	AsyncTask *base_task_p = & (win_task_p -> wat_base_task);
	void *res_p = NULL;

	#if WINDOWS_ASYNC_TASK_DEBUG >= STM_LEVEL_FINEST
	PrintLog (STM_LEVEL_FINE, __FILE__, __LINE__, "DoAsyncTaskRun about to run for \"%s\" at %.16X", async_task_p -> at_name_s, async_task_p);
	#endif

	res_p = base_task_p-> at_run_fn (base_task_p -> at_data_p);

	#if WINDOWS_ASYNC_TASK_DEBUG >= STM_LEVEL_FINEST
	PrintLog (STM_LEVEL_FINE, __FILE__, __LINE__, "DoAsyncTaskRun ran for \"%s\" at %.16X", async_task_p -> at_name_s, async_task_p);
	#endif

	if (base_task_p-> at_consumer_p)
		{
			#if WINDOWS_ASYNC_TASK_DEBUG >= STM_LEVEL_FINEST
			PrintLog (STM_LEVEL_FINE, __FILE__, __LINE__, "Sending message to EventConsumer as task \"%s\" has completed", async_task_p -> at_name_s);
			#endif

			RunEventConsumer (base_task_p -> at_consumer_p, base_task_p);
		}
		
	ExitThread (0);
}
