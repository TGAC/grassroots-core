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

#include "json_tools.h"
#include "request_tools.h"
#include "grassroots_server.h"
#include "json_util.h"
#include "streams.h"
#include "string_utils.h"
#include "schema_version.h"
#include "filesystem_utils.h"
#include "providers_state_table.h"

#include "uuid_defs.h"
#include "uuid_util.h"



#ifdef _DEBUG
#define JSON_TOOLS_DEBUG	(STM_LEVEL_INFO)
#else
#define JSON_TOOLS_DEBUG	(STM_LEVEL_NONE)
#endif


static json_t *LoadConfig (const char *path_s);

static bool AddKeyAndStringValue (json_t *json_p, const char * const key_s, const char * const value_s);

static json_t *GetServicesInfoRequest (const uuid_t **ids_pp, const uint32 num_ids, OperationStatus status, Connection *connection_p, const SchemaVersion * const sv_p);

static bool AddContexts (json_t *data_p);

static json_t *GetGenericNamedServicesRequest (const UserDetails *user_p, const Operation op, const char * const service_names_s, const SchemaVersion * const sv_p);



void WipeJSON (json_t *json_p)
{
	if (json_p)
		{
			if (json_p -> refcount == 1)
				{
					if (json_is_array (json_p))
						{
							json_array_clear (json_p);
						}
					else if (json_is_object (json_p))
						{
							json_object_clear (json_p);
						}
				}

			json_decref (json_p);
		}
}


json_t *MakeRemoteJsonCall (json_t *req_p, Connection *connection_p)
{
	json_t *response_p = NULL;
	const char *data_s = MakeRemoteJsonCallViaConnection (connection_p, req_p);

	if (data_s)
		{
			json_error_t err;

#if JSON_TOOLS_DEBUG >= STM_LEVEL_FINER
			PrintLog (STM_LEVEL_FINER, __FILE__, __LINE__, "MakeRemoteJsonCall >\n%s\n", data_s);
			FlushLog ();
#endif

			response_p = json_loads (data_s, 0, &err);

			if (!response_p)
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "error decoding response: \"%s\"\n\"%s\"\n%d %d %d\n%s\n", err.text, err.source, err.line, err.column, err.position, data_s);
				}
		}

	return response_p;
}


json_t *GetInitialisedMessage (const SchemaVersion * const sv_p)
{
	json_t *message_p = json_object ();

	if (message_p)
		{
			json_t *header_p = json_object ();

			if (header_p)
				{
					if (json_object_set_new (message_p, HEADER_S, header_p) == 0)
						{
							json_t *schema_p = GetSchemaVersionAsJSON (sv_p);

							if (schema_p)
								{
									if (json_object_set_new (header_p, SCHEMA_S, schema_p) == 0)
										{
											if (AddContexts (message_p))
												{
													return message_p;
												}
										}
									else
										{
											json_decref (schema_p);
										}
								}
						}
					else
						{
							json_decref (header_p);
						}

				}		/* if (header_p) */

			json_decref (message_p);
		}		/* if (message_p) */

	return NULL;
}


