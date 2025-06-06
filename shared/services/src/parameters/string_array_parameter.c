


#include "string_array_parameter.h"
#include "string_utils.h"
#include "string_parameter.h"


/*
 * STATIC DECLARATIONS
 */

static void ClearStringArrayParameter (Parameter *param_p);


static StringArrayParameter *GetNewStringArrayParameter (char **current_values_ss, char **default_values_ss, const size_t num_values);

static bool SetStringArrayParameterValue (char ***param_value_sss, const size_t num_existing_value, char **new_values_ss, const size_t num_new_values);

static bool AddStringArrayParameterDetailsToJSON (const Parameter *param_p, json_t *param_json_p, const bool full_definition_flag);

static Parameter *CloneStringArrayParameter (const Parameter *param_p, const ServiceData *service_data_p);


static bool SetStringArrayParameterCurrentValueFromString (Parameter *param_p, const char *value_s);

static bool AddNonTrivialStringArrayValuesToJSON (char **values_ss, const size_t num_values, json_t *param_json_p, const char *key_s);


/*
 * API DEFINITIONS
 */


StringArrayParameter *AllocateStringArrayParameter (const struct ServiceData *service_data_p,
																					const char * const name_s, const char * const display_name_s,
																					const char * const description_s,
																					char **default_values_ss, char **current_values_ss,
																					const size_t num_values,
																					ParameterLevel level)
{
	StringArrayParameter *param_p = GetNewStringArrayParameter (current_values_ss, default_values_ss, num_values);

	if (param_p)
		{
			if (InitParameter (& (param_p -> sap_base_param), service_data_p, PT_STRING_ARRAY, name_s, display_name_s, description_s, level,
												 ClearStringArrayParameter, AddStringArrayParameterDetailsToJSON,
												 CloneStringArrayParameter, SetStringArrayParameterCurrentValueFromString))
				{
					if (service_data_p)
						{
//									GetParameterDefaultValueFromConfig (service_data_p, name_s, param_p -> pa_type, &default_value);
						}

					return param_p;
				}

			ClearParameter (& (param_p -> sap_base_param));
		}

	return NULL;
}





StringArrayParameter *AllocateStringArrayParameterFromJSON (const json_t *param_json_p, const struct Service *service_p, const bool concise_flag, const ParameterType *pt_p)
{
	StringArrayParameter *param_p = NULL;
	char **current_value_ss = NULL;
	char **default_value_ss = NULL;
	size_t num_values = 0;

	const json_t *value_p = json_object_get (param_json_p, PARAM_CURRENT_VALUE_S);

	if (value_p)
		{
			if (json_is_array (value_p))
				{
					current_value_ss = GetStringArrayFromJSON (value_p, true);
					num_values = json_array_size (value_p);
				}
		}


	if (!concise_flag)
		{
			value_p = json_object_get (param_json_p, PARAM_DEFAULT_VALUE_S);

			if (value_p)
				{
					if (json_is_array (value_p))
						{
							default_value_ss = GetStringArrayFromJSON (value_p, true);

							if (num_values == 0)
								{
									num_values = json_array_size (value_p);
								}
						}
				}
		}

	param_p = GetNewStringArrayParameter (current_value_ss, default_value_ss, num_values);

	if (current_value_ss)
		{
			FreeStringArray (current_value_ss, num_values);
		}

	if (default_value_ss)
		{
			FreeStringArray (default_value_ss, num_values);
		}


	if (param_p)
		{
			Parameter *base_param_p = & (param_p -> sap_base_param);
			ParameterType pt = pt_p ? *pt_p : PT_STRING_ARRAY;

			if (InitParameterFromJSON (base_param_p, param_json_p, service_p, concise_flag, &pt))
				{
					if (GetStringParameterOptionsFromJSON (base_param_p, param_json_p))
						{
							SetParameterCallbacks (base_param_p, ClearStringArrayParameter, AddStringArrayParameterDetailsToJSON,
																		 CloneStringArrayParameter, SetStringArrayParameterCurrentValueFromString);

							return param_p;

						}

				}
			else
				{
					ClearParameter (& (param_p -> sap_base_param));
					FreeMemory (param_p);
				}
		}

	return NULL;
}


