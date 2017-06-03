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


/* forward declaration */
struct AsyncTasksManagerEventConsumer;


typedef struct AsyncTasksManager
{
	LinkedList *atm_tasks_p;
	SyncData *atm_sync_p;
	CountAsyncTask *atm_monitor_p;
	struct AsyncTasksManagerEventConsumer *atm_consumer_p;
} AsyncTasksManager;


typedef struct AsyncTasksManagerEventConsumer
{
	EventConsumer atmec_base_consumer;
	AsyncTasksManager *atmec_tasks_manager_p;
} AsyncTasksManagerEventConsumer;



#ifdef __cplusplus
extern "C"
{
#endif


GRASSROOTS_TASK_API AsyncTasksManager *AllocateAsyncTasksManager (const char * const name_s);


GRASSROOTS_TASK_API bool InitialiseAsyncTasksManager (AsyncTasksManager *manager_p);


GRASSROOTS_TASK_API bool StartAsyncTasksManagerTasks (AsyncTasksManager *manager_p);


GRASSROOTS_TASK_API AsyncTask *GetAsyncTaskFromAsyncTasksManager (AsyncTasksManager *manager_p, const char * const task_name_s);


GRASSROOTS_TASK_API AsyncTasksManagerEventConsumer *AllocateAsyncTasksManagerEventConsumer (void (*consumer_fn) (EventConsumer *consumer_p, struct AsyncTask *task_p), AsyncTasksManager *manager_p);


GRASSROOTS_TASK_API void FreeAsyncTasksManagerEventConsumer (AsyncTasksManagerEventConsumer *consumer_p);


GRASSROOTS_TASK_API void FreeAsyncTasksManagerEventConsumer (AsyncTasksManagerEventConsumer *consumer_p);


#ifdef __cplusplus
}
#endif

#endif /* CORE_SERVER_TASK_INCLUDE_ASYNC_TASKS_MANAGER_H_ */
