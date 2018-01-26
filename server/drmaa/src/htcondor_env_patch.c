/*
 ** Copyright 2014-2017 The Earlham Institute
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
/*
 * htcondor_env_patch.c
 *
 *  Created on: 22 Jan 2018
 *      Author: billy
 */

#include <stdio.h>
#include <string.h>

static const char *s_env_vars_s = NULL;


int set_environment_variables (const char *env_vars_s)
{
	s_env_vars_s = env_vars_s;

	return 0;
}


int prepare_command (char *buffer_s, const size_t buffer_length, const char * const submit_command_s, const char * const submit_file_name_s)
{
	int res = -1;

	if (submit_command_s && submit_file_name_s)
		{
			size_t size = strlen (submit_command_s) + strlen (submit_file_name_s) + 2;

			if (s_env_vars_s)
				{
					size += strlen (s_env_vars_s) + 2;
				}

			if (size < buffer_length)
				{
					int ret;

					if (s_env_vars_s)
						{
							#ifdef WIN32
								ret = sprintf (buffer_s, "%s %s %s", s_env_vars_s, submit_command_s, submit_file_name_s);
							#else
								ret = sprintf (buffer_s, "%s %s %s 2>&1", s_env_vars_s, submit_command_s, submit_file_name_s);
							#endif
						}
					else
						{
							#ifdef WIN32
								ret = sprintf (buffer_s, "%s %s", submit_command_s, submit_file_name_s);
							#else
								ret = sprintf (buffer_s, "%s %s 2>&1", submit_command_s, submit_file_name_s);
							#endif
						}

					if (ret > 0)
						{
							res = 0;
						}
				}

		}		/* if (submit_command_s && submit_file_name_s) */

	return res;
}
