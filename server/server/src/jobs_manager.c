
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
/*
 * jobs_manager.c
 *
 *  Created on: 27 May 2015
 *      Author: tyrrells
 */

#define ALLOCATE_JOBS_MANAGER_TAGS (1)
#include "jobs_manager.h"
#include "service_config.h"
#include "string_utils.h"


static JobsManager *s_jobs_manager_p = NULL;
static Plugin *s_jobs_manager_plugin_p = NULL;



//
//	Get Symbol
//
JobsManager *GetJobsManagerFromPlugin (Plugin * const plugin_p)
{
	JobsManager *manager_p = NULL;

	if (plugin_p -> pl_type == PN_JOBS_MANAGER)
		{
			manager_p = plugin_p -> pl_value.pv_jobs_manager_p;
		}
	else if (plugin_p -> pl_type == PN_UNKNOWN)
		{
			void *symbol_p = GetSymbolFromPlugin (plugin_p, "GetCustomJobsManager");

			if (symbol_p)
				{
					JobsManager *(*fn_p) (void) = (JobsManager *(*) (void)) symbol_p;

					manager_p = fn_p ();

					if (manager_p)
						{
							manager_p -> jm_plugin_p = plugin_p;

							plugin_p -> pl_value.pv_jobs_manager_p = manager_p;
							plugin_p -> pl_type = PN_JOBS_MANAGER;

							s_jobs_manager_plugin_p = plugin_p;
						}
				}
		}

	return manager_p;
}


JobsManager *GetJobsManager (void)
{
	return s_jobs_manager_p;
}


void InitJobsManager (JobsManager *manager_p,
                      bool (*add_job_fn) (JobsManager *manager_p, uuid_t job_key, ServiceJob *job_p),
                      ServiceJob *(*get_job_fn)  (JobsManager *manager_p, const uuid_t key),
                      ServiceJob *(*remove_job_fn) (JobsManager *manager_p, const uuid_t key, bool get_job_flag),
											LinkedList *(*get_all_jobs_fn) (struct JobsManager *manager_p),
											bool (*delete_manager_fn) (struct JobsManager *manager_p))
{
	manager_p -> jm_add_job_fn = add_job_fn;
	manager_p -> jm_get_job_fn = get_job_fn;
	manager_p -> jm_remove_job_fn = remove_job_fn;
	manager_p -> jm_get_all_jobs_fn = get_all_jobs_fn;
	manager_p -> jm_delete_manager_fn = delete_manager_fn;

	s_jobs_manager_p = manager_p;
}


bool FreeJobsManager (JobsManager *manager_p)
{
	bool success_flag = true;

	if (manager_p -> jm_delete_manager_fn)
		{
			success_flag = (manager_p -> jm_delete_manager_fn (manager_p));
		}

	return true;
}


bool AddServiceJobToJobsManager (JobsManager *manager_p, uuid_t job_key, ServiceJob  *job_p)
{
	return (manager_p -> jm_add_job_fn (manager_p, job_key, job_p));
}


ServiceJob *GetServiceJobFromJobsManager (JobsManager *manager_p, const uuid_t job_key)
{
	return (manager_p -> jm_get_job_fn (manager_p, job_key));
}



ServiceJob *RemoveServiceJobFromJobsManager (JobsManager *manager_p, const uuid_t job_key, bool get_job_flag)
{
	return (manager_p -> jm_remove_job_fn (manager_p, job_key, get_job_flag));
}



ServiceJob *NullDeserialiser (unsigned char *data_p, void *config_p)
{
	return NULL;
}


LinkedList *GetAllServiceJobsFromJobsManager (struct JobsManager *manager_p)
{
	LinkedList *jobs_p = NULL;

	if (manager_p -> jm_get_all_jobs_fn)
		{
			jobs_p = manager_p -> jm_get_all_jobs_fn (manager_p);
		}


	return jobs_p;
}


JobsManager *LoadJobsManager (const char *jobs_manager_s)
{
	const char *plugin_name_s = MakePluginName (jobs_manager_s);
	JobsManager *manager_p = NULL;

	if (plugin_name_s)
		{
			const char *root_path_s = GetServerRootDirectory ();
			char file_sep_s [2];
			char *full_jobs_managers_path_s = NULL;

			*file_sep_s = GetFileSeparatorChar ();
			* (file_sep_s + 1) = '\0';

			full_jobs_managers_path_s = ConcatenateVarargsStrings (root_path_s, file_sep_s, JOBS_MANAGERS_PATH_S, file_sep_s, plugin_name_s, NULL);

			if (full_jobs_managers_path_s)
				{
					Plugin *plugin_p = AllocatePlugin (full_jobs_managers_path_s);

					if (plugin_p)
						{
							bool using_plugin_flag = false;

							if (OpenPlugin (plugin_p))
								{
									manager_p = GetJobsManagerFromPlugin (plugin_p);

									if (manager_p)
										{
											using_plugin_flag = true;
										}
									else
										{

										}
								}		/* if (OpenPlugin (plugin_p)) */
							else
								{

								}

							if (plugin_p && (!using_plugin_flag))
								{
									FreePlugin (plugin_p);
								}


						}		/* if (plugin_p) */

					FreeCopiedString (full_jobs_managers_path_s);
				}		/* if (full_jobs_managers_path_s) */

			FreeCopiedString (plugin_name_s);
		}		/* if (plugin_name_s) */

	return manager_p;
}


