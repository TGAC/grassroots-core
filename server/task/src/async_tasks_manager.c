/*
 * async_tasks_manager.c
 *
 *  Created on: 19 May 2017
 *      Author: billy
 */

#include "async_tasks_manager.h"


static void *RunMonitor (void *data_p);


AsyncTasksManager *AllocateAsyncTasksManager (const char * const name_s)
{
	LinkedList *tasks_p = AllocateLinkedList (FreeAsyncTaskNode);

	if (tasks_p)
		{
			SyncData *sync_data_p = AllocateSyncData ();

			if (sync_data_p)
				{
					CountAsyncTask *monitor_p = AllocateCountAsyncTask (name_s, 0);

					if (monitor_p)
						{
							AsyncTasksManager *manager_p = AllocMemory (sizeof (AsyncTasksManager));

							if (manager_p)
								{
									manager_p -> atm_tasks_p = tasks_p;
									manager_p -> atm_sync_p = sync_data_p;
									manager_p -> atm_monitor_p = monitor_p;

									manager_p -> atm_consumer_p = NULL;

									return manager_p;
								}

							FreeCountAsyncTask (monitor_p);
						}

					FreeSyncData (sync_data_p);
				}

			FreeLinkedList (tasks_p);
		}

	return NULL;
}



void FreeAsyncTasksManager (AsyncTasksManager *manager_p)
{
	FreeCountAsyncTask (manager_p -> atm_monitor_p);
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

			while (node_p)
				{
					AsyncTask *task_p = node_p -> atn_task_p;

					task_p -> at_consumer_p = manager_p -> atm_consumer_p;

					node_p = (AsyncTaskNode *) (node_p -> atn_node.ln_next_p);
				}		/* while (node_p) */


			/*
			 * Set up the monitoring AsyncTask
			 */
			SetCountAsyncTaskLimit (manager_p -> atm_monitor_p, manager_p -> atm_tasks_p -> ll_size);
			SetAsyncTaskRunData (manager_p -> atm_monitor_p -> cat_task_p, RunMonitor, manager_p -> atm_monitor_p);
			RunAsyncTask (manager_p -> atm_monitor_p -> cat_task_p);

			/*
			 * Now run each of the worker threads
			 */
			while (node_p)
				{
					RunAsyncTask (node_p -> atn_task_p);
					node_p = (AsyncTaskNode *) (node_p -> atn_node.ln_next_p);
				}		/* while (node_p) */

		}		/* if (num_tasks) */


	return success_flag;
}


static void *RunMonitor (void *data_p)
{
	CountAsyncTask *monitor_p = (CountAsyncTask *) monitor_p;

	WaitOnSyncData (monitor_p -> cat_task_p -> at_sync_data_p, monitor_p -> cat_continue_fn, monitor_p);

	return NULL;
}



static bool ContinueTask (void *data_p)
{
	CountAsyncTask *task_p = (CountAsyncTask *) data_p;

	return ContinueCountAsyncTask (task_p);
}
