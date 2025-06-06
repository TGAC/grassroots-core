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
#include <string.h>

#define ALLOCATE_SERVICE_JOB_TAGS (1)
#include "service_job.h"
#include "service.h"

#include "string_utils.h"
#include "json_tools.h"
#include "json_parameter.h"
#include "jobs_manager.h"
#include "uuid_util.h"

#include "data_resource.h"

#ifdef _DEBUG
#define SERVICE_JOB_DEBUG	(STM_LEVEL_INFO)
#else
#define SERVICE_JOB_DEBUG	(STM_LEVEL_NONE)
#endif


static bool CopyValidJSON (const json_t *src_p, const char *src_key_s, json_t **dest_pp);

static bool AddValidJSON (json_t *parent_p, const char * const key_s, json_t *child_p, bool set_as_new_flag);

static bool AddStatusToServiceJobJSON (ServiceJob *job_p, json_t *value_p);


static bool AddLinkedServicesToServiceJobJSON (ServiceJob *job_p, json_t *value_p);

static bool AddResultEntryToServiceJob (ServiceJob *job_p, json_t **results_pp, json_t *result_to_add_p);

static json_t *CreateAndAddErrorObjectForParameter (json_t *root_p, const char *param_s, const ParameterType param_type, const bool add_type_flag);



ServiceJob *AllocateEmptyServiceJob (void)
{
	ServiceJob *job_p = (ServiceJob *) AllocMemory (sizeof (ServiceJob));

	if (job_p)
		{
			memset (job_p, 0, sizeof (ServiceJob));
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE,  __FILE__, __LINE__, "Failed to allocate empty ServiceJob");
		}

	return job_p;
}



ServiceJobSet *AllocateSimpleServiceJobSet (struct Service *service_p, const char *job_name_s, const char *job_description_s)
{
	ServiceJobSet *job_set_p = AllocateServiceJobSet (service_p);

	if (job_set_p)
		{
			ServiceJob *job_p = CreateAndAddServiceJobToService (service_p, job_name_s, job_description_s, NULL, NULL, NULL);

			if (job_p)
				{
					return job_set_p;
				}

			FreeServiceJobSet (job_set_p);
		}		/* if (job_set_p) */

	return NULL;
}



ServiceJob *AllocateServiceJob (Service *service_p, const char *job_name_s, const char *job_description_s, bool (*update_fn) (struct ServiceJob *job_p), bool (*calculate_results_fn) (struct ServiceJob *job_p), void (*free_job_fn) (struct ServiceJob *job_p), const char *job_type_s)
{
	ServiceJob *job_p = AllocateEmptyServiceJob ();

	if (job_p)
		{
			if (InitServiceJob (job_p, service_p, job_name_s, job_description_s, update_fn, calculate_results_fn, free_job_fn, NULL, job_type_s))
				{
					return job_p;
				}

			FreeServiceJob (job_p);
		}		/* if (job_p) */

	return NULL;
}


ServiceJob *CreateAndAddServiceJobToService (Service *service_p, const char *job_name_s, const char *job_description_s, bool (*update_fn) (struct ServiceJob *job_p), bool (*calculate_results_fn) (struct ServiceJob *job_p), void (*free_job_fn) (struct ServiceJob *job_p))
{
	ServiceJob *job_p = AllocateServiceJob (service_p, job_name_s, job_description_s, update_fn, calculate_results_fn, free_job_fn, SJ_DEFAULT_TYPE_S);

	return job_p;
}


bool InitServiceJob (ServiceJob *job_p, Service *service_p, const char *job_name_s, const char *job_description_s, bool (*update_fn) (struct ServiceJob *job_p), bool (*calculate_results_fn) (struct ServiceJob *job_p), void (*free_job_fn) (struct ServiceJob *job_p), uuid_t *id_p, const char *job_type_s)
{
#if SERVICE_JOB_DEBUG >= STM_LEVEL_FINER
	PrintLog (STM_LEVEL_FINE, __FILE__, __LINE__, "Initialising Job: %.16x\n", job_p);
#endif

	memset (job_p, 0, sizeof (ServiceJob));

	job_p -> sj_service_p = service_p;

	if (id_p)
		{
			PrintLog (STM_LEVEL_INFO, __FILE__, __LINE__, "InitServiceJob () for \"%s\" with name \"%s\" calling uuid_copy", GetServiceName (service_p), job_name_s ? job_name_s : "NULL");
			uuid_copy (job_p -> sj_id, *id_p);
			PrintUUIDT (& (job_p -> sj_id), "InitServiceJob () with uuid_copy dest");
			PrintUUIDT (id_p, "InitServiceJob () with uuid_copy src");
		}
	else
		{
			PrintLog (STM_LEVEL_INFO, __FILE__, __LINE__, "InitServiceJob () for \"%s\" with name \"%s\" calling uuid_generate ()", GetServiceName (service_p), job_name_s ? job_name_s : "NULL");
			uuid_generate (job_p -> sj_id);
			PrintUUIDT (& (job_p -> sj_id), "InitServiceJob () with uuid_generate ()");
		}

	SetServiceJobStatus (job_p, OS_IDLE);

	job_p -> sj_errors_p = json_object ();

	if (job_p -> sj_errors_p)
		{
			job_p -> sj_linked_services_p = json_array ();

			if (job_p -> sj_linked_services_p)
				{
					char *copied_service_name_s = NULL;
					char *copied_job_name_s = NULL;
					char *copied_job_description_s = NULL;

					if (CloneValidString (GetServiceName (service_p), &copied_service_name_s))
						{
							if (CloneValidString (job_name_s, &copied_job_name_s))
								{
									if (CloneValidString (job_description_s, &copied_job_description_s))
										{
											char *copied_job_type_s = NULL;

											if (CloneValidString (job_type_s, &copied_job_type_s))
												{
													if (AddServiceJobToService (service_p, job_p))
														{
															job_p -> sj_name_s = copied_job_name_s;
															job_p -> sj_description_s = copied_job_description_s;
															job_p -> sj_service_name_s = copied_service_name_s;

															job_p -> sj_url_s = NULL;

															job_p -> sj_result_p = NULL;
															job_p -> sj_metadata_p = NULL;

															job_p -> sj_update_fn = update_fn;
															job_p -> sj_free_fn = free_job_fn;

															job_p -> sj_calculate_result_fn = calculate_results_fn;

															job_p -> sj_is_updating_flag = false;

															job_p -> sj_type_s = copied_job_type_s;

				#if SERVICE_JOB_DEBUG >= STM_LEVEL_FINE
															{
																char uuid_s [UUID_STRING_BUFFER_SIZE];

																ConvertUUIDToString (job_p -> sj_id, uuid_s);
																PrintLog (STM_LEVEL_FINE, __FILE__, __LINE__, "Job: %s\n", uuid_s);
															}
				#endif

															return true;
														}
													else
														{
															PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add job \"%s\" to service  \"%s\"", job_name_s, GetServiceName (service_p));
														}

												}
											else
												{
													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to copy job type \"%s\"", job_type_s);
												}
										}
									else
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to copy job description \"%s\"", job_description_s);
										}

									if (copied_job_name_s)
										{
											FreeCopiedString (copied_job_name_s);
										}
								}
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to copy job name \"%s\"", job_name_s);
								}

							if (copied_job_description_s)
								{
									FreeCopiedString (copied_job_description_s);
								}
						}

				}		/* if (job_p -> sj_linked_services_p) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create job linked services array");
				}

			json_decref (job_p -> sj_errors_p);
			job_p -> sj_errors_p = NULL;
		}		/* if (job_p -> sj_errors_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create job errors");
		}

	return false;
}



ServiceJob *CloneServiceJob (const ServiceJob *src_p)
{
	ServiceJob *dest_p = (ServiceJob *) AllocMemory (sizeof (ServiceJob));

	if (dest_p)
		{
			memset (dest_p, 0, sizeof (ServiceJob));

			if (!CopyServiceJob (src_p, dest_p))
				{
					FreeMemory (dest_p);
					dest_p = NULL;
				}
		}

	return dest_p;
}


bool CopyServiceJob (const ServiceJob *src_p, ServiceJob *dest_p)
{
	char *service_name_s = NULL;

	if (CloneValidString (src_p -> sj_service_name_s, &service_name_s))
		{
			char *job_name_s = NULL;

			if (CloneValidString (src_p -> sj_name_s, &job_name_s))
				{
					char *job_description_s = NULL;

					if (CloneValidString (src_p -> sj_description_s, &job_description_s))
						{
							json_t *result_p = NULL;

							if (DeepCopyValidJSON (src_p -> sj_result_p, &result_p))
								{
									json_t *metadata_p = NULL;

									if (DeepCopyValidJSON (src_p -> sj_metadata_p, &metadata_p))
										{
											json_t *errors_p = NULL;

											if (DeepCopyValidJSON (src_p -> sj_errors_p, &errors_p))
												{
													json_t *linked_services_p = NULL;

													if (DeepCopyValidJSON (src_p -> sj_linked_services_p, &linked_services_p))
														{
															GrassrootsServer *grassroots_p = GetGrassrootsServerFromService (src_p -> sj_service_p);
															Service *copied_service_p = GetServiceByName (grassroots_p, service_name_s, NULL);

															if (copied_service_p)
																{
																	ClearServiceJob (dest_p);

																	dest_p -> sj_service_p = copied_service_p;

																	uuid_copy (dest_p -> sj_id, src_p -> sj_id);

																	SetServiceJobStatus (dest_p, src_p -> sj_status);

																	dest_p -> sj_service_name_s = service_name_s;
																	dest_p -> sj_name_s = job_name_s;
																	dest_p -> sj_description_s = job_description_s;

																	dest_p -> sj_result_p = result_p;
																	dest_p -> sj_metadata_p = metadata_p;
																	dest_p -> sj_errors_p = errors_p;
																	dest_p -> sj_linked_services_p = linked_services_p;
																	dest_p -> sj_result_p = result_p;

																	dest_p -> sj_update_fn = src_p -> sj_update_fn;
																	dest_p -> sj_free_fn = src_p -> sj_free_fn;

																	dest_p -> sj_is_updating_flag = false;


																	return true;
																}


														}		/* if (DeepCopyValidJSON (src_p -> sj_linked_services_p, &linked_services_p)) */

													if (errors_p)
														{
															json_decref (errors_p);
														}

												}		/* if (DeepCopyValidJSON (src_p -> sj_errors_p, &errors_p)) */

											if (metadata_p)
												{
													json_decref (metadata_p);
												}

										}		/* if (DeepCopyValidJSON (src_p -> sj_metadata_p, &metadata_p)) */

									if (result_p)
										{
											json_decref (result_p);
										}

								}		/* if (DeepCopyValidJSON (src_p -> sj_result_p, &result_p)) */

							if (job_description_s)
								{
									FreeCopiedString (job_description_s);
								}

						}		/* if (CloneValidString (src_p -> sj_description_s, &job_description_s)) */

					if (job_name_s)
						{
							FreeCopiedString (job_name_s);
						}

				}		/* if (CloneValidString (src_p -> sj_name_s, &job_name_s)) */

			if (service_name_s)
				{
					FreeCopiedString (service_name_s);
				}

		}		/* if (CloneValidString (src_p -> sj_service_name_s, &service_name_s)) */

	return false;
}


