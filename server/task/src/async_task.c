/*
 * async_task.c
 *
 *  Created on: 12 May 2017
 *      Author: billy
 */


#include "async_task.h"

#include "string_utils.h"
#include "streams.h"
#include "memory_allocations.h"


bool InitialiseAsyncTask (AsyncTask *task_p, const char *name_s)
{
	bool success_flag = true;
	char *copied_name_s = NULL;

	if (name_s)
		{
			copied_name_s = CopyToNewString (name_s, 0, false);

			if (!copied_name_s)
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to copy AsyncTask name \"%s\"", name_s);
					success_flag = false;
				}
		}

	if (success_flag)
		{
			ClearAsyncTask (task_p);

			task_p -> at_name_s = copied_name_s;
		}


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
					task_p -> at_sync_data_mem = MF_ALREADY_FREED;
				}

			task_p -> at_sync_data_p = NULL;
		}
}


void SetAsyncTaskRunData (AsyncTask *task_p, void *(*run_fn) (void *data_p), void *data_p)
{
	task_p -> at_run_fn = run_fn;
	task_p -> at_data_p = data_p;
}



AsyncTaskNode *AllocateAsyncTaskNode (AsyncTask *task_p)
{
	AsyncTaskNode *node_p = (AsyncTaskNode *) AllocMemory (sizeof (AsyncTaskNode));

	if (node_p)
		{
			node_p -> atn_task_p = task_p;

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

	FreeAsyncTask (at_node_p -> atn_task_p);
	FreeMemory (at_node_p);
}

