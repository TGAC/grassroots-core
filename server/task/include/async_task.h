/*
 * async_task.h
 *
 *  Created on: 3 May 2017
 *      Author: billy
 */

#ifndef CORE_SERVER_TASK_INCLUDE_ASYNC_TASK_H_
#define CORE_SERVER_TASK_INCLUDE_ASYNC_TASK_H_

#include "grassroots_task_library.h"
#include "typedefs.h"
#include "linked_list.h"
#include "sync_data.h"
#include "memory_allocations.h"
#include "event_consumer.h"


/**
 * A datatype to used to run tasks asynchronously.
 */

typedef struct AsyncTask
{
	char *at_name_s;
	struct SyncData *at_sync_data_p;
	MEM_FLAG at_sync_data_mem;
	void *(*at_run_fn) (void *data_p);
	void *at_data_p;
	EventConsumer *at_consumer_p;
} AsyncTask;



typedef struct AsyncTaskNode
{
	ListItem atn_node;
	AsyncTask *atn_task_p;
	MEM_FLAG atn_mem;
} AsyncTaskNode;


#ifdef __cplusplus
extern "C"
{
#endif


/**
 * Create an AsyncTask.
 *
 * @return The new AsyncTask or <code>NULL</code> upon error.
 * @memberof AsyncTask
 */
GRASSROOTS_TASK_API	AsyncTask *AllocateAsyncTask (const char *name_s);


/**
 * Free an AsyncTask. If it is currently running, it will be stopped
 * first by calling CloseAsyncTask ().
 *
 * @param task_p The AsyncTask to free.
 * @memberof AsyncTask
 */
GRASSROOTS_TASK_API	void FreeAsyncTask (AsyncTask *task_p);


/**
 * Initialise an AsyncTask.
 *
 * @param The AsyncTask to initialise.
 * @param The name to give the AsyncTask.
 * @return <code>true</code> if the AsyncTask was started
 * successfully, <code>false</code> otherwise.
 * @memberof AsyncTask
 */
GRASSROOTS_TASK_API	bool InitialiseAsyncTask (AsyncTask *task_p, const char *name_s);


/**
 * Clear an AsyncTask. If it is currently running, it will be stopped
 * first by calling CloseAsyncTask ().
 *
 * @param task_p The AsyncTask to clear.
 * @memberof AsyncTask
 */
GRASSROOTS_TASK_API	void ClearAsyncTask (AsyncTask *task_p);




/**
 * Stop a the given AsyncTask if it is currently running
 *
 * @param task_p The AsyncTask to stop.
 * @memberof AsyncTask
 */
GRASSROOTS_TASK_API void CloseAsyncTask (AsyncTask *task_p);


/**
 * Set the SyncData for an AsyncTask.
 *
 * @param task_p The AsyncTask to amend.
 * @param sync_data_p The SyncData to set.
 * @param mem The MEM_FLAG specifying how the SyncData will be dealt with
 * when the AsyncTask is freed.
 * @memberof AsyncTask
 */
GRASSROOTS_TASK_API	bool SetAsyncTaskSyncData (AsyncTask *task_p, SyncData *sync_data_p, MEM_FLAG mem);


/**
 * Set the function and data used when running a given AsyncTask.
 *
 * @param task_p The AsyncTask to amend.
 * @param sync_data_p The SyncData to set.
 * @param mem The MEM_FLAG specifying how the SyncData will be dealt with
 * when the AsyncTask is freed.
 * @memberof AsyncTask
 */
GRASSROOTS_TASK_API	void SetAsyncTaskRunData (AsyncTask *task_p, void *(*run_fn) (void *data_p), void *data_p);



/**
 * Check whether an AsyncTask is currently running.
 *
 * @param task_p The AsyncTask to check.
 * @return <code>true</code> if the AsyncTask is running, <code>
 * false</code> otherwise.
 * @memberof AsyncTask
 */
GRASSROOTS_TASK_API	bool IsAsyncTaskRunning (const AsyncTask *task_p);


/**
 * Run an AsyncTask.
 *
 * @param task_p The AsyncTask to run.
 * @param run_fn The function that the AsyncTask will run. It will
 * be called with the task_data_p parameter.
 * @param task_data_p The parameter to use when calling run_fn.
 * @return <code>true</code> if the AsyncTask was started
 * successfully, <code>false</code> otherwise.
 * @memberof AsyncTask
 */
GRASSROOTS_TASK_API	bool RunAsyncTask (AsyncTask *task_p);


/**
 * Close all currently running AsyncTasks for the current process.
 *
 * @return <code>true</code> if all of the AsyncTasks were closed
 * successfully, <code>false</code> otherwise.
 */
GRASSROOTS_TASK_API	 bool CloseAllAsyncTasks (void);


/**
 * Create an AsyncTaskNode.
 *
 * @return The new AsyncTaskNode or <code>NULL</code> upon error.
 * @memberof AsyncTask
 */
GRASSROOTS_TASK_API	AsyncTaskNode *AllocateAsyncTaskNode (AsyncTask *task_p, MEM_FLAG mem);


/**
 * Free an AsyncTaskNode.
 *
 * @param The AsyncTaskNode to free.
 * @memberof AsyncTask
 */
GRASSROOTS_TASK_API	void FreeAsyncTaskNode (ListItem *node_p);


/**
 * Run an AsyncTask.
 *
 * @param task_p The AsyncTask to run.
 * @param run_fn The function that the AsyncTask will run. It will
 * be called with the task_data_p parameter.
 * @param task_data_p The parameter to use when calling run_fn.
 * @return <code>true</code> if the AsyncTask was started
 * successfully, <code>false</code> otherwise.
 * @memberof AsyncTask
 */
GRASSROOTS_TASK_API void RunEventConsumerFromAsyncTask (AsyncTask *task_p);


#ifdef __cplusplus
}
#endif


#endif /* CORE_SERVER_TASK_INCLUDE_ASYNC_TASK_H_ */