static bool AddContexts (json_t *data_p)
{
	bool success_flag = false;
	json_t *context_p = json_object ();

	if (context_p)
		{
			if (json_object_set_new (context_p, CONTEXT_PREFIX_SCHEMA_ORG_S, json_string (CONTEXT_URL_SCHEMA_ORG_S)) == 0)
				{
					if (json_object_set_new (context_p, CONTEXT_PREFIX_EDAM_ONTOLOGY_S, json_string (CONTEXT_URL_EDAM_ONOTOLOGY_S)) == 0)
						{
							if (json_object_set_new (context_p, CONTEXT_PREFIX_EXPERIMENTAL_FACTOR_ONTOLOGY_S, json_string (CONTEXT_URL_EXPERIMENTAL_FACTOR_ONOTOLOGY_S)) == 0)
								{
									if (json_object_set_new (context_p, CONTEXT_PREFIX_SOFTWARE_ONTOLOGY_S, json_string (CONTEXT_URL_SOFTWARE_ONOTOLOGY_S)) == 0)
										{
											if (json_object_set_new (data_p, "@context", context_p) == 0)
												{
													success_flag = true;
												}

										}		/* if (json_object_set_new (context_p, CONTEXT_PREFIX_SOFTWARE_ONTOLOGY_S, json_string (CONTEXT_URL_SOFTWARE_ONOTOLOGY_S)) == 0) */

								}		/* if (json_object_set_new (context_p, CONTEXT_PREFIX_EXPERIMENTAL_FACTOR_ONTOLOGY_S, json_string (CONTEXT_URL_EXPERIMENTAL_FACTOR_ONOTOLOGY_S)) == 0) */

						}		/* if (json_object_set_new (context_p, CONTEXT_PREFIX_SCHEMA_ORG_S, json_string (CONTEXT_URL_SCHEMA_ORG_S)) == 0) */

				}		/* if (json_object_set_new (context_p, SCHEMA_ORG_PREFIX_S, json_string (SCHEMA_ORG_URL_S)) == 0) */


			if (!success_flag)
				{
					json_decref (context_p);
				}

		}		/* if (context_p) */

	return success_flag;
}


/*
 {
	credentials:
		{
			dropbox:
				{
					token_key:    onr78fxbbne0gzfy,
					token_secret: bnr6bfxwgy995su
				}
		}
}
 * */

bool AddClientConfigToJSON (json_t *root_p, const UserDetails *user_p)
{
	bool success_flag = false;
	char *home_s = GetHomeDirectory ();

	if (home_s)
		{
			char *config_path_s = MakeFilename (home_s, ".grassroots");

			if (config_path_s)
				{
					json_t *config_p = LoadConfig (config_path_s);

					if (config_p)
						{
							if (json_object_set (root_p, CONFIG_S, config_p) == 0)
								{
									success_flag = true;
								}

							json_decref (config_p);
						}		/* if (config_p) */

					FreeCopiedString (config_path_s);
				}		/* if (config_path_s) */

			FreeCopiedString (home_s);
		}


	#if JSON_TOOLS_DEBUG >= STM_LEVEL_FINE
		PrintJSONToLog (STM_LEVEL_FINE, __FILE__, __LINE__, root_p, "root_p: ");
	#endif

	return success_flag;
}


static json_t *LoadConfig (const char *path_s)
{
	json_error_t error;
	json_t *config_json_p = json_load_file (path_s, 0, &error);	

#if JSON_TOOLS_DEBUG >= STM_LEVEL_FINE
	char *value_s = json_dumps (config_json_p, JSON_INDENT (2));
#endif

#if JSON_TOOLS_DEBUG >= STM_LEVEL_FINE
	free (value_s);
#endif

	return config_json_p;
}




static bool AddKeyAndStringValue (json_t *json_p, const char * const key_s, const char * const value_s)
{
	bool success_flag = false;
	json_t *value_p = json_string (value_s);

	if (value_p)
		{
			if (json_object_set_new (json_p, key_s, value_p) == 0)
				{
					success_flag = true;
				}
			else
				{
					json_decref (value_p);
				}
		}

	return success_flag;
}