Parameter *EasyCreateAndAddStringArrayParameterToParameterSet (const ServiceData *service_data_p, ParameterSet *params_p, ParameterGroup *group_p,
																											const char * const name_s, const char * const display_name_s, const char * const description_s,
																											char **default_values_ss, uint32 num_entries, ParameterLevel level)
{
	return CreateAndAddStringArrayParameterToParameterSet (service_data_p, params_p, group_p,
																										name_s, display_name_s, description_s,
																										default_values_ss, default_values_ss, num_entries, level);
}

Parameter *CreateAndAddStringArrayParameterToParameterSet (const ServiceData *service_data_p, ParameterSet *params_p, ParameterGroup *group_p,
																											const char * const name_s, const char * const display_name_s, const char * const description_s,
																											char **default_values_ss, char **current_values_ss, uint32 num_entries, ParameterLevel level)
{
	Parameter *base_param_p = NULL;
	StringArrayParameter *string_array_param_p = AllocateStringArrayParameter (service_data_p, name_s, display_name_s, description_s, default_values_ss, current_values_ss, num_entries, level);

	if (string_array_param_p)
		{
			base_param_p = & (string_array_param_p -> sap_base_param);

			if (group_p)
				{
					/*
					 * If the parameter fails to get added to the group, it's
					 * not a terminal error so still carry on
					 */
					if (!AddParameterToParameterGroup (group_p, base_param_p))
						{
							PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to add param \"%s\" to group \"%s\"", name_s, group_p -> pg_name_s);
						}
				}

			if (!AddParameterToParameterSet (params_p, base_param_p))
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add param \"%s\" to set \"%s\"", name_s, params_p -> ps_name_s);
					FreeParameter (base_param_p);
					base_param_p = NULL;
				}

		}		/* if (param_p) */

	return base_param_p;
}


const char **GetStringArrayParameterCurrentValues (const StringArrayParameter *param_p)
{
	return (const char **) (param_p -> sap_current_values_ss);
}


const char *GetStringArrayParameterCurrentValueAtIndex (const StringArrayParameter *param_p, const size_t index)
{
	if (index < param_p -> sap_num_values)
		{
			return * ((param_p -> sap_current_values_ss) + index);
		}

	return NULL;
}




bool SetStringArrayParameterCurrentValues (StringArrayParameter *param_p, char **values_ss, const size_t num_values)
{
	bool success_flag = false;
	char **copied_values_ss = CopyStringArray (values_ss, num_values);

	if (copied_values_ss)
		{
			if (param_p -> sap_current_values_ss)
				{
					FreeStringArray (param_p -> sap_current_values_ss, param_p -> sap_num_values);
				}

			param_p -> sap_current_values_ss = copied_values_ss;
			param_p -> sap_num_values = num_values;
			success_flag = true;
		}

	return success_flag;
}


const char **GetStringArrayParameterDefaultValues (const StringArrayParameter *param_p)
{
	return (const char **) (param_p -> sap_default_values_ss);
}


bool SetStringArrayParameterDefaultValues (StringArrayParameter *param_p, char **values_ss, const size_t num_values)
{
	bool success_flag = false;
	char **copied_values_ss = CopyStringArray (values_ss, num_values);

	if (copied_values_ss)
		{
			if (param_p -> sap_default_values_ss)
				{
					FreeStringArray (param_p -> sap_default_values_ss, param_p -> sap_num_values);
				}

			param_p -> sap_default_values_ss = copied_values_ss;
			param_p -> sap_num_values = num_values;
			success_flag = true;
		}

	return success_flag;
}


bool IsStringArrayParameter (const Parameter *param_p)
{
	bool match_flag = false;

	if (param_p -> pa_type == PT_STRING_ARRAY)
		{
			match_flag = true;
		}

	return match_flag;
}


size_t GetNumberOfStringArrayCurrentParameterValues (const StringArrayParameter *param_p)
{
	return (param_p -> sap_num_values);
}


