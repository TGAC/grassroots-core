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
 * audit.h
 *
 *  Created on: 21 Feb 2017
 *      Author: billy
 *
 * @file
 * @brief
 */

#ifndef SERVER_SRC_LIB_INCLUDE_AUDIT_H_
#define SERVER_SRC_LIB_INCLUDE_AUDIT_H_


#include "grassroots_service_manager_library.h"
#include "../../../shared/services/include/service_job.h"
#include "parameter_set.h"



#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Log the current state of a ServiceJob to a job logger.
 *
 * The Grassroots infrastructure can be configured to send the JSON
 * fragments for all of its ServiceJobs to an external auditing environment
 * if required. The environment is set up in the server configuration.
 *
 * @param job_p The ServiceJob to log.
 * @return <code>true</code> if the ServiceJob was logged successfully,
 * <code>false</code> otherwise.
 * @see GetJobLoggingURI
 */
GRASSROOTS_SERVICE_MANAGER_API bool LogServiceJob (ServiceJob *job_p);


/**
 * Log the current state of a ServiceJob and a ParameterSet to a job logger.
 *
 * The Grassroots infrastructure can be configured to send the JSON
 * fragments for all of its ServiceJobs to an external auditing environment
 * if required. The environment is set up in the server configuration.
 *
 * @param job_p The ServiceJob to log.
 * @param params_p The ParameterSet to log.
 * @return <code>true</code> if the ServiceJob was logged successfully,
 * <code>false</code> otherwise.
 * @see GetJobLoggingURI
 */
GRASSROOTS_SERVICE_MANAGER_API bool LogParameterSet (ParameterSet *params_p, ServiceJob *job_p);


#ifdef __cplusplus
}
#endif


#endif /* SERVER_SRC_LIB_INCLUDE_AUDIT_H_ */