void ClearServiceJob (ServiceJob *job_p)
{
#if SERVICE_JOB_DEBUG >= STM_LEVEL_FINER
	PrintLog (STM_LEVEL_FINE, __FILE__, __LINE__, "Clearing Job: %.16x\n", job_p);
#endif

	if (job_p -> sj_name_s)
		{
			FreeCopiedString (job_p -> sj_name_s);
			job_p -> sj_name_s = NULL;
		}

	if (job_p -> sj_description_s)
		{
			FreeCopiedString (job_p -> sj_description_s);
			job_p -> sj_description_s = NULL;
		}


	if (job_p -> sj_result_p)
		{
#if SERVICE_JOB_DEBUG >= STM_LEVEL_FINER
			PrintJSONRefCounts (STM_LEVEL_FINER, __FILE__, __LINE__, job_p -> sj_result_p, "pre decref res_p");
#endif

			json_decref (job_p -> sj_result_p);

			job_p -> sj_result_p = NULL;
		}

	if (job_p -> sj_metadata_p)
		{
			json_decref (job_p -> sj_metadata_p);
			job_p -> sj_metadata_p = NULL;
		}

	if (job_p -> sj_errors_p)
		{
			json_decref (job_p -> sj_errors_p);
			job_p -> sj_errors_p = NULL;
		}


	if (job_p -> sj_linked_services_p)
		{
			json_decref (job_p -> sj_linked_services_p);
			job_p -> sj_linked_services_p = NULL;
		}

	if (job_p -> sj_service_name_s)
		{
			FreeCopiedString (job_p -> sj_service_name_s);
			job_p -> sj_service_name_s = NULL;
		}

	if (job_p -> sj_type_s)
		{
			FreeCopiedString (job_p -> sj_type_s);
			job_p -> sj_type_s = NULL;
		}

	SetServiceJobStatus (job_p, OS_CLEANED_UP);
}


Service *GetServiceFromServiceJob (ServiceJob *job_p)
{
	if (! (job_p -> sj_service_p))
		{
			GrassrootsServer *grassroots_p = GetGrassrootsServerFromService (job_p -> sj_service_p);
			job_p -> sj_service_p = GetServiceByName (grassroots_p, job_p -> sj_service_name_s, NULL);

			if (! (job_p -> sj_service_p))
				{
					char uuid_s [UUID_STRING_BUFFER_SIZE];

					ConvertUUIDToString (job_p -> sj_id, uuid_s);

					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get service with name \"%s\" for ServiceJob \"%s\"", job_p -> sj_service_name_s, uuid_s);
				}
		}

	return (job_p -> sj_service_p);
}


bool SetServiceJobName (ServiceJob *job_p, const char * const name_s)
{
	bool success_flag = ReplaceStringValue (& (job_p -> sj_name_s), name_s);

	return success_flag;
}


bool SetServiceJobDescription (ServiceJob *job_p, const char * const description_s)
{
	bool success_flag = ReplaceStringValue (& (job_p -> sj_description_s), description_s);

	return success_flag;
}


bool SetServiceJobURL (ServiceJob *job_p, const char * const url_s)
{
	bool success_flag = ReplaceStringValue (& (job_p -> sj_url_s), url_s);

	return success_flag;
}


void SetServiceJobStatus (ServiceJob *job_p, OperationStatus status)
{
#if SERVICE_JOB_DEBUG >= STM_LEVEL_FINER
	PrintLog (STM_LEVEL_FINE, __FILE__, __LINE__, "Setting Job \"%s\" to status \"%s\"", job_p -> sj_name_s ? job_p -> sj_name_s : "unnamed", GetOperationStatusAsString (status));
#endif

	job_p -> sj_status = status;
}



void MergeServiceJobStatus (ServiceJob *job_p, OperationStatus status)
{
	MergeOperationStatuses (& (job_p -> sj_status), status);
}


