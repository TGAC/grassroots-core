/*
 * async_task_monitor.h
 *
 *  Created on: 9 May 2017
 *      Author: billy
 */

#ifndef CORE_SERVER_TASK_INCLUDE_ASYNC_TASK_MONITOR_H_
#define CORE_SERVER_TASK_INCLUDE_ASYNC_TASK_MONITOR_H_

struct AsyncTaskMonitor;


/**
 * A datatype to communicate between AsyncTasks.
 */

typedef struct AsyncTaskMonitor
{
	bool (*atr_continue_fn) (struct AsyncTaskResource *resource_p);
	void *atr_data_p;
	void (*atr_free_data_fn) (void *data_p);
} AsyncTaskMonitor;



#ifdef __cplusplus
extern "C"
{
#endif



#ifdef __cplusplus
}
#endif




#endif /* CORE_SERVER_TASK_INCLUDE_ASYNC_TASK_MONITOR_H_ */
