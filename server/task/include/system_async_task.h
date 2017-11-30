/*
** Copyright 2014-2016 The Earlham Institute
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

/**
 * process.h
 *
 *  Created on: 2 Feb 2017
 *      Author: billy
 *
 * @file
 * @brief
 */

#ifndef SHARED_SRC_UTIL_INCLUDE_PROCESS_H_
#define SHARED_SRC_UTIL_INCLUDE_PROCESS_H_


#include "grassroots_task_library.h"
#include "typedefs.h"
#include "operation.h"
#include "service_job.h"
#include "jobs_manager.h"
#include "async_task.h"

/* forward declaration */
struct SystemAsyncTask;

/**
 * A datatype to allow the running of command-line applications
 * in a separate task.
 */
typedef struct SystemAsyncTask
{
	/** The underlying AsyncTask. */
	AsyncTask *std_async_task_p;

	/**
	 * The ServiceJob that will run this SystemAsyncTask.
	 * The SystemAsyncTask will not free this ServiceJob when
	 * the SystemAsyncTask is deleted so the ServiceJob will
	 * need to be freed separately.
	 */
	ServiceJob *std_service_job_p;

	/** The command line that this SystemAsyncTask will run. */
	char *std_command_line_s;

	/**
	 * The callback function that this SystemAsyncTask will call
	 * if it runs successfully. If this is <code>NULL</code>, it will
	 * be ignored.
	 */
	void (*std_on_success_callback_fn) (ServiceJob *job_p);
} SystemAsyncTask;




#ifdef __cplusplus
extern "C"
{
#endif


/**
 * Allocate a SystemAsyncTask.
 *
 * @param job_p The ServiceJob that will run this SystemAsyncTask.
 * @param name_s The name to give to the underlying AsyncTask. The
 * SystemAsyncTask will make a deep copy of this value.
 * @param command_s The command line that this SystemAsyncTask will run.
 * The SystemAsyncTask will make a deep copy of this value.
 * @param on_success_callback_fn If the SystemAsyncTask runs successfully,
 * this function will be called with the SystemAsyncTask's ServiceJob as its
 * parameter.
 * @return The newly-allocated SystemAsyncTask or <code>NULL</code> upon error.
 * @memberof SystemAsyncTask
 */
GRASSROOTS_TASK_API	SystemAsyncTask *AllocateSystemAsyncTask (ServiceJob *job_p, const char *name_s, struct AsyncTasksManager *manager_p, bool add_flag, const char *command_s, void (*on_success_callback_fn) (ServiceJob *job_p));


/**
 * Set the command line for a SystemAsyncTask to run.
 *
 * @param task_p The SystemAsyncTask to alter.
 * @param command_s The command line to run. The SystemAsyncTask will make a deep-copy
 * of this value.
 * @return <code>true</code> if the ServiceJob was altered successfully,
 * <code>false</code> otherwise.
 * @memberof SystemAsyncTask
 */
GRASSROOTS_TASK_API bool SetSystemAsyncTaskCommand (SystemAsyncTask *task_p, const char *command_s);


/**
 * Free a SystemAsyncTask.
 *
 * @param task_p The SystemAsyncTask to free.
 * @memberof SystemAsyncTask
 */
GRASSROOTS_TASK_API	void FreeSystemAsyncTask (SystemAsyncTask *task_p);


/**
 * Run a SystemAsyncTask.
 *
 * @param task_p The SystemAsyncTask to run.
 * @return <code>true</code> if the ServiceJob was started successfully,
 * <code>false</code> otherwise.
 * @memberof SystemAsyncTask
 */
GRASSROOTS_TASK_API bool RunSystemAsyncTask (SystemAsyncTask *task_p);


/**
 * Run the SystemAsyncTask's callback function upon successful completion
 * of the SystemAsyncTask.
 *
 * If the std_on_success_callback_fn function for the given SystemAsyncTask is
 * <code>NULL</code>, then this function will return without error.
 * @param task_p The SystemAsyncTask to run.
 * @param job_p The ServiceJob to pass to std_on_success_callback_fn.
 * @memberof SystemAsyncTask
 */
GRASSROOTS_TASK_LOCAL void RunSystemAsyncTaskSuccess (SystemAsyncTask *task_p, ServiceJob *job_p);


#ifdef __cplusplus
}
#endif


#endif /* SHARED_SRC_UTIL_INCLUDE_PROCESS_H_ */