uint32 GetNumberOfServiceJobResults (const ServiceJob *job_p)
{
	uint32 size = 0;

	if (job_p -> sj_result_p)
		{
			size = (uint32) json_array_size (job_p -> sj_result_p);
		}		/* if (job_p -> sj_result_p) */

	return size;
}

void FreeServiceJob (ServiceJob *job_p)
{
	#if SERVICE_JOB_DEBUG >= STM_LEVEL_FINER
	PrintLog (STM_LEVEL_FINER, __FILE__, __LINE__, "FreeServiceJob freed job \"%s\" at %.16X", job_p -> sj_name_s, job_p);
	#endif


	if (job_p -> sj_free_fn)
		{
			job_p -> sj_free_fn (job_p);
		}
	else
		{
			FreeBaseServiceJob (job_p);
		}
}


void FreeBaseServiceJob (ServiceJob *job_p)
{
	ClearServiceJob (job_p);
	FreeMemory (job_p);
}


ServiceJobSet *AllocateServiceJobSet (Service *service_p)
{
	ServiceJobSet *job_set_p = (ServiceJobSet *) AllocMemory (sizeof (ServiceJobSet));

	if (job_set_p)
		{
			job_set_p -> sjs_jobs_p = AllocateLinkedList (FreeServiceJobNode);

			if (job_set_p -> sjs_jobs_p)
				{
					job_set_p -> sjs_service_p = service_p;

					if (service_p -> se_jobs_p)
						{
							FreeServiceJobSet (service_p -> se_jobs_p);
						}
					service_p -> se_jobs_p = job_set_p;

					return job_set_p;
				}

			FreeMemory (job_set_p);
		}		/* if (job_set_p) */

	return NULL;
}


void FreeServiceJobSet (ServiceJobSet *jobs_p)
{
	FreeLinkedList (jobs_p -> sjs_jobs_p);
	FreeMemory (jobs_p);
}


uint32 GetServiceJobSetSize (const ServiceJobSet * const jobs_p)
{
	return (jobs_p -> sjs_jobs_p -> ll_size);
}



ServiceJobNode *AllocateServiceJobNode (ServiceJob *job_p)
{
	ServiceJobNode *node_p = (ServiceJobNode *) AllocMemory (sizeof (ServiceJobNode));

	if (node_p)
		{
			node_p -> sjn_job_p = job_p;
			node_p -> sjn_node.ln_prev_p = NULL;
			node_p -> sjn_node.ln_next_p = NULL;
		}

	return node_p;
}


void FreeServiceJobNode (ListItem *node_p)
{
	ServiceJobNode *service_job_node_p = (ServiceJobNode *) node_p;

	if (service_job_node_p)
		{
			FreeServiceJob (service_job_node_p -> sjn_job_p);
		}

	FreeMemory (service_job_node_p);
}



ServiceJobNode *FindServiceJobNodeByUUIDInServiceJobSet (const ServiceJobSet *job_set_p, const uuid_t job_id)
{
	if (job_set_p)
		{
			ServiceJobNode *node_p = (ServiceJobNode *) (job_set_p -> sjs_jobs_p -> ll_head_p);

			while (node_p)
				{
					if (uuid_compare (node_p -> sjn_job_p -> sj_id, job_id) == 0)
						{
							return node_p;
						}
				}
		}

	return NULL;
}


ServiceJob *GetServiceJobFromServiceJobSetById (const ServiceJobSet *jobs_p, const uuid_t job_id)
{
	ServiceJobNode *node_p = FindServiceJobNodeByUUIDInServiceJobSet (jobs_p, job_id);
	ServiceJob *job_p = NULL;

	if (node_p)
		{
			job_p = node_p -> sjn_job_p;
		}

	return job_p;
}



bool IsServiceJobInServiceJobSet (const ServiceJobSet *job_set_p, const ServiceJob *job_p)
{
	return (FindServiceJobNodeInServiceJobSet (job_set_p, job_p) != NULL);
}


ServiceJobNode *FindServiceJobNodeInServiceJobSet (const ServiceJobSet *job_set_p, const ServiceJob *job_p)
{
	if (job_set_p && job_p)
		{
			ServiceJobNode *node_p = (ServiceJobNode *) (job_set_p -> sjs_jobs_p -> ll_head_p);

			while (node_p)
				{
					if (node_p -> sjn_job_p == job_p)
						{
							return node_p;
						}
				}
		}

	return NULL;
}


bool RemoveServiceJobByUUIDFromServiceJobSet (ServiceJobSet *job_set_p, uuid_t job_id)
{
	bool removed_flag = false;
	ServiceJobNode *node_p = FindServiceJobNodeByUUIDInServiceJobSet (job_set_p, job_id);

	if (node_p)
		{
			LinkedListRemove (job_set_p -> sjs_jobs_p, (ListItem *) node_p);
			removed_flag = true;
		}

	return removed_flag;
}


bool RemoveServiceJobFromServiceJobSet (ServiceJobSet *job_set_p, ServiceJob *job_p)
{
	bool removed_flag = false;
	ServiceJobNode *node_p = FindServiceJobNodeInServiceJobSet (job_set_p, job_p);

	if (node_p)
		{
			LinkedListRemove (job_set_p -> sjs_jobs_p, (ListItem *) node_p);
			removed_flag = true;
		}

	return removed_flag;
}


bool CloseServiceJob (ServiceJob *job_p)
{
	bool success_flag = true;

#if SERVICE_JOB_DEBUG >= STM_LEVEL_FINER
	PrintLog (STM_LEVEL_FINER, __FILE__, __LINE__, "Closing ServiceJob %.8X for %s", job_p, job_p -> sj_service_p);
#endif

	if (job_p -> sj_result_p)
		{
#if SERVICE_JOB_DEBUG >= STM_LEVEL_FINER
			PrintJSONRefCounts (STM_LEVEL_FINER, __FILE__, __LINE__, job_p -> sj_result_p, "pre wipe results");
#endif

			json_decref (job_p -> sj_result_p);

#if SERVICE_JOB_DEBUG >= STM_LEVEL_FINER
			PrintJSONRefCounts (STM_LEVEL_FINER, __FILE__, __LINE__, job_p -> sj_result_p, "post wipe results");
#endif

			job_p -> sj_result_p = NULL;
		}

	return success_flag;
}


void ClearServiceJobResults (ServiceJob *job_p, bool free_memory_flag)
{
	if (job_p -> sj_result_p)
		{
			if (free_memory_flag)
				{
					json_decref (job_p -> sj_result_p);
				}

			job_p -> sj_result_p = NULL;
		}
}



static bool AddValidJSON (json_t *parent_p, const char * const key_s, json_t *child_p, bool set_as_new_flag)
{
	bool success_flag = true;

	if (child_p)
		{
			bool add_flag = true;

			/* check that the JSON object is not empty */
			if (json_is_array (child_p))
				{
					add_flag = (json_array_size (child_p) != 0);
				}
			else if (json_is_object (child_p))
				{
					add_flag = (json_object_size (child_p) != 0);
				}

			if (add_flag)
				{
					if (set_as_new_flag)
						{
							if (json_object_set_new (parent_p, key_s, child_p) != 0)
								{
									success_flag = false;
								}
						}
					else
						{
							if (json_object_set (parent_p, key_s, child_p) != 0)
								{
									success_flag = false;
								}
						}

				}		/* if (add_flag) */

		}		/* if (child_p) */

	if (!success_flag)
		{
			PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to add child json for \"%s\" set_as_new_flag: %d", key_s, set_as_new_flag);
		}

	return success_flag;
}