json_t *GetAvailableServicesRequest (const UserDetails * UNUSED_PARAM (user_p), const SchemaVersion * const sv_p)
{	
	json_t *op_p = GetOperationAsJSON (OP_LIST_ALL_SERVICES, sv_p);

	if (op_p)
		{
			return op_p;
		}		/* if (op_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get JSON for OP_LIST_ALL_SERVICES");
		}

	return NULL;
}


json_t *GetInterestedServicesRequest (const UserDetails *user_p, const char * const protocol_s, const char * const filename_s, const SchemaVersion * const sv_p)
{
	json_t *res_p = NULL;
	json_error_t error;
	json_t *op_data_p = json_pack_ex (&error, 0, "{s:s, s:s}", RESOURCE_PROTOCOL_S, protocol_s, RESOURCE_VALUE_S, filename_s);

	if (op_data_p)
		{
			res_p = GetServicesRequest (user_p, OP_LIST_INTERESTED_SERVICES, RESOURCE_S, op_data_p, sv_p);
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "GetInterestedServicesRequest failed for %s:%s", protocol_s, filename_s);
		}

	return res_p;
}



json_t *GetNamedServicesIndexingDataRequest (const UserDetails *user_p, const char * const service_names_s, const SchemaVersion * const sv_p)
{
	return GetGenericNamedServicesRequest (user_p, OP_GET_SERVICE_INFO, service_names_s, sv_p);

}


json_t *GetNamedServicesRequest (const UserDetails *user_p, const char * const service_names_s, const SchemaVersion * const sv_p)
{
	return GetGenericNamedServicesRequest (user_p, OP_GET_NAMED_SERVICES, service_names_s, sv_p);
}



static json_t *GetGenericNamedServicesRequest (const UserDetails *user_p, const Operation op, const char * const service_names_s, const SchemaVersion * const sv_p)
{
	json_t *res_p = NULL;
	LinkedList *service_names_list_p = ParseStringToStringLinkedList (service_names_s, ",", false);

	if (service_names_list_p)
		{
			json_t *service_names_p = json_array ();

			if (service_names_p)
				{
					StringListNode *node_p = (StringListNode *) (service_names_list_p -> ll_head_p);
					bool success_flag = true;

					while (node_p && success_flag)
						{
							json_t *service_p = json_object ();

							if (service_p)
								{
									const char * const service_name_s = node_p -> sln_string_s;

									if (SetJSONString (service_p, SERVICE_NAME_S, service_name_s))
										{
											if (SetJSONString (service_p, SERVICE_ALIAS_S, service_name_s))
												{
													if (json_array_append_new (service_names_p, service_p) == 0)
														{
															node_p = (StringListNode *) (node_p -> sln_node.ln_next_p);
														}
													else
														{
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, service_p, "Failed to append service request to array");
															success_flag = false;
														}
												}
											else
												{
													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to set \"%s\": \"%s\" for JSON object", SERVICE_ALIAS_S, service_name_s);
													success_flag = false;
												}
										}
									else
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to set \"%s\": \"%s\" for JSON object", SERVICE_NAME_S, service_name_s);
											success_flag = false;
										}

									if (!success_flag)
										{
											json_decref (service_p);
										}
								}
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate JSON object for service");
									success_flag = false;
								}

						}		/* while (node_p && success_flag) */

					if (success_flag)
						{
							res_p = GetServicesRequest (user_p, op, SERVICES_NAME_S, service_names_p, sv_p);

							if (!res_p)
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "GetServicesRequest failed for %s", service_names_s);
								}

						}		/* if (success_flag) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get list of services from \"%s\"", service_names_s);
						}

					json_decref (service_names_p);
				}		/* if (service_names_p) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create services array for %s", service_names_s);
				}

			FreeLinkedList (service_names_list_p);
		}		/* if (service_names_list_p) */

	return res_p;
}



json_t *GetServicesRequest (const UserDetails *user_p, const Operation op, const char * const op_key_s, json_t * const op_data_p, const SchemaVersion * const sv_p)
{
	json_t *root_p = GetOperationAsJSON (op, sv_p);

	if (root_p)
		{
			bool success_flag = true;

			if (user_p)
				{
					if (!AddClientConfigToJSON (root_p, user_p))
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add user details");
						}		/* if (!AddCredentialsToJson (root_p, user_p)) */

				}		/* if (user_p) */

			if (success_flag)
				{
					if (json_object_set (root_p, op_key_s, op_data_p) == 0)
						{
							return root_p;
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to set %s", op_key_s);
						}
				}

			json_decref (root_p);
		}

	return NULL;
}


