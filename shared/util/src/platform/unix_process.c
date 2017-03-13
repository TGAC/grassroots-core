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
 * unix_process.c
 *
 *  Created on: 2 Feb 2017
 *      Author: billy
 *
 * @file
 * @brief
 */

#include <errno.h>
#include <spawn.h>
#include <fcntl.h>

#include <sys/wait.h>

#include "process.h"
#include "streams.h"


int32 CreateProcess (char *command_s, char **environment_ss, const char * const logfile_s)
{
	pid_t pid = -1;
	posix_spawn_file_actions_t actions;
	posix_spawn_file_actions_t *actions_p = NULL;
	int res = 0;
	const char * const program_s = "/bin/sh";
	char *args_ss [4];

	*args_ss = program_s;
	* (args_ss + 1) = "-c";
	* (args_ss + 2) = command_s;
	* (args_ss + 3) = NULL;


	if (logfile_s)
		{
			posix_spawn_file_actions_init (&actions);

			if (res == 0)
				{
					res = posix_spawn_file_actions_addopen (&actions, 1, logfile_s, O_WRONLY | O_CREAT | O_TRUNC, 0644);

					if (res == 0)
						{
					    res = posix_spawn_file_actions_adddup2 (&actions, 1, 2);

					    if (res == 0)
					    	{
					    		actions_p = &actions;
					    	}
						}
					else
						{

						}

				}
			else
				{

				}

		}

	res = posix_spawn (&pid, program_s, actions_p, NULL, args_ss, environment_ss);

	return (int32) pid;
}


bool TerminateProcess (int32 process_id)
{
	return false;
}


OperationStatus GetProcessStatus (int32 process_id)
{
	OperationStatus status = OS_ERROR;
	int state;
	pid_t pid = waitpid (process_id, &state, WNOHANG);

	if (pid == 0)
		{
			status = OS_STARTED;
		}
	else if (pid == -1)
		{
			switch (errno)
				{
					case ECHILD:
						PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Process " INT32_FMT " could not be found", process_id);
						break;

					case EINTR:
						PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "waitpid () was interrupted for process " INT32_FMT, process_id);
						break;

					case EINVAL:
						PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "The options value for waitpid () is invalid for process " INT32_FMT, process_id);
						break;
				}
		}
	else
		{
			if (WIFEXITED (state))
				{
					int process_exit_code = WEXITSTATUS (state);

					if (process_exit_code == 0)
						{
							status = OS_SUCCEEDED;
						}
					else
						{
							status = OS_ERROR;
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Process " INT32_FMT " exited with state " INT32_FMT, process_id, process_exit_code);
						}
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Process " INT32_FMT " exited unsuccessfully ", process_id);
				}
		}

	return status;
}

