/*
 * async_task_manager.h
 *
 *  Created on: 12 May 2017
 *      Author: billy
 */

#ifndef CORE_SERVER_TASK_INCLUDE_ASYNC_TASKS_MANAGER_H_
#define CORE_SERVER_TASK_INCLUDE_ASYNC_TASKS_MANAGER_H_


#include "async_task.h"
#include "async_task_producer.h"

typedef struct AsyncTasksManager
{
	LinkedList *atm_tasks_p;
	struct SyncData *atm_sync_p;
} AsyncTasksManager;



#ifdef __cplusplus
extern "C"
{
#endif


GRASSROOTS_TASK_API AsyncTasksManager *AllocateAsyncTasksManager (void);


GRASSROOTS_TASK_API bool InitialiseAsyncTasksManager (AsyncTasksManager *manager_p);




#ifdef __cplusplus
}
#endif

#endif /* CORE_SERVER_TASK_INCLUDE_ASYNC_TASKS_MANAGER_H_ */