bool GetUsernameAndPassword (const UserDetails * const user_p, const char *provider_s, const char **username_ss, const char **password_ss)
{
	bool success_flag = false;


	return success_flag;
}


Operation GetOperationFromTopLevelJSON (const json_t * const json_p)
{
	Operation op = OP_NONE;
	const json_t *server_op_p = json_object_get (json_p, SERVER_OPERATION_S);

	if (!server_op_p)
		{
			server_op_p = json_object_get (json_p, SERVER_OPERATIONS_S);
		}

	if (server_op_p)
		{
			const char *op_s = GetJSONString (server_op_p, OPERATION_S);

			if (op_s)
				{
					op = GetOperationFromString (op_s);
				}		/* if (op_s) */
			else
				{
					json_int_t value;

					if (GetJSONInteger (server_op_p, OPERATION_S, &value))
						{
							if ((value >= OP_NONE) && (value <= OP_NUM_OPERATIONS))
								{
									op = (Operation) value;
								}
							else
								{
									PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Operation value out of range %d", value);
								}
						}
					else
						{
							PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to get operation from \"%s\"", op_s);
						}
				}

		}		/* if (server_op_p) */
	else
		{
			PrintJSONToErrors (STM_LEVEL_FINE, __FILE__, __LINE__, json_p, "Failed to get operation");
		}

	return op;
}


json_t *GetOperationAsJSON (Operation op, const SchemaVersion * const sv_p)
{
	json_t *msg_p = GetInitialisedMessage (sv_p);

	if (msg_p)
		{
			json_t *op_p = json_object ();

			if (op_p)
				{
					const char *op_s = GetOperationAsString (op);

					if (op_s)
						{
							if (json_object_set_new (op_p, OPERATION_S, json_string (op_s)) == 0)
								{
									if (json_object_set_new (msg_p, SERVER_OPERATIONS_S, op_p) == 0)
										{
											return msg_p;
										}		/* if (json_object_set_new (msg_p, SERVER_OPERATIONS_S, op_p) == 0) */
									else
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, op_p, "Failed to add ops");
										}

								}		/* if (json_object_set (op_p, OPERATION_ID_S, json_integer (op)) == 0) */
							else
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, op_p, "Failed to add op id");
								}

						}		/* if (op_s) */
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, op_p, "Failed to get op string for " UINT32_FMT, op);
						}

					json_decref (op_p);
				}		/* if (op_p) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create op for %d", op);
				}

			json_decref (msg_p);
		}		/* if (msg_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create message for %d", op);
		}

	return NULL;
}


json_t *CallServices (json_t *client_params_json_p, const UserDetails *user_p, Connection *connection_p)
{
	json_t *services_json_p = NULL;

	if (client_params_json_p)
		{
			json_t *new_req_p = NULL;

			/* Is the request already wrapped? */
			if (json_object_get (client_params_json_p, SERVICES_NAME_S))
				{
					new_req_p = client_params_json_p;
				}
			else
				{
					/* We need to wrap the request */
					new_req_p = json_object ();

					if (new_req_p)
						{
							if (json_object_set (new_req_p, SERVICES_NAME_S, client_params_json_p) != 0)
								{
									json_decref (new_req_p);
									new_req_p = NULL;
								}
						}
				}

			if (new_req_p)
				{
					if (!AddClientConfigToJSON (new_req_p, user_p))
						{
							PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "failed to add credentials to request");
						}

					#if JSON_TOOLS_DEBUG >= STM_LEVEL_FINER
					PrintJSONToLog (STM_LEVEL_FINER, __FILE__, __LINE__, new_req_p, "Client sending: ");
					#endif

					services_json_p = MakeRemoteJsonCall (new_req_p, connection_p);

					if (services_json_p)
						{
							#if JSON_TOOLS_DEBUG >= STM_LEVEL_FINER
							PrintJSONToLog (STM_LEVEL_FINER, __FILE__, __LINE__, services_json_p, "Client received: ");
							#endif
						}
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, new_req_p, "Empty response after client sent: ");
						}

					if (new_req_p != client_params_json_p)
						{
							json_decref (new_req_p);
						}

				}		/* if (new_req_p) */
		}
	else
		{
			PrintErrors (STM_LEVEL_INFO, __FILE__, __LINE__, "no results from client");
		}

	return services_json_p;
}


