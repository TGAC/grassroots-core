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

struct AsyncTasksManager;

/**
 * A datatype to use to run tasks asynchronously.
 */

typedef struct AsyncTask
{
	/** The name of the AsyncTask. */
	char *at_name_s;

	/**
	 * The SyncData used to coordinate data access between
	 * the AsyncTask and other processes.
	 */
	struct SyncData *at_sync_data_p;

	/** The memory flag indicating the ownership of the SyncData. */
	MEM_FLAG at_sync_data_mem;

	/**
	 * The callback function that will be run as a separate task by this
	 * AsyncTask.
	 *
	 * @param data_p The AsyncTask's custom data.
	 */
	void *(*at_run_fn) (void *data_p);

	/**
	 * Any custom data that will be passed to at_fun_fn.
	 */
	void *at_data_p;

	/**
	 * An EventConsumer to notify when the AsyncTask has finished running.
	 */
	EventConsumer *at_consumer_p;

	/**
	 * The AsyncTasksManager that has this AsyncTask.
	 */
	struct AsyncTasksManager *at_manager_p;

} AsyncTask;


/**
 * A datatype to allow storing AsynTasks on a LinkedList.
 */
typedef struct AsyncTaskNode
{
	/** The base list node. */
	ListItem atn_node;

	/** The AsyncTask to store on the list. */
	AsyncTask *atn_task_p;

	/**
	 * The memory flag indicating what action to
	 * perform upon atn_task_p when this AsyncTaskNode
	 * is freed.
	 */
	MEM_FLAG atn_mem;
} AsyncTaskNode;


#ifdef __cplusplus
extern "C"
{
#endif


/**
 * Create an AsyncTask.
 *
 * @param name_s The optional name to give to the AsyncTask. This will
 * be deep-copied by the AsyncTask so this value doesn't have to stay
 * in scope.
 * @param manager_p The AsyncTasksManager that will own this AsyncTask.
 * @param add_flag If this is <code>true</code> then the newly-allocated AsyncTask
 * will be added to the given AsyncTasksManager's list of AsyncTasks, <code>false</code> otherwise.
 * @return The new AsyncTask or <code>NULL</code> upon error.
 * @memberof AsyncTask
 */
GRASSROOTS_TASK_API	AsyncTask *AllocateAsyncTask (const char *name_s, struct AsyncTasksManager *manager_p, bool add_flag);


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
 * @param task_p The AsyncTask to initialise.
 * @param name_s The optional name to give to the AsyncTask. This will
 * be deep-copied by the AsyncTask so this value doesn't have to stay
 * in scope.
 * @param manager_p The AsyncTasksManager that will own this AsyncTask.
 * @param add_flag If this is <code>true</code> then the newly-allocated AsyncTask
 * will be added to the given AsyncTasksManager's list of AsyncTasks, <code>false</code> otherwise.
 * @return <code>true</code> if the AsyncTask was started
 * successfully, <code>false</code> otherwise.
 * @memberof AsyncTask
 */
GRASSROOTS_TASK_API	bool InitialiseAsyncTask (AsyncTask *task_p, const char *name_s, struct AsyncTasksManager *manager_p, bool add_flag);


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
 * @return <code>true</code> if the AsyncTask was altered
 * successfully, <code>false</code> otherwise.
 * @memberof AsyncTask
 */
GRASSROOTS_TASK_API	bool SetAsyncTaskSyncData (AsyncTask *task_p, SyncData *sync_data_p, MEM_FLAG mem);

/**
 * Set the callback function that an AsyncTask will call when it is ran.
 *
 * @param task_p The AsyncTask to run.
 * @param run_fn The function that the AsyncTask will run. It will
 * be called with the task_data_p parameter.
 * @param data_p The parameter to use when calling run_fn.
 * @return <code>true</code> if the AsyncTask was started
 * successfully, <code>false</code> otherwise.
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
 * @param task_p The AsyncTask to store in the newly-created AsyncTaskNode.
 * @param mem The MEM_FLAG specifying how the AsyncTask will be dealt with
 * when the AsyncTaskNode is freed.
 * @return The new AsyncTaskNode or <code>NULL</code> upon error.
 * @memberof AsyncTask
 */
GRASSROOTS_TASK_API	AsyncTaskNode *AllocateAsyncTaskNode (AsyncTask *task_p, MEM_FLAG mem);


/**
 * Free an AsyncTaskNode.
 *
 * @param node_p The AsyncTaskNode to free.
 * @memberof AsyncTask
 */
GRASSROOTS_TASK_API	void FreeAsyncTaskNode (ListItem *node_p);



GRASSROOTS_TASK_API void SetAsyncTaskConsumer (AsyncTask *task_p, EventConsumer *consumer_p);


/**
 * Run the EventConsumer for the given AsyncTask.
 *
 * @param task_p The AsyncTask to run.
 * @memberof AsyncTask
 */
GRASSROOTS_TASK_API void RunEventConsumerFromAsyncTask (AsyncTask *task_p);





#ifdef __cplusplus
}
#endif


#endif /* CORE_SERVER_TASK_INCLUDE_ASYNC_TASK_H_ */
