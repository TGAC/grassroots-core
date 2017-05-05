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


typedef struct SystemTaskData
{
	struct AsyncTask *std_async_task_p;
	ServiceJob *std_service_job_p;
	char *std_command_line_s;
} SystemTaskData;




#ifdef __cplusplus
extern "C"
{
#endif


GRASSROOTS_TASK_API	SystemTaskData *CreateSystemTaskData (ServiceJob *job_p, const char *command_s, bool );


GRASSROOTS_TASK_API	void FreeSystemTaskData (SystemTaskData *task_p);


GRASSROOTS_TASK_API bool RunAsyncSystemTask (SystemTaskData *task_data_p);


#ifdef __cplusplus
}
#endif


#endif /* SHARED_SRC_UTIL_INCLUDE_PROCESS_H_ */
