/*
 * async_tasks_manager.c
 *
 *  Created on: 19 May 2017
 *      Author: billy
 */

#include "async_tasks_manager.h"



AsyncTasksManager *AllocateAsyncTasksManager (void)
{
	LinkedList *tasks_p = AllocateLinkedList (FreeAsyncTaskNode);

	if (tasks_p)
		{
			SyncData *sync_data_p = AllocateSyncData ();

			if (sync_data_p)
				{
					AsyncTasksManager *manager_p = AllocMemory (sizeof (AsyncTasksManager));

					if (manager_p)
						{
							manager_p -> atm_tasks_p = tasks_p;
							manager_p -> atm_sync_p = sync_data_p;

							return manager_p;
						}

					FreeSyncData (sync_data_p);
				}

			FreeLinkedList (tasks_p);
		}

	return NULL;
}



void FreeAsyncTasksManager (AsyncTasksManager *manager_p)
{
	FreeSyncData (manager_p -> atm_sync_p);
	FreeLinkedList (manager_p -> atm_tasks_p);

	FreeMemory (manager_p);
}


bool InitialiseAsyncTasksManager (AsyncTasksManager *manager_p)
{
	bool success_flag = false;
	LinkedList *tasks_p = AllocateLinkedList (FreeAsyncTaskNode);

	if (tasks_p)
		{
			manager_p -> atm_tasks_p = tasks_p;
			success_flag = true;
		}

	return success_flag;
}



AsyncTask *GetAsyncTaskFromAsyncTasksManager (AsyncTasksManager *manager_p, const char * const task_name_s)
{
	AsyncTask *task_p = AllocateAsyncTask (task_name_s);

	if (task_p)
		{
			AsyncTaskNode *node_p = AllocateAsyncTaskNode (task_p);

			if (node_p)
				{
					LinkedListAddTail (manager_p -> atm_tasks_p, & (node_p -> atn_node));

					return task_p;
				}

			FreeAsyncTask (task_p);
		}

	return NULL;
}


bool StartAsyncTasksManagerTasks (AsyncTasksManager *manager_p)
{
	bool success_flag = false;
	const uint32 num_tasks = manager_p -> atm_tasks_p -> ll_size;

	if (num_tasks)
		{
			AsyncTaskNode *node_p = (AsyncTaskNode *) (manager_p -> atm_tasks_p -> ll_head_p);

			SetCountAsyncTaskLimit (manager_p -> atm_monitor_p, manager_p -> atm_tasks_p -> ll_size);

			while (node_p)
				{
					RunAsyncTask (node_p -> atn_task_p);
					node_p = (AsyncTaskNode *) (node_p -> atn_node.ln_next_p);
				}		/* while (node_p) */

		}		/* if (num_tasks) */


	return success_flag;
}