static bool AddStatusToServiceJobJSON (ServiceJob *job_p, json_t *value_p)
{
	bool success_flag = false;

	if (json_is_object (value_p))
		{
			if (json_object_set_new (value_p, SERVICE_STATUS_VALUE_S, json_integer (job_p -> sj_status)) == 0)
				{
					const char *status_text_s = GetOperationStatusAsString (job_p -> sj_status);

					if (status_text_s)
						{
							if (json_object_set_new (value_p, SERVICE_STATUS_S, json_string (status_text_s)) == 0)
								{
									success_flag = true;
								}
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add status text \"%s\" to json", status_text_s);
								}
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get status text for ", job_p -> sj_status);
						}
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add status value \"%d\" to json", job_p -> sj_status);
				}
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Job status JSON can only be added to an object, value is of type %d", json_typeof (value_p));
		}

	return success_flag;
}


static bool AddLinkedServicesToServiceJobJSON (ServiceJob *job_p, json_t *value_p)
{
	bool success_flag = true;

	if ((job_p -> sj_linked_services_p) && (json_array_size (job_p -> sj_linked_services_p) > 0))
		{

		}

	return success_flag;
}


bool GetOperationStatusFromServiceJobJSON (const json_t *value_p, OperationStatus *status_p)
{
	bool success_flag = false;

	if (json_is_object (value_p))
		{
			json_int_t i;

			if (GetJSONInteger (value_p, SERVICE_STATUS_VALUE_S, &i))
				{
					OperationStatus s = (OperationStatus) i;

					if ((s > OS_LOWER_LIMIT) && (s < OS_UPPER_LIMIT))
						{
							*status_p = s;
							success_flag = true;
						}
				}
			else
				{
					const char *status_s = GetJSONString (value_p, SERVICE_STATUS_S);

					if (status_s)
						{
							OperationStatus s = GetOperationStatusFromString (status_s);

							if (s != OS_NUM_STATUSES)
								{
									*status_p = s;
									success_flag = true;
								}
						}
				}		/* if (!GetJSONInteger (value_p, SERVICE_STATUS_VALUE_S, &i)) */
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Job status JSON can only be got from an object, value is of type %d", json_typeof (value_p));
		}

	return success_flag;
}


static bool CopyValidJSON (const json_t *src_p, const char *src_key_s, json_t **dest_pp)
{
	bool success_flag = true;
	json_t *value_p = json_object_get (src_p, src_key_s);


	if (value_p)
		{
			*dest_pp = json_deep_copy (value_p);

			if (!dest_pp)
				{
					success_flag = false;
				}
		}
	else
		{
			dest_pp = NULL;
		}

	return success_flag;
}


bool InitServiceJobFromJSON (ServiceJob *job_p, const json_t *job_json_p, Service *service_p, GrassrootsServer *grassroots_p)
{
	bool success_flag = false;

	if (job_json_p)
		{
			const char *service_name_s = GetJSONString (job_json_p, JOB_SERVICE_S);

			#if SERVICE_JOB_DEBUG >= STM_LEVEL_FINER
			PrintJSONToLog (STM_LEVEL_FINER, __FILE__, __LINE__, job_json_p, "Starting InitServiceJobFromJSON");
			#endif

			if (service_name_s)
				{
					const char *service_s = GetServiceName (service_p);

					if (strcmp (service_name_s, service_s) == 0)
						{
							const char *uuid_s = GetJSONString (job_json_p, JOB_UUID_S);

							if (uuid_s)
								{
									const char *job_name_s = GetJSONString (job_json_p, JOB_NAME_S);
									const char *job_description_s  = GetJSONString (job_json_p, JOB_DESCRIPTION_S);
									OperationStatus status;

									if (GetOperationStatusFromServiceJobJSON (job_json_p, &status))
										{
											const char *type_s = GetJSONString (job_json_p, JOB_TYPE_S);

											if (type_s)
												{
													uuid_t *id_p = NULL;
													uuid_t id = { 0 };



													if (uuid_s)
														{
															if (uuid_parse (uuid_s, id) == 0)
																{
																	id_p = &id;
																}
															else
																{
																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, job_json_p, "Couldn't parse uuid \"%s\"", uuid_s);
																}
														}


													if (InitServiceJob (job_p, service_p, job_name_s, job_description_s, NULL, NULL, NULL, id_p, type_s))
														{
															if (CopyValidJSON (job_json_p, JOB_RESULTS_S, & (job_p -> sj_result_p)))
																{
																	if (CopyValidJSON (job_json_p, JOB_METADATA_S, & (job_p -> sj_metadata_p)))
																		{
																			if (CopyValidJSON (job_json_p, JOB_ERRORS_S, & (job_p -> sj_errors_p)))
																				{
																					SetServiceJobStatus (job_p, status);


																					success_flag = true;
																				}
																			else
																				{
																					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, job_json_p, "Couldn't copy %s from job representation", JOB_ERRORS_S);
																				}
																		}
																	else
																		{
																			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, job_json_p, "Couldn't copy %s from job representation", JOB_METADATA_S);
																		}
																}
															else
																{
																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, job_json_p, "Couldn't copy %s from job representation", JOB_RESULTS_S);
																}

														}
													else
														{
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, job_json_p, "InitServiceJob failed for job \"%s\"", uuid_s);
														}

												}		/* if (type_s) */


										}		/* if (GetOperationStatusFromServiceJobJSON (job_json_p, &status)) */
									else
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, job_json_p, "Couldn't get status from JSON");
										}

								}		/* if (uuid_s) */
							else
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, job_json_p, "Couldn't get uuid from JSON");
								}

						}		/* if (strcmp (service_name_s, service_s) == 0) */
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, job_json_p, "Service name in json \"%s\" does not match instantiated service name \"%s\"", service_name_s, service_s);
						}


				}		/* if (service_name_s) */
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, job_json_p, "Couldn't get service name from JSON");
				}

		}		/* if (job_json_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Job JSON is NULL");
		}


#if SERVICE_JOB_DEBUG >= STM_LEVEL_FINER
	PrintJSONToLog (STM_LEVEL_FINER, __FILE__, __LINE__, job_json_p, "Ending InitServiceJobFromJSON, success = %d", success_flag);
#endif

	return success_flag;
}


ServiceJob *CreateServiceJobFromJSON (const json_t *job_json_p, GrassrootsServer *grassroots_p)
{
	ServiceJob *job_p = NULL;
	const char *service_name_s = GetJSONString (job_json_p, JOB_SERVICE_S);

	/* Check whether the job needs a custom deserialiser */
	if (service_name_s)
		{
			Service *service_p = GetServiceByName (grassroots_p, service_name_s, NULL);

			if (service_p)
				{
					bool add_job_flag = false;

					if (DoesServiceHaveCustomServiceJobSerialisation (service_p))
						{

							#if SERVICE_JOB_DEBUG >= STM_LEVEL_FINER
							PrintLog (STM_LEVEL_FINER, __FILE__, __LINE__, "CreateServiceJobFromJSON using custom deserialising for service \"%s\"", service_name_s);
							#endif

							job_p = CreateSerialisedServiceJobFromService (service_p, job_json_p);

							if (!job_p)
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, job_json_p, "Failed to create ServiceJob");
								}

						}		/* if (DoesServiceHaveCustomServiceJobSerialisation (service_p)) */
					else
						{
							job_p = AllocateEmptyServiceJob ();

							if (job_p)
								{
									if (!InitServiceJobFromJSON (job_p, job_json_p, service_p, grassroots_p))
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, job_json_p, "Failed to create ServiceJob with InitServiceJobFromJSON");
											FreeServiceJob (job_p);
											job_p = NULL;
										}
								}
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate empty ServiceJob");
								}
						}

					if (job_p)
						{

							/*
							 * Make sure that the ServiceJob's service parameter is set
							 * and if it differs from the service_p created earlier, then
							 * clean up service_p
							 */
							if (job_p -> sj_service_p)
								{
									if (job_p -> sj_service_p != service_p)
										{
											if (RemoveServiceJobFromService (job_p -> sj_service_p, job_p))
												{
													FreeService (service_p);
												}
											else
												{
													char uuid_s [UUID_STRING_BUFFER_SIZE];
													const char *name_s = GetServiceName (service_p);

													ConvertUUIDToString (job_p -> sj_id, uuid_s);

													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to remove job \"%s\" from service \"%s""", uuid_s, name_s);
												}

											service_p = job_p -> sj_service_p;
											add_job_flag = true;
										}
								}
							else
								{
									add_job_flag = true;
								}

						}		/* if (job_p) */




					if (add_job_flag)
						{
							if (!AddServiceJobToService (service_p, job_p))
								{
									char uuid_s [UUID_STRING_BUFFER_SIZE];

									ConvertUUIDToString (job_p -> sj_id, uuid_s);

									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add deserialised ServiceJob \"%s\" with id \"%s\" to Service \"%s\"", job_p -> sj_name_s, uuid_s, service_name_s);

									FreeServiceJob (job_p);
									job_p = NULL;
								}
						}		/* if (add_job_flag) */

				}		/* if (service_p) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get service with name \"%s\"", service_name_s);
				}

		}		/* if (service_name_s) */
	else
		{
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, job_json_p, "Failed to get service name from job JSON");
		}


	#if SERVICE_JOB_DEBUG >= STM_LEVEL_FINER
	PrintLog (STM_LEVEL_FINER, __FILE__, __LINE__, "CreateServiceJobFromJSON created job \"%s\" at %.16X", job_p -> sj_name_s, job_p);
	#endif

	return job_p;
}