const char *GetUserUUIDStringFromJSON (const json_t *credentials_p)
{
	return GetJSONString (credentials_p, CREDENTIALS_UUID_S);
}


json_t *GetServicesResultsRequest (const uuid_t **ids_pp, const uint32 num_ids, Connection *connection_p, const SchemaVersion * const sv_p)
{
	return GetServicesInfoRequest (ids_pp, num_ids, OP_GET_SERVICE_RESULTS, connection_p, sv_p);
}



static json_t *GetServicesInfoRequest (const uuid_t **ids_pp, const uint32 num_ids, OperationStatus status, Connection * UNUSED_PARAM (connection_p), const SchemaVersion * const sv_p)
{
	json_error_t error;
	json_t *req_p = json_pack_ex (&error, 0, "{s:{s:i}}", SERVER_OPERATIONS_S, SERVER_OPERATION_S, status);

	if (req_p)
		{
			json_t *services_p = json_array ();

			if (services_p)
				{
					if (json_object_set_new (req_p, SERVICES_NAME_S, services_p) == 0)
						{
							uint32 i = num_ids;
							const uuid_t **id_pp = ids_pp;
							bool success_flag = true;

							while ((i > 0) && success_flag)
								{
									if (*id_pp)
										{
											char *uuid_s = GetUUIDAsString (**id_pp);

											if (uuid_s)
												{
													if (json_array_append_new (services_p, json_string (uuid_s)) == 0)
														{
															-- i;
															++ id_pp;
														}
													else
														{
															success_flag = false;
														}

													FreeUUIDString (uuid_s);
												}
											else
												{
													success_flag = false;
												}

										}		/* if (*id_pp) */
									else
										{
											PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "UUID is NULL");
											success_flag = false;
										}
								}		/* while ((i > 0) && success_flag) */

							if (success_flag)
								{
									return req_p;
								}		/* if (success_flag) */

						}		/* if (json_object_set_new- (req_p, SERVICES_NAME_S, services_p) == 0) */
					else
						{
							json_decref (services_p);
						}

				}		/* if (services_p) */

			json_object_clear (req_p);
			json_decref (req_p);
		}		/* if (req_p) */

	return NULL;
}



bool GetStatusFromJSON (const json_t *service_json_p, OperationStatus *status_p)
{
	bool success_flag = false;
	json_t *status_json_p = json_object_get (service_json_p, SERVICE_STATUS_VALUE_S);

	if (status_json_p)
		{
			if (json_is_integer (status_json_p))
				{
					int i = json_integer_value (status_json_p);

					if ((i > OS_LOWER_LIMIT) && (i < OS_UPPER_LIMIT))
						{
							*status_p = (OperationStatus) i;

							success_flag = true;
						}		/* if ((i > OS_LOWER_LIMIT) && (i < OS_UPPER_LIMIT)) */

				}		/* if (json_is_integer (status_p)) */

		}		/* if (status_p) */

	return success_flag;
}



bool GetUUIDFromJSON (const json_t *service_json_p, uuid_t uuid)
{
	bool success_flag = false;
	json_t *uuid_json_p = json_object_get (service_json_p, SERVICE_UUID_S);

	if (uuid_json_p)
		{
			if (json_is_string (uuid_json_p))
				{
					const char *uuid_s = json_string_value (uuid_json_p);

					if (uuid_parse (uuid_s, uuid) == 0)
						{
							success_flag = true;
						}
				}
		}

	return success_flag;
}


