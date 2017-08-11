/*
 * count_async_task_resource.h
 *
 *  Created on: 9 May 2017
 *      Author: billy
 */

#ifndef CORE_SERVER_TASK_INCLUDE_COUNT_ASYNC_TASK_H_
#define CORE_SERVER_TASK_INCLUDE_COUNT_ASYNC_TASK_H_

#include "async_task.h"


/**
 * A datatype for an AsyncTask that has a counter
 * that sends a signal when that counter reaches a
 * given limit.
 *
 * This is used when you are running a set number of
 * jobs and you are waiting for a certain number of
 * them to complete.
 */
typedef struct CountAsyncTask
{
	/** The underlying AsyncTask */
	AsyncTask *cat_task_p;

	/** The current value of the counter for this CountAsyncTask */
	int32 cat_current_value;

	/**
	 * The value at which this CountAsyncTask will send the
	 * message saying that it has completed.
	 */
	int32 cat_limit;
} CountAsyncTask;



#ifdef __cplusplus
extern "C"
{
#endif


/**
 * Allocate a CountAsyncTask.
 *
 * @param name_s The name to give to this CountAsyncTask. This value will be deep-copied.
 * @param limit The limit at which this CountAsyncTask will signal its completion.
 * @return The newly-allocated CountAsyncTask or <code>NULL</code> upon error.
 * @memberof CountAsyncTask
 */
GRASSROOTS_TASK_API CountAsyncTask *AllocateCountAsyncTask (const char *name_s, int32 limit);


/**
 * Initialise a CountAsyncTask.
 *
 * @param count_task_p The CountAsyncTask to initialise.
 * @param name_s The name to give to this CountAsyncTask. This value will be deep-copied.
 * @param limit The limit at which this CountAsyncTask will signal its completion.
 * @return <code>true</code> if the CountAsyncTask was initialised successfully, <code>false</code>
 * otherwise.
 * @memberof CountAsyncTask
 */
GRASSROOTS_TASK_API bool InitCountAsyncTask (CountAsyncTask *count_task_p, const char *name_s, int32 limit);

/**
 * Get the underlying AsyncTask for a given CountAsyncTask.
 *
 * @param count_task_p The CountAsyncTask to get the AsyncTask for.
 * @return The AsyncTask.
 * @memberof CountAsyncTask
 */
GRASSROOTS_TASK_API AsyncTask *GetAsyncTaskFromCountAsyncTask (const CountAsyncTask *count_task_p);


/**
 * Free a CountAsyncTask.
 *
 * @param count_task_p The CountAsyncTask to free.
 * @memberof CountAsyncTask
 */
GRASSROOTS_TASK_API void FreeCountAsyncTask (CountAsyncTask *count_task_p);


/**
 * Remove and free the underlying AsyncTask from a given CountAsyncTask.
 *
 * @param count_task_p The CountAsyncTask to alter.
 * @memberof CountAsyncTask
 */
GRASSROOTS_TASK_API void ClearCountAsyncTask (CountAsyncTask *count_task_p);


/**
 * Increment the counter for a given CountAsyncTask.
 *
 * If the CountAsyncTask reaches its limit, it will call SendSyncData for
 * its underlying AsyncTask to notify any EventConsumers that it has
 * completed
 * @param count_task_p The CountAsyncTask to increment.
 * @return <code>true</code> if the CountAsyncTask was incremented successfully,
 * <code>false</code> otherwise.
 * @memberof CountAsyncTask
 */
GRASSROOTS_TASK_API bool IncrementCountAsyncTask (CountAsyncTask *count_task_p);


/**
 * The default callback function to check whether a CountAsyncTask could continue
 * running.

 * @param count_task_p The CountAsyncTask to check.
 * @return <code>true</code> if the CountAsyncTask's counter has reached the
 * CountAsyncTask's limit, <code>false</code> otherwise.
 * @memberof CountAsyncTask
 */
GRASSROOTS_TASK_API bool ContinueCountAsyncTask (const CountAsyncTask *count_task_p);


/**
 * Set the limit for a given CountAsyncTask.
 *
 * @param task_p The CountAsyncTask to set the limit for.
 * @param limit The new limit.
 * @memberof CountAsyncTask
 */
GRASSROOTS_TASK_API void SetCountAsyncTaskLimit (CountAsyncTask *task_p, int32 limit);




#ifdef __cplusplus
}
#endif


#endif /* CORE_SERVER_TASK_INCLUDE_COUNT_ASYNC_TASK_H_ */