json_t *GetServiceJobAsJSON (ServiceJob *job_p, bool omit_results_flag)
{
	json_t *job_json_p = json_object ();

	if (job_json_p)
		{
			bool success_flag = false;

			if (json_object_set_new (job_json_p, JOB_SERVICE_S, json_string (job_p -> sj_service_name_s)) == 0)
				{
					if (json_object_set_new (job_json_p, JOB_TYPE_S, json_string (job_p -> sj_type_s)) == 0)
						{
							if (AddValidJSON (job_json_p, JOB_ERRORS_S, job_p -> sj_errors_p, false))
								{
									if (AddValidJSON (job_json_p, JOB_METADATA_S, job_p -> sj_metadata_p, false))
										{
											if (AddStatusToServiceJobJSON (job_p, job_json_p))
												{
													char buffer_s [UUID_STRING_BUFFER_SIZE];

													ConvertUUIDToString (job_p -> sj_id, buffer_s);

													if (json_object_set_new (job_json_p, JOB_UUID_S, json_string (buffer_s)) == 0)
														{
															if (AddValidJSONString (job_json_p, JOB_NAME_S, job_p -> sj_name_s))
																{
																	if (AddValidJSONString (job_json_p, JOB_DESCRIPTION_S, job_p -> sj_description_s))
																		{
																			if (AddValidJSONString (job_json_p, JOB_URL_S, job_p -> sj_url_s))
																				{
																					if ((job_p -> sj_status == OS_SUCCEEDED) || (job_p -> sj_status == OS_PARTIALLY_SUCCEEDED))
																						{
																							/*
																							 * If this service has any linked services, fill in the data here
																							 */
																							if (omit_results_flag)
																								{
																									success_flag = (json_object_set_new (job_json_p, JOB_OMITTED_RESULTS_S, json_true ()) == 0);
																								}
																							else
																								{
																									if (AddValidJSON (job_json_p, JOB_RESULTS_S, job_p -> sj_result_p, false))
																										{
																											success_flag = true;
																											ProcessLinkedServices (job_p);

																											if (!AddLinkedServicesToServiceJobJSON (job_p, job_json_p))
																												{
																													PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to Linked Services for %s to json", job_p -> sj_name_s ? job_p -> sj_name_s : "unnamed job");
																												}
																										}		/* if (AddValidJSON (job_json_p, JOB_RESULTS_S, results_json_p, false)) */
																									else
																										{
																											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add results to service job json");
																										}
																								}
																						}
																					else
																						{
																							success_flag = true;
																						}
																				}		/* if (AddValidJSONString (job_json_p, JOB_URL_S, job_p -> sj_url_s)) */
																			else
																				{
																					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add url \"%s\" %s to json", job_p -> sj_url_s ? job_p -> sj_url_s : "");
																				}

																		}
																	else
																		{
																			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add description \"%s\" %s to json", job_p -> sj_description_s ? job_p -> sj_description_s : "");
																		}

																}		/* if (AddValidJSONString (job_json_p, JOB_NAME_S, job_p -> sj_name_s)) */
															else
																{
																	PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add name \"%s\" to json", job_p -> sj_name_s ? job_p -> sj_name_s : "");
																}

														}		/* if (json_object_set_new (job_json_p, JOB_UUID_S, json_string (buffer_s)) == 0) */
													else
														{
															PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add uuid %s to json", buffer_s);
														}

												}		/* if (AddStatusToServiceJobJSON (job_p, job_json_p)) */
											else
												{
													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add status to service job json");
												}

										}		/* if (AddValidJSON (job_json_p, JOB_METADATA_S, job_p -> sj_metadata_p, false)) */
									else
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add metadata to service job json");
										}

								}		/* if (AddValidJSON (job_json_p, JOB_ERRORS_S, job_p -> sj_errors_p, false)) */
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add errors to service job json");
								}

						}		/* if (json_object_set_new (job_json_p, JOB_TYPE_S, json_string (job_p -> sj_type_s)) == 0) */


				}		/* if (json_object_set_new (SERVICE_NAME_S, json_string (service_name_s)) == 0) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add service name \"%s\" to json", job_p -> sj_service_name_s);
				}


#if SERVICE_JOB_DEBUG >= STM_LEVEL_FINER
			PrintJSONToLog (STM_LEVEL_FINER, __FILE__, __LINE__, job_json_p, "service job: ");
			PrintLog (STM_LEVEL_FINER, __FILE__, __LINE__, "success_flag %d", success_flag);
#endif

			if (!success_flag)
				{
					json_decref (job_json_p);
					job_json_p = NULL;
				}
		}		/* if (job_json_p) */


	return job_json_p;
}


bool ProcessServiceJobSet (ServiceJobSet *jobs_p, json_t *res_p)
{
	bool success_flag = true;
	uint32 i = 0;
	ServiceJobNode *node_p = (ServiceJobNode *) (jobs_p -> sjs_jobs_p -> ll_head_p);

	while (node_p)
		{
			ServiceJob *job_p = node_p -> sjn_job_p;

			json_t *job_json_p = NULL;
			const OperationStatus job_status = GetServiceJobStatus (job_p);

#if SERVICE_JOB_DEBUG >= STM_LEVEL_FINE
			PrintLog (STM_LEVEL_FINE, __FILE__, __LINE__, "Job " UINT32_FMT ": status: %d", i, job_status);
#endif

			if ((job_status == OS_SUCCEEDED) || (job_status == OS_PARTIALLY_SUCCEEDED))
				{
					job_json_p = GetServiceJobAsJSON (job_p, false);
				}
			else
				{
					job_json_p = GetServiceJobStatusAsJSON (job_p, true);
				}

			if (job_json_p)
				{
#if SERVICE_JOB_DEBUG >= STM_LEVEL_FINE
					PrintJSONToLog (STM_LEVEL_FINE, __FILE__, __LINE__, job_json_p, "Job JSON");
#endif

#if SERVICE_JOB_DEBUG >= STM_LEVEL_FINER
					PrintJSONRefCounts (STM_LEVEL_FINE, __FILE__, __LINE__, job_json_p, "Job JSON");
#endif

					if (json_array_append_new (res_p, job_json_p) == 0)
						{
#if SERVICE_JOB_DEBUG >= STM_LEVEL_FINE
							PrintJSONRefCounts (STM_LEVEL_FINE, __FILE__, __LINE__, job_json_p, "after array adding, job json ");
#endif
						}
					else
						{
							char *uuid_s = GetUUIDAsString (job_p -> sj_id);

							if (uuid_s)
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add job results %s to json response", uuid_s);
									FreeCopiedString (uuid_s);
								}
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add job results %s to json response", job_p -> sj_name_s);
								}

							json_decref (job_json_p);
						}
				}
			else
				{
					char *uuid_s = GetUUIDAsString (job_p -> sj_id);

					if (uuid_s)
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to json response for json %s", uuid_s);
							FreeCopiedString (uuid_s);
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to json response for json %s", job_p -> sj_name_s);
						}
				}

			node_p = (ServiceJobNode *) (node_p -> sjn_node.ln_next_p);
			++ i;
		}		/* while (node_p) */