const char *GetServiceDescriptionFromJSON (const json_t * const root_p)
{
	return GetJSONString (root_p, SERVICE_DESCRIPTION_S);
}


const char *GetServiceAliasFromJSON (const json_t * const root_p)
{
	return GetJSONString (root_p, SERVICE_ALIAS_S);
}


const char *GetServiceNameFromJSON (const json_t * const root_p)
{
	const char *name_s = GetJSONString (root_p, SERVICES_NAME_S);

	if (!name_s)
		{
			name_s = GetJSONString (root_p, SERVICE_NAME_S);
		}

	return name_s;
}


const char *GetOperationDescriptionFromJSON (const json_t * const root_p)
{
	return GetJSONString (root_p, OPERATION_DESCRIPTION_S);
}


const char *GetOperationNameFromJSON (const json_t * const root_p)
{
	const char *result_s = GetJSONString (root_p, OPERATION_ID_S);

	if (!result_s)
		{
			result_s = GetJSONString (root_p, OPERATION_ID_OLD_S);
		}

	return result_s;
}


const char *GetOperationInformationURIFromJSON (const json_t * const root_p)
{
	return GetJSONString (root_p, OPERATION_INFORMATION_URI_S);
}


const char *GetOperationIconURIFromJSON (const json_t * const root_p)
{
	return GetJSONString (root_p, OPERATION_ICON_URI_S);
}


const json_t *GetProviderFromServiceJSON (const json_t *service_json_p)
{
	json_t *provider_p = json_object_get (service_json_p, SERVER_PROVIDER_S);

	if (!provider_p)
		{
			provider_p = json_object_get (service_json_p, SERVER_MULTIPLE_PROVIDERS_S);
		}

	return provider_p;

}



void UpdateStatuses (const uuid_t **ids_pp, const size_t size, Connection *connection_p, const SchemaVersion *schema_p)
{
	json_t *req_p = GetServicesResultsRequest (ids_pp, size, connection_p, schema_p);

	if (req_p)
		{
			json_t *results_json_p = MakeRemoteJsonCall (req_p, connection_p);

			if (results_json_p)
				{
					json_t *services_json_p = json_object_get (results_json_p, SERVICES_NAME_S);

					if (services_json_p)
						{
							if (json_is_array (services_json_p))
								{
									const size_t num_services = json_array_size (services_json_p);
									size_t i;
									json_t *service_json_p;

									json_array_foreach (services_json_p, i, service_json_p)
										{
											const char *uuid_s = GetJSONString (service_json_p, SERVICE_UUID_S);

											if (uuid_s)
												{
												uuid_t uuid = { 0 };

													if (uuid_parse (uuid_s, uuid) == 0)
														{
															size_t j;
															json_t *job_p;

															json_array_foreach (results_json_p, j, job_p)
																{
																	const char *service_name_s = GetJSONString (job_p, SERVICE_NAME_S);
																	const char *service_description_s = GetJSONString (job_p, OPERATION_DESCRIPTION_S);
																	const char *service_uri_s =  GetJSONString (job_p, OPERATION_INFORMATION_URI_S);

																	/* Get the job status */
																	OperationStatus status = OS_ERROR;
																	const char *value_s = GetJSONString (job_p, SERVICE_STATUS_S);

																	if (value_s)
																		{
																			status = GetOperationStatusFromString (value_s);
																		}
																	else
																		{
																			json_int_t k;
																			/* Get the job status */

																			if (GetJSONInteger(job_p, SERVICE_STATUS_VALUE_S, &k))
																				{
																					if ((k > OS_LOWER_LIMIT) && (k < OS_UPPER_LIMIT))
																						{
																							status = (OperationStatus) k;
																						}
																				}
																		}

																	if ((status == OS_SUCCEEDED) || (status == OS_PARTIALLY_SUCCEEDED))
																		{

																		}		/* if ((status == OS_SUCCEEDED) || (status == OS_PARTIALLY_SUCCEEDED)) */

																}		/* json_array_foreach (results_json_p, j, job_p) */

														}		/* if (uuid_parse (uuid_s, uuid) == 0) */

												}		/* if (uuid_s) */

										}		/* for (size_t i = 0; i < num_services; ++ i) */

								}		/* if (json_is_array (services_json_p)) */


						}		/* if (services_json_p) */

				}		/* if (results_json_p) */

		}		/* if (req_p) */


}



