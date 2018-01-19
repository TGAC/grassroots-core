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
#include <stdlib.h>

#include "drmaa_tool.h"
#include "drmaa_util.h"
#include "memory_allocations.h"
#include "platform.h"
#include "jansson.h"

static DrmaaTool *CreateBlastDrmaaJob (const char *program_name_s, const char *db_name_s, const char *query_filename_s, const char *output_filename_s, const char *log_filename_s, const char **email_addresses_ss, const char *env_vars_s);


int main (int argc, char *argv [])
{
	int ret = 10;
	int num_runs = 1;

	json_t *config_p = NULL;


	if (argc == 2)
		{
			json_error_t err;

			config_p = json_load_file (argv [1], 0, &err);

			if (config_p)
				{
					const char *program_name_s = NULL;

					if ((program_name_s = GetJSONString (config_p, "program")) != NULL)
						{
							const char *query_filename_s = NULL;

							if ((query_filename_s = GetJSONString (config_p, "query")) != NULL)
								{
									const char *output_filename_s = NULL;

									if ((output_filename_s = GetJSONString (config_p, "output")) != NULL)
										{
											const char *db_name_s = NULL;

											if ((db_name_s = GetJSONString (config_p, "db")) != NULL)
												{
													const char *log_filename_s = NULL;

													if ((log_filename_s = GetJSONString (config_p, "log")) != NULL)
														{
															const char *env_vars_s = GetJSONString (config_p, "env");
															const char *email_addresses_ss [2] = { NULL, NULL };

															email_addresses_ss [0] =  GetJSONString (config_p, "email");


															if (InitDrmaa ())
																{
																	DrmaaTool **tools_pp = AllocMemoryArray (num_runs, sizeof (DrmaaTool *));

																	if (tools_pp)
																		{
																			size_t i;
																			bool success_flag = true;

																			for (i = 0; i < num_runs; ++ i)
																				{
																					DrmaaTool *tool_p = CreateBlastDrmaaJob (program_name_s, db_name_s, query_filename_s, output_filename_s, log_filename_s, email_addresses_ss, env_vars_s);

																					if (tool_p)
																						{
																							* (tools_pp + i) = tool_p;
																						}
																					else
																						{
																							success_flag = false;
																							i = num_runs;
																						}
																				}

																			if (success_flag)
																				{
																					for (i = 0; i < num_runs; ++ i)
																						{
																							success_flag = RunDrmaaTool (* (tools_pp + i), true, log_filename_s);

																							if (!success_flag)
																								{
																									printf ("failed to run drmaa tool %lu\n", i);
																								}
																						}

																					if (success_flag)
																						{
																							bool loop_flag = true;

																							/* pause for 5 seconds */
																							Snooze (5000);

																							while (loop_flag)
																								{
																									bool all_finished_flag = true;

																									for (i = 0; i < num_runs; ++ i)
																										{
																											DrmaaTool *tool_p = * (tools_pp + i);

																											OperationStatus status = GetDrmaaToolStatus (tool_p);
																											printf ("drmaa tool " SIZET_FMT " status " INT32_FMT " \n", i, status);

																											if (status == OS_STARTED || status == OS_PENDING)
																												{
																													all_finished_flag = false;
																												}
																										}

																									if (all_finished_flag)
																										{
																											loop_flag = false;
																										}
																									else
																										{
																											/* pause for 1 seconds */
																											Snooze (1000);
																										}
																								}
																						}
																				}

																			for (i = 0; i < num_runs; ++ i)
																				{
																					if (* (tools_pp + i))
																						{
																							FreeDrmaaTool (* (tools_pp + i));
																						}
																				}

																			FreeMemory (tools_pp);
																		}		/* if (tools_pp) */

																	ExitDrmaa ();
																}		/* if (InitDrmaa ()) */


														}		/* if ((log_filename_s = GetJSONString (config_p, "log")) != NULL) */

												}		/* if ((db_name_s = GetJSONString (config_p, "db")) != NULL) */

										}		/* if ((output_filename_s = GetJSONString (config_p, "output")) != NULL) */

								}		/* if ((query_filename_s = GetJSONString (config_p, "query")) != NULL) */

						}		/* if ((program_name_s = GetJSONString (config_p, "program")) != NULL) */

					json_decref (config_p);

				}		/* if (config_p) */

		}

	return ret;
}



static DrmaaTool *CreateBlastDrmaaJob (const char *program_name_s, const char *db_name_s, const char *query_filename_s, const char *output_filename_s, const char *log_filename_s, const char **email_addresses_ss, const char *env_vars_s)
{
	DrmaaTool *tool_p = NULL;
	uuid_t id;

	uuid_generate (id);

	tool_p = AllocateDrmaaTool (program_name_s, id);

	if (tool_p)
		{
			if (1 /*SetDrmaaToolQueueName (tool_p, "webservices")*/)
				{
					if (SetDrmaaToolEmailNotifications (tool_p, email_addresses_ss))
						{
							if (AddDrmaaToolArgument (tool_p, "-db"))
								{
									if (AddDrmaaToolArgument (tool_p, db_name_s))
										{
											if (AddDrmaaToolArgument (tool_p, "-query"))
												{
													if (AddDrmaaToolArgument (tool_p, query_filename_s))
														{
															if (AddDrmaaToolArgument (tool_p, "-out"))
																{
																	if (AddDrmaaToolArgument (tool_p, output_filename_s))
																		{
																			if (SetDrmaaToolOutputFilename (tool_p, log_filename_s))
																				{
																					bool success_flag = true;

																					if (env_vars_s)
																						{
																							success_flag = SetDrmaaToolEnvVars (tool_p, env_vars_s);
																						}

																					if (success_flag)
																						{
																							return tool_p;
																						}
																				}
																		}		/* if (AddDrmaaToolArgument (tool_p, output_filename_s)) */

																}		/* if (AddDrmaaToolArgument (tool_p, "-out")) */

														}		/* if (AddDrmaaToolArgument (tool_p, query_filename_s)) */

												}		/* if (AddDrmaaToolArgument (tool_p, "-query")) */

										}		/* if (AddDrmaaToolArgument (tool_p, db_name_s)) */

								}		/* if (AddDrmaaToolArgument (tool_p, "-db")) */

						}		/* if (SetDrmaaToolEmailNotifictaions (tool_p, email_addresses_ss)) */

				}		/* if (SetDrmaaToolQueueName ("webservices")) */

			FreeDrmaaTool (tool_p);
		}		/* if (tool_p) */

	return NULL;
}
