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
 * @file
 * @brief
 */

#ifndef DRMAA_TOOL_H_
#define DRMAA_TOOL_H_

#include "drmaa_library.h"
#include "typedefs.h"
#include "linked_list.h"
#include "operation.h"
#include "json_util.h"
#include "uuid_defs.h"
//#include "drmaa.h"
#include "grassroots_server.h"



/**
 * The default buffer size used for storing job ids
 *
 * @ingroup drmaa_group
 */
#define DRMAA_ID_BUFFER_SIZE (256)

/* forward declaration */
struct drmaa_job_template_s;

/**
 * @struct DrmaaTool
 * @brief A Tool for running Drmaa jobs.
 *
 * Drmaa is a programmatic way of running jobs on grids, clusters or
 * cloud-based systems. It allows to specify things such as the job to
 * run, what resources it will use, whether to run asynchronously, <i>etc.</i>
 *
 * @ingroup drmaa_group
 */
typedef struct DrmaaTool
{
	/** @privatesection */
	char *dt_program_name_s;
	LinkedList *dt_args_p;
	char *dt_queue_name_s;
	char *dt_working_directory_s;

	struct drmaa_job_template_s *dt_job_p;
	char *dt_grassroots_uuid_s;
	char dt_id_s [DRMAA_ID_BUFFER_SIZE];

	char dt_id_out_s [DRMAA_ID_BUFFER_SIZE];

	/** Filename for where to store the stdout/stderr for the drmaa job */
	char *dt_output_filename_s;
	uint32 dt_num_cores;
	uint32 dt_mb_mem_usage;
	char *dt_host_name_s;
	char *dt_user_name_s;
	char **dt_email_addresses_ss;


	char *dt_environment_s;

	bool (*dt_run_fn) (struct DrmaaTool *tool_p, const bool async_flag);

	OperationStatus (*dt_get_status_fn) (struct DrmaaTool *tool_p);

} DrmaaTool;



