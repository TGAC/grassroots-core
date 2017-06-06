/*
 * async_tasks_manager.c
 *
 *  Created on: 19 May 2017
 *      Author: billy
 */

#include "async_tasks_manager.h"


static void *RunMonitor (void *data_p);

static void ConsumeFinishedWorkerTask (EventConsumer *consumer_p, struct AsyncTask *task_p);

static bool ContinueTask (void *data_p);


AsyncTasksManager *AllocateAsyncTasksManager (const char * const name_s)
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
							AsyncTasksManagerCountTask *monitor_p = AllocateAsyncTasksManagerCountTask (name_s, 0, manager_p);

							if (monitor_p)
								{
									AsyncTasksManagerEventConsumer *consumer_p = AllocateAsyncTasksManagerEventConsumer (ConsumeFinishedWorkerTask, manager_p);

									if (consumer_p)
										{
											manager_p -> atm_tasks_p = tasks_p;
											manager_p -> atm_sync_p = sync_data_p;
											manager_p -> atm_monitor_p = monitor_p;
											manager_p -> atm_consumer_p = consumer_p;

											return manager_p;
										}

									FreeAsyncTasksManagerCountTask (monitor_p);
								}

							FreeMemory (manager_p);
						}

					FreeSyncData (sync_data_p);
				}

			FreeLinkedList (tasks_p);
		}

	return NULL;
}



void FreeAsyncTasksManager (AsyncTasksManager *manager_p)
{
	FreeAsyncTasksManagerEventConsumer (manager_p -> atm_consumer_p);
	FreeAsyncTasksManagerCountTask (manager_p -> atm_monitor_p);
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
			if (AddAsyncTaskToAsyncTaskaManager (manager_p, task_p))
				{
					return task_p;
				}

			FreeAsyncTask (task_p);
		}

	return NULL;
}


bool AddAsyncTaskToAsyncTaskaManager (AsyncTasksManager *manager_p, AsyncTask *task_p)
{
	bool success_flag = false;
	AsyncTaskNode *node_p = AllocateAsyncTaskNode (task_p);

	if (node_p)
		{
			LinkedListAddTail (manager_p -> atm_tasks_p, & (node_p -> atn_node));

			success_flag = true;
		}

	return success_flag;
}


SystemAsyncTask *GetSystemAsyncTaskFromAsyncTasksManager (AsyncTasksManager *manager_p, ServiceJob *job_p, char *command_line_s)
{
	SystemAsyncTask *task_p = AllocateSystemAsyncTask (job_p, job_p -> sj_name_s, command_line_s);

	if (task_p)
		{
			if (AddAsyncTaskToAsyncTaskaManager (manager_p, task_p -> std_async_task_p))
				{
					return task_p;
				}

			FreeSystemAsyncTask (task_p);
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
			CountAsyncTask *monitor_task_p = & (manager_p -> atm_monitor_p -> atmct_base_task);

			while (node_p)
				{
					AsyncTask *task_p = node_p -> atn_task_p;

					task_p -> at_consumer_p = & (manager_p -> atm_consumer_p -> atmec_base_consumer);

					node_p = (AsyncTaskNode *) (node_p -> atn_node.ln_next_p);
				}		/* while (node_p) */


			/*
			 * Set up the monitoring AsyncTask
			 */
			SetCountAsyncTaskLimit (monitor_task_p, manager_p -> atm_tasks_p -> ll_size);
			SetAsyncTaskRunData (monitor_task_p -> cat_task_p, RunMonitor, manager_p -> atm_monitor_p);
			RunAsyncTask (monitor_task_p -> cat_task_p);

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


AsyncTasksManagerEventConsumer *AllocateAsyncTasksManagerEventConsumer (void (*consumer_fn) (EventConsumer *consumer_p, struct AsyncTask *task_p), AsyncTasksManager *manager_p)
{
	AsyncTasksManagerEventConsumer *consumer_p = (AsyncTasksManagerEventConsumer *) AllocMemory (sizeof (AsyncTasksManagerEventConsumer));

	if (consumer_p)
		{
			if (InitEventConsumer (& (consumer_p -> atmec_base_consumer), consumer_fn))
				{
					consumer_p -> atmec_tasks_manager_p = manager_p;

					return consumer_p;
				}

			FreeMemory (consumer_p);
		}

	return NULL;
}


void FreeAsyncTasksManagerEventConsumer (AsyncTasksManagerEventConsumer *consumer_p)
{
	ClearEventConsumer (& (consumer_p -> atmec_base_consumer));
	FreeMemory (consumer_p);
}


AsyncTasksManagerCountTask *AllocateAsyncTasksManagerCountTask (const char *name_s, int32 limit, AsyncTasksManager *manager_p)
{
	AsyncTasksManagerCountTask *task_p = (AsyncTasksManagerCountTask *) AllocMemory (sizeof (AsyncTasksManagerCountTask));

	if (task_p)
		{
			if (InitCountAsyncTask (& (task_p -> atmct_base_task), name_s, limit))
				{
					task_p -> atmec_tasks_manager_p = manager_p;

					return task_p;
				}

			FreeMemory (task_p);
		}

	return NULL;
}


void FreeAsyncTasksManagerCountTask (AsyncTasksManagerCountTask *task_p)
{
	ClearCountAsyncTask (& (task_p -> atmct_base_task));
	FreeMemory (task_p);
}



static void *RunMonitor (void *data_p)
{
	AsyncTasksManagerCountTask *monitor_p = (AsyncTasksManagerCountTask *) data_p;

	WaitOnSyncData (monitor_p -> atmct_base_task.cat_task_p -> at_sync_data_p, ContinueTask, monitor_p);

	/* To get here, all worker threads have finished so we can delete the tasks manager */
	FreeAsyncTasksManager (monitor_p -> atmec_tasks_manager_p);

	return NULL;
}



static bool ContinueTask (void *data_p)
{
	CountAsyncTask *task_p = (CountAsyncTask *) data_p;

	return ContinueCountAsyncTask (task_p);
}


static void ConsumeFinishedWorkerTask (EventConsumer *consumer_p, struct AsyncTask *task_p)
{
	AsyncTasksManagerEventConsumer *manager_consumer_p = (AsyncTasksManagerEventConsumer *) consumer_p;
	AsyncTasksManager *manager_p = manager_consumer_p -> atmec_tasks_manager_p;

	IncrementCountAsyncTask (& (manager_p -> atm_monitor_p -> atmct_base_task));
}
