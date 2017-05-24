/*
 * async_task_manager.h
 *
 *  Created on: 12 May 2017
 *      Author: billy
 */

#ifndef CORE_SERVER_TASK_INCLUDE_ASYNC_TASKS_MANAGER_H_
#define CORE_SERVER_TASK_INCLUDE_ASYNC_TASKS_MANAGER_H_


#include "async_task.h"
#include "count_async_task.h"


typedef struct AsyncTasksManager
{
	LinkedList *atm_tasks_p;
	SyncData *atm_sync_p;
	CountAsyncTask *atm_monitor_p;
	EventConsumer *atm_consumer_p;
} AsyncTasksManager;



#ifdef __cplusplus
extern "C"
{
#endif


GRASSROOTS_TASK_API AsyncTasksManager *AllocateAsyncTasksManager (const char * const name_s);


GRASSROOTS_TASK_API bool InitialiseAsyncTasksManager (AsyncTasksManager *manager_p);


GRASSROOTS_TASK_API bool StartAsyncTasksManagerTasks (AsyncTasksManager *manager_p);


GRASSROOTS_TASK_API AsyncTask *GetAsyncTaskFromAsyncTasksManager (AsyncTasksManager *manager_p, const char * const task_name_s);





#ifdef __cplusplus
}
#endif

#endif /* CORE_SERVER_TASK_INCLUDE_ASYNC_TASKS_MANAGER_H_ */