#if SERVICE_JOB_DEBUG >= STM_LEVEL_FINE
	PrintJSONToLog (STM_LEVEL_FINE, __FILE__, __LINE__, res_p, "service json: ");
	PrintLog (STM_LEVEL_FINE, __FILE__, __LINE__, "success_flag %s", success_flag ? "true" : "false");
#endif

	return success_flag;
}


OperationStatus GetCachedServiceJobStatus (const ServiceJob *job_p)
{
	return job_p -> sj_status;
}


void ProcessLinkedServices (ServiceJob *job_p)
{
	Service *service_p = job_p -> sj_service_p;

	if (service_p -> se_linked_services.ll_size)
		{
			LinkedServiceNode *linked_service_node_p = (LinkedServiceNode *) (service_p -> se_linked_services.ll_head_p);

			while (linked_service_node_p)
				{
					LinkedService *linked_service_p = linked_service_node_p -> lsn_linked_service_p;

					if (ProcessLinkedService (linked_service_p, job_p))
						{

						}
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, job_p -> sj_result_p, "ProcessLinkedService failed for input service %s to output service %s", GetServiceName (job_p -> sj_service_p), linked_service_p -> ls_output_service_s);
						}

					linked_service_node_p = (LinkedServiceNode *) (linked_service_node_p -> lsn_node.ln_next_p);
				}		/* while (linked_service_node_p) */

		}		/* if (service_p -> se_linked_services.ll_size) */
}

OperationStatus GetServiceJobStatus (ServiceJob *job_p)
{
	bool update_flag = false;

	/* If job has been started or is waiting to run, check its status */
	/*
	OS_LOWER_LIMIT = -4,
	OS_FAILED = -3,
	OS_FAILED_TO_START = -2,
	OS_ERROR = -1,
	OS_IDLE = 0,
	OS_PENDING,
	OS_STARTED,
	OS_FINISHED,
	OS_PARTIALLY_SUCCEEDED,
	OS_SUCCEEDED,
	OS_CLEANED_UP,
	OS_UPPER_LIMIT,
	OS_NUM_STATUSES = OS_UPPER_LIMIT - OS_LOWER_LIMIT + 1
	 */
	switch (job_p -> sj_status)
	{
	case OS_IDLE:
	case OS_PENDING:
	case OS_STARTED:
		update_flag = true;
		break;

	case OS_SUCCEEDED:
	case OS_PARTIALLY_SUCCEEDED:
		if (! (job_p -> sj_result_p))
			{
				update_flag = true;
			}
		break;

	default:
		break;
	}


	if (update_flag)
		{
			if (!UpdateServiceJob (job_p))
				{
					char uuid_s [UUID_STRING_BUFFER_SIZE];
					const char *service_name_s = GetServiceName (job_p -> sj_service_p);

					ConvertUUIDToString (job_p -> sj_id, uuid_s);

					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to update ServiceJob %s for service %s", uuid_s, service_name_s);
				}
		}

	return job_p -> sj_status;
}



json_t *GetServiceJobStatusAsJSON (ServiceJob *job_p, bool omit_results_flag)
{
	json_t *job_json_p = json_object ();

	if (job_json_p)
		{
			char *uuid_s = GetUUIDAsString (job_p -> sj_id);

			/* Make sure that the job's status is up to date */
			GetServiceJobStatus (job_p);

			if (uuid_s)
				{
					if (json_object_set_new (job_json_p, JOB_UUID_S, json_string (uuid_s)) == 0)
						{
#if SERVICE_JOB_DEBUG >= STM_LEVEL_FINER
							if (job_json_p)
								{
									PrintJSONToLog (STM_LEVEL_FINER, __FILE__, __LINE__, job_json_p, "Job after adding uuid:");
								}
#endif

							if (AddStatusToServiceJobJSON (job_p, job_json_p))
								{
									const char *service_name_s = GetServiceName (job_p -> sj_service_p);

#if SERVICE_JOB_DEBUG >= STM_LEVEL_FINER
									if (job_json_p)
										{
											PrintJSONToLog (STM_LEVEL_FINER, __FILE__, __LINE__, job_json_p, "Job after adding status:");
										}
#endif

									if (!AddValidJSONString (job_json_p, JOB_SERVICE_S, service_name_s))
										{
											PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to add service %s to job status json", service_name_s);
										}

#if SERVICE_JOB_DEBUG >= STM_LEVEL_FINER
									if (job_json_p)
										{
											PrintJSONToLog (STM_LEVEL_FINER, __FILE__, __LINE__, job_json_p, "Job after adding service_name_s:");
										}
#endif

									if (!AddValidJSONString (job_json_p, JOB_NAME_S, job_p -> sj_name_s))
										{
											PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to add job name %s to job status json", job_p -> sj_name_s);
										}

#if SERVICE_JOB_DEBUG >= STM_LEVEL_FINER
									if (job_json_p)
										{
											PrintJSONToLog (STM_LEVEL_FINER, __FILE__, __LINE__, job_json_p, "Job after adding job_p -> sj_name_s:");
										}
#endif
									if (!AddValidJSONString (job_json_p, JOB_DESCRIPTION_S, job_p -> sj_description_s))
										{
											PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to add job description %s to job status json", job_p -> sj_description_s);
										}

#if SERVICE_JOB_DEBUG >= STM_LEVEL_FINER
									if (job_json_p)
										{
											PrintJSONToLog (STM_LEVEL_FINER, __FILE__, __LINE__, job_json_p, "Job after adding job_p -> sj_description_s:");
										}
#endif
									if (!AddValidJSON (job_json_p, JOB_ERRORS_S, job_p -> sj_errors_p, false))
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add job errors to job %s, %s", job_p -> sj_name_s, uuid_s);
										}

#if SERVICE_JOB_DEBUG >= STM_LEVEL_FINER
									if (job_json_p)
										{
											PrintJSONToLog (STM_LEVEL_FINER, __FILE__, __LINE__, job_json_p, "Job after adding JOB_ERRORS_S:");
										}
#endif
								}		/* if (AddStatusToServiceJobJSON (job_p, job_json_p)) */
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to  add status for job uuid \"%s\" to json", uuid_s);
								}

						}		/* if (json_object_set_new (job_json_p, JOB_UUID_S, uuid_s) == 0) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to set job uuid \"%s\" as json  for job %s", uuid_s, job_p -> sj_name_s ? job_p -> sj_name_s : "");
						}

					FreeUUIDString (uuid_s);
				}		/* if (uuid_s) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get job uuid as string for job %s", job_p -> sj_name_s ? job_p -> sj_name_s : "");
				}

#if SERVICE_JOB_DEBUG >= STM_LEVEL_FINE
			if (job_json_p)
				{
					PrintJSONToLog (STM_LEVEL_FINE, __FILE__, __LINE__, job_json_p, "Job:");
				}
#endif

		}		/* if (json_p) */

	return job_json_p;
}



