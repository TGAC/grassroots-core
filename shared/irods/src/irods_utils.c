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

#include <stdio.h>

#include "irods_util.h"
#include "grassroots_config.h"


#include "typedefs.h"


void InitRodsEnv (void)
{
	json_t *config_p = GetGlobalConfigValue ("irods");

	if (config_p)
		{
			const char *env_file_s = GetJSONString (config_p, "env_file");

			if (env_file_s)
				{
					/* Use the given iRODS env file */
					setenv ("IRODS_ENVIRONMENT_FILE", env_file_s, 1);
				}
		}

}


bool GetRodsEnv (rodsEnv *rods_env_p)
{
	bool success_flag = true;

	return success_flag;
}


int PrintRodsPath (FILE *out_f, const rodsPath_t * const rods_path_p, const char * const description_s)
{
	return fprintf (out_f, "%s: in \"%s\" out \"%s\"\n", description_s, rods_path_p -> inPath, rods_path_p -> outPath);
}



