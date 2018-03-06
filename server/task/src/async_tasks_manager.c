/*
 * async_tasks_manager.c
 *
 *  Created on: 19 May 2017
 *      Author: billy
 */

#include "async_tasks_manager.h"


static void *RunMonitor (void *data_p);

static void ConsumeFinishedWorkerTask (EventConsumer *consumer_p, struct AsyncTask *task_p);

static void CloseAsyncTasksManager (struct EventConsumer *consumer_p, struct AsyncTask *task_p);


static bool ContinueTask (void *data_p);

static void RunAsyncTaskManagerCleanups (AsyncTasksManager *manager_p);

static CountAsyncTask *AllocateAsyncTasksManagerCountTask (const char *name_s, AsyncTasksManager *manager_p);


#ifdef _DEBUG
	#define ASYNC_TASKS_MANAGER_DEBUG	(STM_LEVEL_FINEST)
#else
	#define ASYNC_TASKS_MANAGER_DEBUG	(STM_LEVEL_NONE)
#endif


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
							CountAsyncTask *monitor_p = NULL;

							manager_p -> atm_tasks_p = tasks_p;
							manager_p -> atm_sync_p = sync_data_p;

							monitor_p = AllocateAsyncTasksManagerCountTask (name_s, manager_p);

							if (monitor_p)
								{
									if (SetAsyncTaskSyncData (monitor_p -> cat_task_p, sync_data_p, MF_SHADOW_USE))
										{
											AsyncTasksManagerEventConsumer *consumer_p = AllocateAsyncTasksManagerEventConsumer (ConsumeFinishedWorkerTask, manager_p);

											if (consumer_p)
												{
													manager_p -> atm_monitor_p = monitor_p;
													manager_p -> atm_consumer_p = consumer_p;

													manager_p -> atm_cleanup_fn = cleanup_fn;
													manager_p -> atm_cleanup_data_p = cleanup_data_p;

													manager_p -> atm_in_use_flag = false;

													return manager_p;
												}
										}

									FreeCountAsyncTask (monitor_p);
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
	FreeCountAsyncTask (manager_p -> atm_monitor_p);
	FreeSyncData (manager_p -> atm_sync_p);
	FreeLinkedList (manager_p -> atm_tasks_p);

	FreeMemory (manager_p);
}

bool AddAsyncTaskToAsyncTasksManager (AsyncTasksManager *manager_p, AsyncTask *task_p, MEM_FLAG mem)
{
	bool success_flag = false;
	AsyncTaskNode *node_p = AllocateAsyncTaskNode (task_p, mem);

	if (node_p)
		{
			LinkedListAddTail (manager_p -> atm_tasks_p, & (node_p -> atn_node));

			task_p -> at_manager_p = manager_p;

			success_flag = true;
		}

	return success_flag;
}

void PrepareAsyncTasksManager (AsyncTasksManager *manager_p, const int32 initial_counter_value)
{
	const uint32 num_tasks = manager_p -> atm_tasks_p -> ll_size;

	if (num_tasks)
		{
			AsyncTaskNode *node_p = (AsyncTaskNode *) (manager_p -> atm_tasks_p -> ll_head_p);
			CountAsyncTask *monitor_task_p = manager_p -> atm_monitor_p;
			EventConsumer *manager_consumer_p =  & (manager_p -> atm_consumer_p -> atmec_base_consumer);

			while (node_p)
				{
					AsyncTask *task_p = node_p -> atn_task_p;

					SetAsyncTaskConsumer (task_p, manager_consumer_p, MF_SHADOW_USE);

					node_p = (AsyncTaskNode *) (node_p -> atn_node.ln_next_p);
				}		/* while (node_p) */


			/*
			 * Set up the monitoring AsyncTask
			 */
			SetCountAsyncTaskLimit (monitor_task_p, initial_counter_value + (manager_p -> atm_tasks_p -> ll_size));
			SetAsyncTaskRunData (monitor_task_p -> cat_task_p, RunMonitor, manager_p -> atm_monitor_p);

			manager_p -> atm_in_use_flag = true;

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
	PrepareAsyncTasksManager (manager_p, 0);

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


static CountAsyncTask *AllocateAsyncTasksManagerCountTask (const char *name_s, AsyncTasksManager *manager_p)
{
	EventConsumer *consumer_p = AllocateEventConsumer (CloseAsyncTasksManager);

	if (consumer_p)
		{
			CountAsyncTask *task_p = AllocateCountAsyncTask (name_s, manager_p, false, 0);

			if (task_p)
				{
					SetAsyncTaskConsumer (task_p -> cat_task_p, consumer_p, MF_SHALLOW_COPY);
					return task_p;
				}

			FreeEventConsumer (consumer_p);
		}

	return NULL;
}


bool IsAsyncTaskManagerRunning (const AsyncTasksManager *manager_p)
{
	return manager_p -> atm_in_use_flag;
}


static void *RunMonitor (void *data_p)
{
	CountAsyncTask *monitor_p = (CountAsyncTask *) data_p;
	AsyncTasksManager *manager_p = monitor_p -> cat_task_p -> at_manager_p;

	WaitOnSyncData (monitor_p -> cat_task_p -> at_sync_data_p, ContinueTask, monitor_p);


	#if ASYNC_TASKS_MANAGER_DEBUG >= STM_LEVEL_FINER
	PrintLog (STM_LEVEL_FINE, __FILE__, __LINE__, "RunMonitor about to call RunAsyncTaskManagerCleanups");
	#endif

	/* To get here, all worker threads have finished so we can delete the tasks manager */
	RunAsyncTaskManagerCleanups (manager_p);

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

	#if ASYNC_TASKS_MANAGER_DEBUG >= STM_LEVEL_FINER
	PrintLog (STM_LEVEL_FINE, __FILE__, __LINE__, "ConsumeFinishedWorkerTask is about to call IncrementCountAsyncTask as %.16X with name \"%s\" has finished", task_p, task_p -> at_name_s);
	#endif

	IncrementAsyncTaskManagerCount (manager_p);
}


void IncrementAsyncTaskManagerCount (AsyncTasksManager *manager_p)
{
	IncrementCountAsyncTask (manager_p -> atm_monitor_p);
}



static void CloseAsyncTasksManager (struct EventConsumer *consumer_p, struct AsyncTask *task_p)
{
	CountAsyncTask *manager_count_task_p = (CountAsyncTask *) task_p;

	#if ASYNC_TASKS_MANAGER_DEBUG >= STM_LEVEL_FINER
	PrintLog (STM_LEVEL_FINE, __FILE__, __LINE__, "CloseAsyncTasksManager about to free  %.16X for \"%s\" at %.16X", task_p -> at_manager_p, task_p -> at_name_s, task_p);
	#endif

	FreeAsyncTasksManager (task_p -> at_manager_p);
}
