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

/**
 * A datatype to used to run tasks asynchronously.
 */

typedef struct AsyncTask
{
	char *at_name_s;
} AsyncTask;


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
GRASSROOTS_TASK_API	AsyncTask *CreateAsyncTask (const char *name_s);


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
GRASSROOTS_TASK_API	bool RunAsyncTask (AsyncTask *task_p, void * (*run_fn) (void *data_p), void *task_data_p);


/**
 * Close all currently running AsyncTasks for the current process.
 *
 * @return <code>true</code> if all of the AsyncTasks were closed
 * successfully, <code>false</code> otherwise.
 */
GRASSROOTS_TASK_API	 bool CloseAllAsyncTasks (void);


#ifdef __cplusplus
}
#endif


#endif /* CORE_SERVER_TASK_INCLUDE_ASYNC_TASK_H_ */
