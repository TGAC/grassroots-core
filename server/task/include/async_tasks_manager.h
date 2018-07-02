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
#include "system_async_task.h"


/* forward declaration */
struct AsyncTasksManagerEventConsumer;


/**
 * A datatype for storing AsyncTasks in a collection.
 */
typedef struct AsyncTasksManager
{
	/** LinkedList of AsyncTaskNodes for storing the AsyncTasks. */
	LinkedList *atm_tasks_p;

	/**
	 * The SyncData used by the AsyncTasksManager to coordinate
	 * access across the differing threads.
	 */
	SyncData *atm_sync_p;


	/**
	 * The CountAsyncTask that keeps the count of
	 * how many of the AsyncTasksManager's AsyncTasks have
	 * been completed
	 */
	struct CountAsyncTask *atm_monitor_p;

	/**
	 * The EventConsumer to call when all of this AsyncTasksManager's
	 * AsyncTasks have been completed.
	 */
	struct AsyncTasksManagerEventConsumer *atm_consumer_p;


	/**
	 * This is the callback function that be called when this AsyncTaskManager's
	 * tasks have finished and any required deallocation routine is needed.
	 *
	 * @param data_p The data to pass to the cleanup function. This will be atm_cleanup_data_p
	 * @return <code>true</code> if the atm_cleanup_fn ran successfully, <code>false</code> otherwise.
	 */
	bool (*atm_cleanup_fn) (void *data_p);

	/**
	 * The data to be passed to atm_cleanup_fn when this AsyncTaskManager's
	 * tasks have finished and any required deallocation routine is needed.
	 */
	void *atm_cleanup_data_p;

	/**
	 * This shows whether the AsyncTasksManager has any AsyncTasks currently running.
	 */
	bool atm_in_use_flag;
} AsyncTasksManager;


/**
 * A datatype that has an EventConsumer and a pointer to
 * the AsyncTasksManager that owns it.
 */
typedef struct AsyncTasksManagerEventConsumer
{
	/** The EventConsumer. */
	EventConsumer atmec_base_consumer;

	/** The AsyncTasksManager that owns this AsyncTasksManagerEventConsumer. */
	AsyncTasksManager *atmec_tasks_manager_p;
} AsyncTasksManagerEventConsumer;







