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

static void RunAsyncTaskManagerCleanups (AsyncTasksManager *manager_p);



AsyncTasksManager *AllocateAsyncTasksManager (const char * const name_s, bool (*cleanup_fn) (void *data_p), void *cleanup_data_p)
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
									if (SetAsyncTaskSyncData (monitor_p -> atmct_base_task.cat_task_p, sync_data_p, MF_SHADOW_USE))
										{
											AsyncTasksManagerEventConsumer *consumer_p = AllocateAsyncTasksManagerEventConsumer (ConsumeFinishedWorkerTask, manager_p);

											if (consumer_p)
												{
													SetAsyncTaskSyncData (monitor_p -> atmct_base_task.cat_task_p, sync_data_p, MF_SHADOW_USE);

													manager_p -> atm_tasks_p = tasks_p;
													manager_p -> atm_sync_p = sync_data_p;
													manager_p -> atm_monitor_p = monitor_p;
													manager_p -> atm_consumer_p = consumer_p;

													manager_p -> atm_cleanup_fn = cleanup_fn;
													manager_p -> atm_cleanup_data_p = cleanup_data_p;

													return manager_p;
												}
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


AsyncTask *GetAsyncTaskFromAsyncTasksManager (AsyncTasksManager *manager_p, const char * const task_name_s)
{
	AsyncTask *task_p = AllocateAsyncTask (task_name_s);

	if (task_p)
		{
			if (AddAsyncTaskToAsyncTasksManager (manager_p, task_p, MF_SHALLOW_COPY))
				{
					return task_p;
				}

			FreeAsyncTask (task_p);
		}

	return NULL;
}


bool AddAsyncTaskToAsyncTasksManager (AsyncTasksManager *manager_p, AsyncTask *task_p, MEM_FLAG mem)
{
	bool success_flag = false;
	AsyncTaskNode *node_p = AllocateAsyncTaskNode (task_p, mem);

	if (node_p)
		{
			LinkedListAddTail (manager_p -> atm_tasks_p, & (node_p -> atn_node));

			success_flag = true;
		}

	return success_flag;
}


SystemAsyncTask *AllocateSystemAsyncTaskFromAsyncTasksManager (AsyncTasksManager *manager_p, ServiceJob *job_p, char *command_line_s, void (*on_success_callback_fn) (ServiceJob *job_p))
{
	SystemAsyncTask *task_p = AllocateSystemAsyncTask (job_p, job_p -> sj_name_s, command_line_s, on_success_callback_fn);

	if (task_p)
		{
			if (AddAsyncTaskToAsyncTasksManager (manager_p, task_p -> std_async_task_p, MF_SHALLOW_COPY))
				{
					return task_p;
				}

			FreeSystemAsyncTask (task_p);
		}

	return NULL;
}


void PrepareAsyncTasksManager (AsyncTasksManager *manager_p)
{
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
		}		/* if (num_tasks) */
}


bool StartAsyncTaskManagerWorkers (AsyncTasksManager *manager_p)
{
	const uint32 num_tasks = manager_p -> atm_tasks_p -> ll_size;
	bool success_flag = true;

	if (num_tasks)
		{
			AsyncTaskNode *node_p = (AsyncTaskNode *) (manager_p -> atm_tasks_p -> ll_head_p);

			/*
			 * Now run each of the worker threads
			 */
			while (node_p && success_flag)
				{
					if (RunAsyncTask (node_p -> atn_task_p))
						{
							node_p = (AsyncTaskNode *) (node_p -> atn_node.ln_next_p);
						}
					else
						{
							success_flag = false;
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to run async task for \"%s\"", node_p -> atn_task_p -> at_name_s);
						}
				}		/* while (node_p) */

		}		/* if (num_tasks) */

	return success_flag;
}


bool RunAsyncTasksManagerTasks (AsyncTasksManager *manager_p)
{
	PrepareAsyncTasksManager (manager_p);

	return StartAsyncTaskManagerWorkers (manager_p);
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
	AsyncTasksManager *manager_p = monitor_p -> atmec_tasks_manager_p;

	WaitOnSyncData (monitor_p -> atmct_base_task.cat_task_p -> at_sync_data_p, ContinueTask, monitor_p);

	/* To get here, all worker threads have finished so we can delete the tasks manager */
	RunAsyncTaskManagerCleanups (manager_p);

	//FreeAsyncTasksManager (manager_p);

	return NULL;
}


static void RunAsyncTaskManagerCleanups (AsyncTasksManager *manager_p)
{
	if (manager_p -> atm_cleanup_fn)
		{
			manager_p -> atm_cleanup_fn (manager_p -> atm_cleanup_data_p);
		}
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