json_t *GetIndexingDataPayload (GrassrootsServer *grassroots_p, const char *service_s, json_t *params_array_json_p)
{
	json_t *payload_p = json_object ();

	if (payload_p)
		{
			json_t *services_p = json_array ();

			if (services_p)
				{
					if (json_object_set_new (payload_p, SERVICES_NAME_S, services_p) == 0)
						{
							json_t *service_p  = json_object ();

							if (service_p)
								{
									if (json_array_append_new (services_p, service_p) == 0)
										{
											if (SetJSONString (service_p, SERVICE_NAME_S, service_s))
												{
													if (SetJSONBoolean (service_p, SERVICE_REFRESH_S, true))
														{
															json_t *param_set_p = json_object ();

															if (param_set_p)
																{
																	if (json_object_set_new (service_p, PARAM_SET_KEY_S, param_set_p) == 0)
																		{
																			if (SetJSONString (param_set_p, PARAM_LEVEL_S, PARAM_LEVEL_TEXT_SIMPLE_S))
																				{
																					if (json_object_set_new (param_set_p, PARAM_SET_PARAMS_S, params_array_json_p) == 0)
																						{
																							return payload_p;

																						}		/* if (json_object_set_new (param_set_p, PARAM_SET_PARAMS_S, params_p) == 0) */
																					else
																						{
																							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, payload_p, "Failed to add params array to param set \"%s\"", service_s);
																						}

																				}		/* if (SetJSONString (param_set_p, PARAM_LEVEL_S, PARAM_LEVEL_TEXT_SIMPLE_S)) */
																			else
																				{
																					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, param_set_p, "Failed to add \"%s\": \"%s\"", PARAM_LEVEL_S, PARAM_LEVEL_TEXT_SIMPLE_S);
																				}

																		}		/* if (json_object_set_new (service_p, PARAM_SET_KEY_S, param_set_p) == 0) */
																	else
																		{
																			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, payload_p, "Failed to add parameter set to service \"%s\"", service_s);
																		}

																}		/* if (param_set_p) */
															else
																{
																	PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate param set  for \"%s\"", service_s);
																}

														}		/* if (SetJSONBoolean (services_p, SERVICE_REFRESH_S, true)) */
													else
														{
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, services_p, "Failed to add \"%s\": true", SERVICE_REFRESH_S);
														}

												}		/* if (SetJSONString (services_p, SERVICE_NAME_S, service_s)) */
											else
												{
													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, services_p, "Failed to add \"%s\": \"%s\"", SERVICE_NAME_S, service_s);
												}

										}		/* if (json_array_append_new (services_p, service_p)) == 0) */
									else
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, payload_p, "Failed to add service to services array \"%s\"", service_s);
										}

								}		/* if (service_p) */
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate service for \"%s\"", service_s);
								}

						}		/* if (json_object_set_new (payload_p, SERVICES_NAME_S, services_p) == 0) */
					else
						{
							json_decref (services_p);
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, payload_p, "Failed to add services to payload \"%s\"", service_s);
						}

				}		/* if (services_p) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate services for \"%s\"", service_s);
				}

			json_decref (payload_p);
		}		/* if (payload_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate payload for \"%s\"", service_s);
		}


	return NULL;
}