#ifdef __cplusplus
extern "C"
{
#endif

/** @publicsection */


/**
 * Initialise a DRMAA session
 *
 * This has to be done once globally for any process which
 * will use DRMAA calls
 *
 * @return <code>true</code> if the DRMAA session was initialised successfully, <code>false</code> upon error.
 * @ingroup drmaa_group
 */
GRASSROOTS_DRMAA_API bool InitDrmaa (void);


/**
 * Close a DRMAA session
 *
 * This has to be done once globally for any process which
 * will use DRMAA calls
 *
 * @return <code>true</code> if the DRMAA session was closed successfully, <code>false</code> upon error.
 * @ingroup drmaa_group
 */
GRASSROOTS_DRMAA_API bool ExitDrmaa (void);



/**
 * Allocate a DrmaaTool to run the given program.
 *
 * @param program_name_s The program that this DrmaaTool will run.
 * @param id The id to give to this DrmaaTool.
 * @return A newly-allocated DrmaaTool or <code>NULL</code> upon error.
 * @memberof DrmaaTool
 */
GRASSROOTS_DRMAA_API DrmaaTool *AllocateDrmaaTool (const char *program_name_s, const uuid_t id, GrassrootsServer *grassroots_p);


/**
 * Initialise a DrmaaTool to run the given program.
 *
 * @param tool_p The DrmaaTool to initialise.
 * @param program_name_s The program that this DrmaaTool will run.
 * @param id The id to give to this DrmaaTool.
 * @return <code>true</code> if the DrmaaTool was initialised successfully, <code>false</code> upon error.
 * @memberof DrmaaTool
 */
GRASSROOTS_DRMAA_API bool InitDrmaaTool (DrmaaTool *tool_p, const char *program_name_s, const uuid_t id, GrassrootsServer *grassroots_p);


/**
 * Free a DrmaaTool.
 *
 * The DrmaaTool will be cleared and then the memory for the tool will be freed.
 *
 * @param tool_p The DrmaaTool to free.
 * @memberof DrmaaTool
 * @see ClearDrmaaTool
 */
GRASSROOTS_DRMAA_API void FreeDrmaaTool (DrmaaTool *tool_p);


/**
 * Clear a DrmaaTool.
 *
 * @param tool_p The DrmaaTool to clear.
 * @memberof DrmaaTool
 */
GRASSROOTS_DRMAA_API void ClearDrmaaTool (DrmaaTool *tool_p);


/**
 * Set the current working directory for a DrmaaTool.
 *
 * @param tool_p The DrmaaTool to set the current working directory for.
 * @param path_s The new current working directory . The DrmaaTool will make a copy of this so the parameter
 * does not need to remain in memory after this call.
 * @return <code>true</code> if the current working directory  was set successfully, <code>false</code> otherwise.
 * @memberof DrmaaTool
 */
GRASSROOTS_DRMAA_API bool SetDrmaaToolCurrentWorkingDirectory (DrmaaTool *tool_p, const char *path_s);

/**
 * Set the queue name that a DrmaaTool will set for run its program on.
 *
 * @param tool_p The DrmaaTool to set the queue name for.
 * @param queue_name_s The queue name to use. The DrmaaTool will make a copy of this so the parameter
 * does not need to remain in memory after this call.
 * @return <code>true</code> if the queue name was set successfully, <code>false</code> otherwise.
 * @memberof DrmaaTool
 */
GRASSROOTS_DRMAA_API bool SetDrmaaToolQueueName (DrmaaTool *tool_p, const char *queue_name_s);



/**
 * Set the environment variables that a DrmaaTool will set when it runs its program.
 *
 * @param tool_p The DrmaaTool to set the  environment variables for.
 * @param env_vars_s The environment variables to use. The DrmaaTool will make a copy of this so the parameter
 * does not need to remain in memory after this call.
 * @return <code>true</code> if the environment variables were set successfully, <code>false</code> otherwise.
 * @memberof DrmaaTool
 */
GRASSROOTS_DRMAA_API bool SetDrmaaToolEnvVars (DrmaaTool *tool_p, const char *env_vars_s);


/**
 * Set the number of cores that a DrmaaTool will use when it runs.
 *
 * @param tool_p The DrmaaTool to set the number of cores for.
 * @param num_cores The number of cores that this DrmaaTool will use.
 * @return <code>true</code> if the number of cores was set successfully, <code>false</code> otherwise.
 * @memberof DrmaaTool
 */
GRASSROOTS_DRMAA_API bool SetDrmaaToolCores (DrmaaTool *tool_p, uint32 num_cores);


/**
 * Set the amount of memory that a DrmaaTool will set for its program when it runs.
 *
 * @param tool_p The DrmaaTool to set the memory for.
 * @param mem The amount of memory in MB that this DrmaaTool will use.
 * @return <code>true</code> if the amount of memory was set successfully, <code>false</code> otherwise.
 * @memberof DrmaaTool
 */
GRASSROOTS_DRMAA_API bool SetDrmaaToolMemory (DrmaaTool *tool_p, uint32 mem);


/**
 * Set the host that a DrmaaTool will set for run its program on.
 *
 * @param tool_p The DrmaaTool to set the hostname for.
 * @param host_name_s The hostname to use. The DrmaaTool will make a copy of this so the parameter
 * does not need to remain in memory after this call.
 * @return <code>true</code> if the hostname was set successfully, <code>false</code> otherwise.
 * @memberof DrmaaTool
 */
GRASSROOTS_DRMAA_API bool SetDrmaaToolHostName (DrmaaTool *tool_p, const char *host_name_s);


/**
 * Set the id that represents the job that a DrmaaTool will run.
 *
 * @param tool_p The DrmaaTool to set the id for.
 * @param id_s The id of the job.
 * @return <code>true</code> if the id was set successfully, <code>false</code> otherwise.
 * @memberof DrmaaTool
 */
GRASSROOTS_DRMAA_API bool SetDrmaaToolJobId (DrmaaTool *tool_p, const char *id_s);


/**
 * Set the id that represents the job output that a DrmaaTool will run.
 *
 * @param tool_p The DrmaaTool to set the id for.
 * @param id_s The id of the job.
 * @return <code>true</code> if the id was set successfully, <code>false</code> otherwise.
 * @memberof DrmaaTool
 */
GRASSROOTS_DRMAA_API bool SetDrmaaToolJobOutId (DrmaaTool *tool_p, const char *id_s);


/**
 * Set the program that a DrmaaTool will run.
 *
 * @param tool_p The DrmaaTool to set the job for.
 * @param job_name_s The program that this DrmaaTool will run. The DrmaaTool will make a copy of this so the parameter
 * does not need to remain in memory after this call.
 * @return <code>true</code> if the program was set successfully, <code>false</code> otherwise.
 * @memberof DrmaaTool
 */
GRASSROOTS_DRMAA_API bool SetDrmaaToolJobName (DrmaaTool *tool_p, const char *job_name_s);


/**
 * Redirect the stdout/stderr streams from a DrmaaTool to a file.
 *
 * @param tool_p The DrmaaTool to redirect the streams for.
 * @param output_name_s The filename where the streams output will be written to. The DrmaaTool will make a copy of this so the parameter
 * does not need to remain in memory after this call.
 * @return <code>true</code> if the filename was set successfully, <code>false</code> otherwise.
 * @memberof DrmaaTool
 */
GRASSROOTS_DRMAA_API bool SetDrmaaToolOutputFilename (DrmaaTool *tool_p, const char *output_name_s);


/**
 * Add an argument to the program that a DrmaaTool will run.
 *
 * @param tool_p The DrmaaTool to add the argument for.
 * @param arg_s The argument to add. The DrmaaTool will make a copy of this so the parameter
 * does not need to remain in memory after this call.
 * @return <code>true</code> if the argument was added successfully, <code>false</code> otherwise.
 * @memberof DrmaaTool
 */
GRASSROOTS_DRMAA_API bool AddDrmaaToolArgument (DrmaaTool *tool_p, const char *arg_s);


/**
 * Run a DrmaaTool.
 *
 * @param tool_p The DrmaaTool to add the argument for.
 * @param async_flag If this is <code>true</code> then the method will return straight away and
 * the job will continue to run asynchrnously or in the background. If this is <code>false</code>
 * then this method will not return until the job has completed.
 * @param log_s Optional filename for where to store the id of the Drmaa job that this DrmaaTool runs.
 * If this is <code>NULL</code>, then the id will not be written to a file.
 * @return <code>true</code> if the job was started successfully, <code>false</code> otherwise. To get the
 * status of whether the job completed successfully, use <code>GetDrmaaToolStatus</code> @see GetDrmaaToolStatus.
 * @memberof DrmaaTool
 */
GRASSROOTS_DRMAA_API bool RunDrmaaTool (DrmaaTool *tool_p, const bool async_flag, const char * const log_s);


/**
 * Get the status of job for a DrmaaTool
 *
 * @param tool_p The DrmaaTool to get the job status for.
 * @return The current status of the job for this DrmaaTool.
 * @memberof DrmaaTool
 */
GRASSROOTS_DRMAA_API OperationStatus GetDrmaaToolStatus (DrmaaTool *tool_p);



/**
 * Set the email recipients for any job notifications for a DrmaaTool
 *
 * @param tool_p The DrmaaTool to get the job status for.
 * @param email_addresses_ss An array of email addresses with the final element being NULL.
 * @return <code>true</code> if the email addresses were set successfully, <code>false</code> otherwise.
 * @memberof DrmaaTool
 */
GRASSROOTS_DRMAA_API bool SetDrmaaToolEmailNotifications (DrmaaTool *tool_p, const char **email_addresses_ss);


/**
 * Serialise the DrmaaTool into a JSON fragment.
 *
 * @param tool_p The DrmaaTool.
 * @return The JSON fragment or <code>NULL</code> upon error.
 * @memberof DrmaaTool
 * @see ConvertDrmaaToolFromJSON
 */
GRASSROOTS_DRMAA_API json_t *ConvertDrmaaToolToJSON (const DrmaaTool * const tool_p);


/**
 * Deserialise a DrmaaTool from a JSON fragment.
 *
 * @param json_p The JSON fragment.
 * @return The DrmaaTool or <code>NULL</code> upon error.
 * @memberof DrmaaTool
 * @see ConvertDrmaaToolFromJSON
 */
GRASSROOTS_DRMAA_API DrmaaTool *ConvertDrmaaToolFromJSON (const json_t * const json_p, GrassrootsServer *grassroots_p);


/**
 * Convert a DrmaaTool into a raw unsigned char array for use with the APRJobsManager.
 *
 * @param job_p The DrmaaTool to serialise.
 * @param size_p Upon success, the size of the array will be stored here.
 * @return The raw unsigned char array or <code>NULL</code> upon error.
 * @memberof DrmaaTool
 * @see DeserialiseDrmaaTool
 */
GRASSROOTS_DRMAA_API unsigned char *SerialiseDrmaaTool (const DrmaaTool * const job_p, const size_t *size_p);


/**
 * Create a DrmaaTool from an unsigned char array.
 *
 * @param data_s The raw serialised data representing a DrmaaTool.
 * @return The DrmaaTool or <code>NULL</code> upon error.
 * @memberof DrmaaTool
 * @see SerialiseDrmaaTool
 */
GRASSROOTS_DRMAA_API DrmaaTool *DeserialiseDrmaaTool (const unsigned char * const data_s);

#ifdef __cplusplus
}
#endif

#endif /* DRMAA_TOOL_H_ */