bool SetServiceJobFromJSON (ServiceJob *job_p, const json_t *json_p)
{
	bool success_flag = false;

	return success_flag;
}


json_t *GetServiceJobSetAsJSON (const ServiceJobSet *jobs_p, bool omit_results_flag)
{
	json_t *jobs_json_p = json_array ();

	if (jobs_json_p)
		{
			ServiceJobNode *node_p = (ServiceJobNode *) (jobs_p -> sjs_jobs_p -> ll_head_p);

			while (node_p)
				{
					ServiceJob *job_p = node_p -> sjn_job_p;
					json_t *job_json_p = GetServiceJobAsJSON (job_p, omit_results_flag);

					if (job_json_p)
						{
							if (json_array_append_new (jobs_json_p, job_json_p) != 0)
								{
									char *uuid_s = GetUUIDAsString (job_p -> sj_id);

									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to append job %s to results", uuid_s ? uuid_s : "");

									if (uuid_s)
										{
											FreeUUIDString (uuid_s);
										}
								}
						}
					else
						{
							char *uuid_s = GetUUIDAsString (job_p -> sj_id);

							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get job %s as json", uuid_s ? uuid_s : "");

							if (uuid_s)
								{
									FreeUUIDString (uuid_s);
								}
						}

					node_p = (ServiceJobNode *) (node_p -> sjn_node.ln_next_p);
				}		/* while (node_p) */
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate json jobset array");
		}

	return jobs_json_p;
}





char *SerialiseServiceJobToJSON (ServiceJob * const job_p, bool omit_results_flag)
{
	char *serialised_data_p = NULL;
	json_t *job_json_p = GetServiceJobAsJSON (job_p, omit_results_flag);

	if (job_json_p)
		{
			serialised_data_p = json_dumps (job_json_p, JSON_INDENT (2));
			json_decref (job_json_p);
		}		/* if (job_json_p) */

	return serialised_data_p;
}




ServiceJob *GetServiceJobFromServiceJobSet (const ServiceJobSet *jobs_p, const uint32 job_index)
{
	ServiceJob *job_p = NULL;

	if (job_index < (jobs_p -> sjs_jobs_p -> ll_size))
		{
			ListItem *node_p = jobs_p -> sjs_jobs_p -> ll_head_p;
			uint32 i;

			for (i = job_index; i > 0; -- i)
				{
					node_p = node_p -> ln_next_p;
				}

			job_p = ((ServiceJobNode *) node_p) -> sjn_job_p;
		}

	return job_p;
}


bool InitServiceJobFromResultsJSON (ServiceJob *job_p, const json_t *results_p, Service *service_p, const char *name_s, const char *description_s, OperationStatus status, const char *type_s)
{
	bool success_flag = false;
	bool (*update_status_fn) (ServiceJob *job_p) = NULL;
	bool (*calculate_results_fn) (struct ServiceJob *job_p) = NULL;
	void (*free_job_fn) (struct ServiceJob *job_p) = NULL;

	memset (job_p, 0, sizeof (*job_p));

	if (InitServiceJob (job_p, service_p, name_s, description_s, update_status_fn, calculate_results_fn, free_job_fn, NULL, type_s))
		{
			SetServiceJobStatus (job_p, status);

			if (results_p)
				{
					const char *value_s = GetJSONString (results_p, RESOURCE_PROTOCOL_S);

					if (value_s)
						{
							if (strcmp (value_s, PROTOCOL_INLINE_S) == 0)
								{
									json_t *data_p = json_object_get (results_p, RESOURCE_DATA_S);

									if (data_p)
										{
											json_t *results_array_p = json_array ();

											if (results_array_p)
												{
													char uuid_s [UUID_STRING_BUFFER_SIZE];
													char *title_s = NULL;
													json_t *resource_p = NULL;

													ConvertUUIDToString (job_p -> sj_id, uuid_s);

													title_s = ConcatenateVarargsStrings (job_p -> sj_name_s, " (", uuid_s, ")", NULL);

													resource_p = GetDataResourceAsJSONByParts (PROTOCOL_INLINE_S, NULL, title_s ? title_s : uuid_s, data_p);

													if (title_s)
														{
															FreeCopiedString (title_s);
														}

													if (resource_p)
														{
															if (json_array_append_new (results_array_p, resource_p) == 0)
																{
																	job_p -> sj_result_p = results_array_p;
																	success_flag = true;
																}		/* if (json_array_append_new (results_array_p, resource_p) == 0) */
															else
																{
																	json_decref (resource_p);
																}
														}

													if (! (job_p -> sj_result_p))
														{
															json_decref (results_array_p);
														}
												}

										}		/* if (data_p) */

								}
						}

				}		/* if (results_p) */
			else
				{
					success_flag = true;
				}

		}		/* if (InitServiceJob (job_p, service_p, name_s, description_s, update_status_fn, calculate_results_fn, free_job_fn, NULL, type_s)) */


	return success_flag;
}


ServiceJob *CreateServiceJobFromResultsJSON (const json_t *results_p, Service *service_p, const char *name_s, const char *description_s, OperationStatus status, const char *type_s)
{
	ServiceJob *job_p = (ServiceJob *) AllocMemory (sizeof (ServiceJob));

	if (job_p)
		{
			if (InitServiceJobFromResultsJSON (job_p, results_p, service_p, name_s, description_s, status, type_s))
				{
					return job_p;
				}

			FreeMemory (job_p);
		}		/* if (job_p) */

	return NULL;
}


bool AddGeneralErrorMessageToServiceJob (ServiceJob *job_p, const char * const error_s)
{
	return AddParameterErrorMessageToServiceJob (job_p, NULL, PT_NUM_TYPES, error_s);
}


bool AddParameterErrorMessageToServiceJob (ServiceJob *job_p, const char * const param_s, const ParameterType param_type, const char * const value_s)
{
	json_t *value_p = json_string (value_s);

	if (value_p)
		{
			if (AddCompoundErrorToServiceJob (job_p, param_s, param_type, value_p))
				{
					return true;
				}
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, value_p, "AddCompoundErrorToServiceJob failed for job %s", job_p -> sj_name_s);
				}

			json_decref (value_p);
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate JSON error object");
		}

	return false;
}