#ifdef __cplusplus
extern "C"
{
#endif


/**
 * Allocate an AsyncTasksManager with the given name.
 *
 * @param name_s The name to give to the AsyncTasksManager. This value
 * will be deep-copied.
 * @param cleanup_fn The callback function that the AsyncTasksManager will call when all of its
 * AsyncTasks have finished. This is optional and can be <code>NULL</code> if not needed.
 * @param cleanup_data_p The data to pass to the cleanup function when all of its
 * AsyncTasks have finished. This is optional and can be <code>NULL</code> if not needed.
 * @return The new AsyncTasksManager or <code>NULL</code> upon error.
 * @memberof AsyncTasksManager
 */

GRASSROOTS_TASK_API AsyncTasksManager *AllocateAsyncTasksManager (const char * const name_s, bool (*cleanup_fn) (void *data_p), void *cleanup_data_p);


/**
 * Free an AsyncTasksManager.
 *
 * @param manager_p The AsyncTasksManager to free.
 * @memberof AsyncTasksManager
 */
GRASSROOTS_TASK_API void FreeAsyncTasksManager (AsyncTasksManager *manager_p);


/**
 * Add an AsyncTask to a AsyncTasksManager.
 *
 * @param manager_p The AsyncTasksManager that the given AsyncTask will be added to.
 * @param task_p The AsyncTask to add.
 * @param mem The MEM_FLAG determining the ownership of the AsyncTask and if it
 * will be freed when the AsyncTasksManager is freed.
 * @return <code>true</code> if the AsynTask was added successfully, <code>false</code> otherwise.
 * @memberof AsyncTasksManager
 */
GRASSROOTS_TASK_API bool AddAsyncTaskToAsyncTasksManager (AsyncTasksManager *manager_p, AsyncTask *task_p, MEM_FLAG mem);


/**
 * Allocate a SystemAsyncTask and add it a given AsyncTasksManager.
 *
 * @param manager_p The AsyncTasksManager to add the new SystemAsyncTask to
 * @param std_service_job_p The ServiceJob that will run this SystemAsyncTask.
 * @param std_command_line_s The command line that this SystemAsyncTask will run.
 * The SystemAsyncTask will make a deep copy of this value.
 * @param on_success_callback_fn If the SystemAsyncTask runs successfully,
 * this function will be called with the SystemAsyncTask's ServiceJob as its
 * parameter.
 * @return The newly-allocated SystemAsyncTask or <code>NULL</code> upon error.
 * @see AllocateSystemAsyncTask()
 * @memberof AsyncTasksManager
 */
GRASSROOTS_TASK_API SystemAsyncTask *AllocateSystemAsyncTaskFromAsyncTasksManager (AsyncTasksManager *manager_p, ServiceJob *std_service_job_p, char *std_command_line_s, void (*on_success_callback_fn) (ServiceJob *job_p));

/**
 * Create a new AsyncTasksManagerEventConsumer and store it in the given AsyncTasksManager.
 *
 * @param consumer_fn The callback function that the new AsyncTasksManagerEventConsumer's atmec_base_consumer will use.
 * @param manager_p The AsyncTasksManager that will store this AsyncTasksManagerEventConsumer.
 * @return The new AsyncTasksManagerEventConsumer or <code>NULL</code> upon error.
 * @memberof AsyncTasksManagerEventConsumer
 */
GRASSROOTS_TASK_API AsyncTasksManagerEventConsumer *AllocateAsyncTasksManagerEventConsumer (void (*consumer_fn) (EventConsumer *consumer_p, struct AsyncTask *task_p), AsyncTasksManager *manager_p);


/**
 * Free an AsyncTasksManagerEventConsumer.
 *
 * @param consumer_p The AsyncTasksManagerEventConsumer to free.
 * @memberof AsyncTasksManagerEventConsumer
 */
GRASSROOTS_TASK_API void FreeAsyncTasksManagerEventConsumer (AsyncTasksManagerEventConsumer *consumer_p);


/**
 * Perform the required functionality prior to running an AsyncTasksManager's
 * AsyncTasks.
 *
 * Although you can call this, the wrapper function RunAsyncTasksManagerTasks()
 * calls this then runs all of the AsyncTasks so may well be more useful.
 *
 * @param manager_p The AsyncTasksManager which will be prepared prior to
 * running its tasks.
 * @param initial_counter_value The value to be added to the size of the list of AsyncTasks that this AsyncTasksManager
 * has. This is if you have custom behaviour in the Service. See PreRunJobs() in the Blast or Polymarker Service for
 * examples of this.
 * @memberof AsyncTasksManager
 */
GRASSROOTS_TASK_API void PrepareAsyncTasksManager (AsyncTasksManager *manager_p, const int32 initial_counter_value);


/**
 * Start all AsyncTasks within an AsyncTasksManager
 *
 * @param manager_p The AsyncTasksManager whose AsyncTasks will be ran.
 * @return <code>true</code> if all of the AsyncTasks were started successfully,
 * <code>false</code> otherwise.
 * @memberof AsyncTasksManager
 */
GRASSROOTS_TASK_API bool StartAsyncTaskManagerWorkers (AsyncTasksManager *manager_p);


/**
 * Run all AsyncTasks within an AsyncTasksManager
 *
 * This is simply a wrapper that calls PrepareAsyncTasksManager() followed by
 * StartAsyncTaskManagerWorkers().
 *
 * @param manager_p The AsyncTasksManager whose AsyncTasks will be ran.
 * @return <code>true</code> if all of the AsyncTasks were started successfully,
 * <code>false</code> otherwise.
 * @memberof AsyncTasksManager
 */
GRASSROOTS_TASK_API bool RunAsyncTasksManagerTasks (AsyncTasksManager *manager_p);


/**
 * Check whether an AsyncTasksManager has any AsyncTasks currently running,
 *
 * @param manager_p The AsyncTasksManager to check.
 * @return <code>true</code> if any AsyncTasks are currently running,
 * <code>false</code> otherwise.
 * @memberof AsyncTasksManager
 */
GRASSROOTS_TASK_API bool IsAsyncTaskManagerRunning (const AsyncTasksManager *manager_p);



/**
 * Notify the AsyncTasksManager that one of its AsyncTasks has finished.
 *
 * @param manager_p The AsyncTasksManager to amend.
 * @memberof AsyncTasksManager
 */
GRASSROOTS_TASK_API void IncrementAsyncTaskManagerCount (AsyncTasksManager *manager_p);


#ifdef __cplusplus
}
#endif

#endif /* CORE_SERVER_TASK_INCLUDE_ASYNC_TASKS_MANAGER_H_ */
