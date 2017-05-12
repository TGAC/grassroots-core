/*
 * async_task_manager.h
 *
 *  Created on: 12 May 2017
 *      Author: billy
 */

#ifndef CORE_SERVER_TASK_INCLUDE_ASYNC_TASK_MANAGER_H_
#define CORE_SERVER_TASK_INCLUDE_ASYNC_TASK_MANAGER_H_


#include "async_task.h"
#include "async_task_consumer.h"


typedef struct AsyncTaskManager
{
	AsyncTaskMonitor *atm_monitor_p;
	AsyncTask **atm_tasks_pp;
	uint32 atm_num_tasks;
} AsyncTaskManager;


#endif /* CORE_SERVER_TASK_INCLUDE_ASYNC_TASK_MANAGER_H_ */