bool AddTabularParameterErrorMessageToServiceJob (ServiceJob *job_p, const char * const param_s, const ParameterType param_type, const char * const value_s, const uint32 row, const char *column_s)
{
	json_t *error_p = json_object ();

	if (error_p)
		{
			if (SetJSONInteger (error_p, TABLE_PARAM_ROW_S, row))
				{
					if ((column_s == NULL) || (SetJSONString (error_p, TABLE_PARAM_COLUMN_S, column_s)))
						{
							if (SetJSONString (error_p, JOB_ERROR_S, value_s))
								{
									if (AddCompoundErrorToServiceJob (job_p, param_s, param_type, error_p))
										{
											return true;
										}
									else
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, error_p, "AddCompoundErrorToServiceJob failed for job %s", job_p -> sj_name_s);
										}
								}
							else
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, error_p, "Failed to set \"%s\": \"%s\" JSON error object", JOB_ERROR_S, value_s);
								}

						}
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, error_p, "Failed to set \"%s\": \"%s\" JSON error object", TABLE_PARAM_COLUMN_S, column_s);
						}

				}
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, error_p, "Failed to set \"%s\":" UINT32_FMT " JSON error object", TABLE_PARAM_ROW_S, row);
				}

			json_decref (error_p);
		}		/* if (error_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate JSON error object");
		}

	return false;
}




bool AddCompoundErrorToServiceJob (ServiceJob *job_p, const char *param_s, const ParameterType param_type, json_t *error_details_p)
{
	json_t *param_errors_p = NULL;

	if (param_s)
		{
			param_errors_p = json_object_get (job_p -> sj_errors_p, param_s);

			if (!param_errors_p)
				{
					param_errors_p = CreateAndAddErrorObjectForParameter (job_p -> sj_errors_p, param_s, param_type, true);
				}
		}
	else
		{
			param_errors_p = json_object_get (job_p -> sj_errors_p, JOB_RUNTIME_ERRORS_S);

			if (!param_errors_p)
				{
					param_errors_p = CreateAndAddErrorObjectForParameter (job_p -> sj_errors_p, JOB_RUNTIME_ERRORS_S, PT_NUM_TYPES, false);
				}
		}

	if (param_errors_p)
		{
			json_t *errors_array_p = json_object_get (param_errors_p, JOB_ERRORS_S);

			if (errors_array_p)
				{
					if (json_array_append_new (errors_array_p, error_details_p) == 0)
						{
							return true;
						}
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, error_details_p, "Failed to add error message to service job \"%s\"", job_p -> sj_name_s);
						}
				}
		}

	return false;
}


bool UpdateServiceJob (ServiceJob *job_p)
{
	bool success_flag = true;

	/*
	 * Make sure we are not in a recursive loop
	 * calling UpdateServiceJob
	 */
	if (! (job_p -> sj_is_updating_flag))
		{
			if (job_p -> sj_update_fn)
				{
					job_p -> sj_is_updating_flag = true;

					success_flag = job_p -> sj_update_fn (job_p);

					job_p -> sj_is_updating_flag = false;
				}
		}

	return success_flag;
}


bool CalculateServiceJobResult (ServiceJob *job_p)
{
	bool success_flag = false;

	if (job_p -> sj_calculate_result_fn)
		{
			success_flag = job_p -> sj_calculate_result_fn (job_p);
		}

	return success_flag;
}


void SetServiceJobUpdateFunction (ServiceJob *job_p, bool (*update_fn) (ServiceJob *job_p))
{
	job_p -> sj_update_fn = update_fn;
}


void SetServiceJobCalculateResultFunction (ServiceJob *job_p, bool (*calculate_fn) (ServiceJob *job_p))
{
	job_p -> sj_calculate_result_fn = calculate_fn;
}


void SetServiceJobFreeFunction (ServiceJob *job_p, void (*free_fn) (ServiceJob *job_p))
{
	job_p -> sj_free_fn = free_fn;
}



bool AddResultToServiceJob (ServiceJob *job_p, json_t *result_p)
{
	return AddResultEntryToServiceJob (job_p, & (job_p -> sj_result_p), result_p);
}


bool ReplaceServiceJobResults (ServiceJob *job_p, json_t *results_p)
{
	bool success_flag = false;

	if (results_p)
		{
			if (json_is_array (results_p))
				{
					if (job_p -> sj_result_p)
						{
							json_decref (job_p -> sj_result_p);
						}

					job_p -> sj_result_p = results_p;
					success_flag = true;

				}		/* if (json_is_array (results_p)) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "New ServiceJob results is not an array %d", json_typeof (results_p));
				}

		}		/* if (results_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "New ServiceJob results is NULL");
		}

	return success_flag;
}



bool AddLinkedServiceToServiceJob (ServiceJob *job_p, LinkedService *linked_service_p)
{
	bool success_flag = false;
	json_t *linked_service_json_p = GetLinkedServiceAsJSON (linked_service_p);

	if (linked_service_json_p)
		{
			if (json_array_append_new (job_p -> sj_linked_services_p, linked_service_json_p) == 0)
				{
					success_flag = true;
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add linked service json object to job \"%s\"", job_p -> sj_name_s);
				}
		}		/* if (linked_service_json_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create linked service json objectjob \"%s\"", job_p -> sj_name_s);
		}

	return success_flag;
}


void SetServiceJobUUID (ServiceJob *job_p, const uuid_t new_job_id)
{
	uuid_copy (job_p -> sj_id, new_job_id);
}


static bool AddResultEntryToServiceJob (ServiceJob *job_p, json_t **results_pp, json_t *result_to_add_p)
{
	bool success_flag = false;
	json_t *results_p = *results_pp;

	if (!results_p)
		{
			results_p = json_array ();

			if (results_p)
				{
					*results_pp = results_p;
				}
			else
				{
					char uuid_s [UUID_STRING_BUFFER_SIZE];

					ConvertUUIDToString (job_p -> sj_id, uuid_s);
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate results array for %s", uuid_s);
				}

		}		/* if (! (job_p -> sj_result_p)) */

	if (results_p)
		{
			if (json_array_append_new (results_p, result_to_add_p) == 0)
				{
					success_flag = true;
				}
			else
				{
					char uuid_s [UUID_STRING_BUFFER_SIZE];

					ConvertUUIDToString (job_p -> sj_id, uuid_s);
					PrintJSONToErrors(STM_LEVEL_SEVERE, __FILE__, __LINE__, result_to_add_p, "Failed to add result to %s", uuid_s);
				}

		}		/* if (job_p -> sj_result_p) */

	return success_flag;
}


int32 GetNumberOfLiveJobsFromServiceJobSet (const ServiceJobSet *jobs_p)
{
	int32 num_live_jobs = 0;
	ServiceJobNode *node_p = (ServiceJobNode *) (jobs_p -> sjs_jobs_p -> ll_head_p);

	while (node_p)
		{
			ServiceJob *job_p = node_p -> sjn_job_p;

			switch (job_p -> sj_status)
				{
					case OS_IDLE:
					case OS_PENDING:
					case OS_STARTED:
					case OS_SUCCEEDED:
					case OS_FINISHED:
						++ num_live_jobs;
						break;

					default:
						break;
				}

			node_p = (ServiceJobNode *) (node_p -> sjn_node.ln_next_p);
		}		/* while (node_p) */

	return num_live_jobs;
}



static json_t *CreateAndAddErrorObjectForParameter (json_t *root_p, const char *param_s, const ParameterType param_type, const bool add_type_flag)
{
	json_t *error_obj_p = json_object ();

	if (error_obj_p)
		{
			json_t *errors_array_p = json_array ();

			if (errors_array_p)
				{
					if (json_object_set_new (error_obj_p, JOB_ERRORS_S, errors_array_p) == 0)
						{
							bool success_flag = false;

							if (add_type_flag)
								{
									const char *type_s = GetGrassrootsTypeAsString (param_type);

									if (type_s)
										{
											if (SetJSONString (error_obj_p, PARAM_GRASSROOTS_TYPE_INFO_TEXT_S, type_s))
												{
													success_flag = true;
												}
										}
								}
							else
								{
									success_flag = true;
								}

							if (success_flag)
								{
									if (json_object_set_new (root_p, param_s, error_obj_p) == 0)
										{
											return error_obj_p;
										}
								}

						}		/* if (json_object_set_new (error_obj_p, JOB_ERRORS_S, errors_array_p) == 0) */
					else
						{
							json_decref (errors_array_p);
						}

				}		/* if (errors_array_p) */

			json_decref (error_obj_p);
		}		/* if (error_obj_p) */

	return NULL;
}