char *GetStringArrayParameterCurrentValuesAsFlattenedString (const StringArrayParameter *param_p)
{
	char *flattened_s = NULL;

	if (param_p -> sap_current_values_ss)
		{
			ByteBuffer *buffer_p = AllocateByteBuffer (1024);

			if (buffer_p)
				{
					bool loop_flag = true;
					bool success_flag = true;
					char **value_pp = param_p -> sap_current_values_ss;
					const char *delimiter_s = "\n";

					while (loop_flag && success_flag)
						{

							if (*value_pp)
								{
									if (AppendStringsToByteBuffer (buffer_p, *value_pp, delimiter_s, NULL))
										{
											++ value_pp;
										}
									else
										{
											success_flag = false;
										}
								}
							else
								{
									loop_flag = false;
								}
						}

					if (success_flag)
						{
							flattened_s = DetachByteBufferData (buffer_p);
						}
					else
						{
							FreeByteBuffer (buffer_p);
						}

				}		/* if (byte_buffer_p) */

		}


	return flattened_s;
}



const char **GetStringArrayValuesForParameter (ParameterSet *param_set_p, const char *param_s, size_t *num_entries_p)
{
	const char **values_ss = NULL;
	Parameter *param_p = GetParameterFromParameterSetByName (param_set_p, param_s);

	if (param_p)
		{
			if (IsStringArrayParameter (param_p))
				{
					StringArrayParameter *sa_param_p = (StringArrayParameter *) param_p;

					values_ss = GetStringArrayParameterCurrentValues (sa_param_p);
					*num_entries_p = GetNumberOfStringArrayCurrentParameterValues (sa_param_p);
				}
			else if (IsStringParameter (param_p))
				{
					StringParameter *st_param_p = (StringParameter *) param_p;
					const char *value_s = GetStringParameterCurrentValue (st_param_p);

					values_ss = &value_s;
					*num_entries_p = 1;									
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Unknown ParameterType for \"%s\" %d", param_s, param_p -> pa_type);
				}
	
		}
	else
		{
			*num_entries_p = 0;
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "No Parameter named \"%s\"", param_s);
		}

	return values_ss;
}


bool GetCurrentStringArrayParameterValuesFromParameterSet (const ParameterSet * const params_p, const char * const name_s, const char ***values_ppp, size_t *num_entries_p)
{
	bool success_flag = false;
	Parameter *param_p = GetParameterFromParameterSetByName (params_p, name_s);

	if (param_p)
		{
			if (IsStringArrayParameter (param_p))
				{
					const StringArrayParameter *string_array_param_p = (const StringArrayParameter *) param_p;

					*values_ppp = GetStringArrayParameterCurrentValues (string_array_param_p);
					*num_entries_p = GetNumberOfStringArrayCurrentParameterValues (string_array_param_p);

					success_flag = true;
				}
		}

	return success_flag;
}


/*
 * STATIC DEFINITIONS
 */

static StringArrayParameter *GetNewStringArrayParameter (char **current_values_ss, char **default_values_ss, const size_t num_values)
{
	StringArrayParameter *param_p = (StringArrayParameter *) AllocMemory (sizeof (StringArrayParameter));

	if (param_p)
		{
			param_p -> sap_current_values_ss = NULL;
			param_p -> sap_default_values_ss = NULL;

			NullifyParameter (& (param_p -> sap_base_param));


			if (SetStringArrayParameterValue (& (param_p -> sap_current_values_ss), 0, current_values_ss, num_values))
				{
					if (SetStringArrayParameterValue (& (param_p -> sap_default_values_ss), 0, default_values_ss, num_values))
						{
							param_p -> sap_num_values = num_values;

							return param_p;
						}

					if (param_p -> sap_current_values_ss)
						{
							FreeStringArray (param_p -> sap_current_values_ss, num_values);
						}
				}

			FreeMemory (param_p);
		}

	return NULL;
}



static Parameter *CloneStringArrayParameter (const Parameter *param_p, const ServiceData *service_data_p)
{
	return NULL;
}


