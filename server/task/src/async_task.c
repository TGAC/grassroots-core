/*
 * async_task.c
 *
 *  Created on: 12 May 2017
 *      Author: billy
 */

#include <string.h>

#include "async_task.h"

#include "string_utils.h"
#include "streams.h"
#include "memory_allocations.h"
#include "async_tasks_manager.h"


#ifdef _DEBUG
	#define ASYNC_TASK_DEBUG	(STM_LEVEL_INFO)
#else
	#define ASYNC_TASK_DEBUG	(STM_LEVEL_NONE)
#endif


bool InitialiseAsyncTask (AsyncTask *task_p, const char *name_s, AsyncTasksManager *manager_p, bool add_flag)
{
	bool success_flag = true;
	char *copied_name_s = NULL;

	if (name_s)
		{
			copied_name_s = EasyCopyToNewString (name_s);

			if (!copied_name_s)
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to copy AsyncTask name \"%s\"", name_s);
					success_flag = false;
				}
		}

	if (success_flag)
		{
			memset (task_p, 0, sizeof (AsyncTask));

			task_p -> at_name_s = copied_name_s;

			task_p -> at_sync_data_p = NULL;
			task_p -> at_sync_data_mem = MF_ALREADY_FREED;

			task_p -> at_consumer_p = NULL;
			task_p -> at_consumer_mem = MF_ALREADY_FREED;

			task_p -> at_data_p = NULL;
			task_p -> at_run_fn = NULL;
			task_p -> at_manager_p = manager_p;

			if (add_flag)
				{
					if (!AddAsyncTaskToAsyncTasksManager (manager_p, task_p, MF_SHALLOW_COPY))
						{
							success_flag = false;
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add AsyncTask name \"%s\" to AsyncTasksManager", name_s);
						}
				}
		}

	#if ASYNC_TASK_DEBUG >= STM_LEVEL_FINEST
	PrintLog (STM_LEVEL_FINE, __FILE__, __LINE__, "InitialiseAsyncTask for \"%s\" at %.16X is %d", task_p -> at_name_s, task_p, success_flag);
	#endif

	return success_flag;
}


void ClearAsyncTask (AsyncTask *task_p)
{
	if (task_p -> at_name_s)
		{
			FreeCopiedString (task_p -> at_name_s);
			task_p -> at_name_s = NULL;
		}

	if (task_p -> at_sync_data_p)
		{
			if ((task_p -> at_sync_data_mem == MF_DEEP_COPY) || ((task_p -> at_sync_data_mem == MF_SHALLOW_COPY)))
				{
					FreeSyncData (task_p -> at_sync_data_p);
				}

			task_p -> at_sync_data_p = NULL;
			task_p -> at_sync_data_mem = MF_ALREADY_FREED;
		}

	task_p -> at_data_p = NULL;
	task_p -> at_run_fn = NULL;


	if (task_p -> at_consumer_p)
		{
			switch (task_p -> at_consumer_mem)
				{
					case MF_DEEP_COPY:
					case MF_SHALLOW_COPY:
						FreeEventConsumer (task_p -> at_consumer_p);
						break;

					default:
						break;
				}

			task_p -> at_consumer_p = NULL;
		}

}


void SetAsyncTaskRunData (AsyncTask *task_p, void *(*run_fn) (void *data_p), void *data_p)
{
	task_p -> at_run_fn = run_fn;
	task_p -> at_data_p = data_p;
}


void SetAsyncTaskConsumer (AsyncTask *task_p, EventConsumer *consumer_p, MEM_FLAG mem)
{
	task_p -> at_consumer_p = consumer_p;
	task_p -> at_consumer_mem = mem;
}



AsyncTaskNode *AllocateAsyncTaskNode (AsyncTask *task_p, MEM_FLAG mem)
{
	AsyncTaskNode *node_p = (AsyncTaskNode *) AllocMemory (sizeof (AsyncTaskNode));

	if (node_p)
		{
			if (mem == MF_DEEP_COPY)
				{
					AsyncTask *node_task_p = NULL;


				}
			else
				{
					node_p -> atn_task_p = task_p;
				}

			node_p -> atn_mem = mem;

			node_p -> atn_node.ln_prev_p = NULL;
			node_p -> atn_node.ln_next_p = NULL;
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create AsyncTaskNode for \"%s\"", task_p -> at_name_s ? task_p -> at_name_s : "");
		}

	return node_p;
}


void FreeAsyncTaskNode (ListItem *node_p)
{
	AsyncTaskNode *at_node_p = (AsyncTaskNode *) node_p;

	switch (at_node_p -> atn_mem)
		{
			case MF_DEEP_COPY:
			case MF_SHALLOW_COPY:
				FreeAsyncTask (at_node_p -> atn_task_p);
				break;

			default:
				break;
		}

	FreeMemory (at_node_p);
}


void RunEventConsumerFromAsyncTask (AsyncTask *task_p)
{
	RunEventConsumer (task_p -> at_consumer_p, task_p);
}