static bool SetStringArrayParameterCurrentValueFromString (Parameter *param_p, const char *value_s)
{
	bool success_flag = false;
	StringArrayParameter *string_param_p = (StringArrayParameter *) param_p;

	LinkedList *lines_p = ParseStringToStringLinkedList (value_s, "\n", false);

	if (lines_p)
		{
			const size_t num_values = lines_p -> ll_size;
			char **values_ss = AllocMemoryArray (num_values, sizeof (char *));

			if (values_ss)
				{
					StringListNode *node_p = (StringListNode *) (lines_p -> ll_head_p);
					char **value_pp = values_ss;

					while (node_p)
						{
							*value_pp = DetachStringFromStringListNode (node_p);

							node_p = (StringListNode *) (node_p -> sln_node.ln_next_p);
							++ value_pp;
						}

					if (SetStringArrayParameterCurrentValues (string_param_p, values_ss, num_values))
						{
							success_flag = true;
						}
					else
						{
							FreeStringArray (values_ss, num_values);
						}

				}

			FreeLinkedList (lines_p);
		}

	return success_flag;
}





static bool SetStringArrayParameterValue (char ***param_value_sss, const size_t num_existing_values, char **new_values_ss, const size_t num_new_values)
{
	bool success_flag = false;

	if (new_values_ss)
		{
			char **copied_values_ss = CopyStringArray (new_values_ss, num_new_values);

			if (copied_values_ss)
				{
					if (*param_value_sss)
						{
							FreeStringArray (*param_value_sss, num_existing_values);
						}

					*param_value_sss = copied_values_ss;
					success_flag = true;
				}
		}
	else
		{
			if (*param_value_sss)
				{
					FreeStringArray (*param_value_sss, num_existing_values);
					*param_value_sss = NULL;
				}

			success_flag = true;
		}

	return success_flag;
}



static bool AddNonTrivialStringArrayValuesToJSON (char **values_ss, const size_t num_values, json_t *param_json_p, const char *key_s)
{
	bool success_flag = false;

	if (values_ss)
		{
			json_t *values_json_p = ConvertStringArrayToJSON (values_ss, num_values);

			if (values_json_p)
				{
					if (json_object_set_new (param_json_p, key_s, values_json_p) == 0)
						{
							success_flag = true;
						}
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, param_json_p, "Failed to add \"%s\" key", key_s);
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, values_json_p, "with value:");
							json_decref (values_json_p);
						}
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, " ConvertTimeArrayToJSON () Failed for \"%s\" key", key_s);
				}
		}
	else
		{
			if (json_object_set_new (param_json_p, key_s, json_null ()) == 0)
				{
					success_flag = true;
				}
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, param_json_p, "Failed to add \"%s\": null", key_s);
				}
		}

	return success_flag;
}


static bool AddStringArrayParameterDetailsToJSON (const Parameter *param_p, json_t *param_json_p, const bool full_definition_flag)
{
	bool success_flag = false;
	StringArrayParameter *string_array_param_p = (StringArrayParameter *) param_p;

	if (AddNonTrivialStringArrayValuesToJSON (string_array_param_p -> sap_current_values_ss, string_array_param_p -> sap_num_values, param_json_p, PARAM_CURRENT_VALUE_S))
		{
			if (full_definition_flag)
				{
					if (AddNonTrivialStringArrayValuesToJSON (string_array_param_p -> sap_default_values_ss, string_array_param_p -> sap_num_values, param_json_p, PARAM_DEFAULT_VALUE_S))
						{
							if (AddStringParameterOptionsToJSON (param_p, param_json_p))
								{
									success_flag = true;
								}
						}		/* if (AddNonTrivialStringArrayValuesToJSON (string_array_param_p -> sp_default_values_ss, param_json_p, PARAM_DEFAULT_VALUE_S)) */

				}		/* if (full_definition_flag) */
			else
				{
					success_flag = true;
				}
		}		/* if (AddNonTrivialStringArrayValuesToJSON (string_array_param_p -> sp_current_values_ss, param_json_p, PARAM_CURRENT_VALUE_S)) */

	return success_flag;
}


static void ClearStringArrayParameter (Parameter *param_p)
{
	StringArrayParameter *string_array_param_p = (StringArrayParameter *) param_p;

	if (string_array_param_p -> sap_current_values_ss)
		{
			FreeStringArray (string_array_param_p -> sap_current_values_ss, string_array_param_p -> sap_num_values);
		}


	if (string_array_param_p -> sap_default_values_ss)
		{
			FreeStringArray (string_array_param_p -> sap_default_values_ss, string_array_param_p -> sap_num_values);
		}
}

