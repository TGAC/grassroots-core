


#include "string_array_parameter.h"
#include "string_utils.h"


/*
 * STATIC DECLARATIONS
 */
static char **CopyStringsArray (const char **src_ss);

static void ClearStringArrayParameter (Parameter *param_p);

static StringArrayParameter *GetNewStringArrayParameter (const char **current_values_ss, const char **default_values_ss)

static bool SetStringArrayParameterValue (char **param_value_ss, const char **new_values_ss);

static bool AddStringArrayParameterDetailsToJSON (const Parameter *param_p, json_t *param_json_p, const bool full_definition_flag);

static Parameter *CloneStringArrayParameter (const Parameter *param_p, const ServiceData *service_data_p);


/*
 * API DEFINITIONS
 */

//
//GRASSROOTS_PARAMS_API StringArrayParameter *AllocateStringArrayParameter (const struct ServiceData *service_data_p, const ParameterType pt, const char * const name_s, const char * const display_name_s, const char * const description_s, const char *default_value_p, const char *current_value_p, ParameterLevel level);
//
StringArrayParameter *AllocateStringArrayParameter (const struct ServiceData *service_data_p, const ParameterType pt,
																					const char * const name_s, const char * const display_name_s,
																					const char * const description_s,
																					const char **default_values_ss, const char **current_values_ss,
																					ParameterLevel level)
{
	StringArrayParameter *param_p = GetNewStringArrayParameter (current_values_ss, default_values_ss);

	if (param_p)
		{
			if (InitParameter (& (param_p -> sap_base_param), service_data_p, pt, name_s, display_name_s, description_s, level,
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





//GRASSROOTS_PARAMS_API StringArrayParameter *AllocateStringArrayParameterFromJSON (const json_t *param_json_p, const struct Service *service_p, const bool concise_flag);
//
//



Parameter *EasyCreateAndAddStringArrayParameterToParameterSet (const ServiceData *service_data_p, ParameterSet *params_p, ParameterGroup *group_p, ParameterType type,
																											const char * const name_s, const char * const display_name_s, const char * const description_s,
																											const char **default_values_ss, uint8 level)
{
	return CreateAndAddStringArrayParameterToParameterSet (service_data_p, params_p, group_p, type,
																										name_s, display_name_s, description_s,
																										default_values_ss, default_values_ss, level);
}

Parameter *CreateAndAddStringArrayParameterToParameterSet (const ServiceData *service_data_p, ParameterSet *params_p, ParameterGroup *group_p, ParameterType type,
																											const char * const name_s, const char * const display_name_s, const char * const description_s,
																											const char **default_values_ss, const char **current_values_ss, uint8 level)
{
	Parameter *base_param_p = NULL;
	StringArrayParameter *string_array_param_p = AllocateStringArrayParameter (service_data_p, type, name_s, display_name_s, description_s, default_values_ss, current_values_ss, level);

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
	return param_p -> sp_current_values_ss;
}


bool SetStringArrayParameterCurrentValues (StringArrayParameter *param_p, const char **values_ss)
{
	bool success_flag = false;
	char **copied_values_ss = CopyStringsArray (values_ss);

	if (copied_values_ss)
		{
			if (param_p -> sp_current_values_ss)
				{
					FreeStringArray (param_p -> sp_current_values_ss);
				}

			param_p -> sp_current_values_ss = copied_values_ss;
			success_flag = true;
		}

	return success_flag;
}


const char **GetStringArrayParameterDefaulttValues (const StringArrayParameter *param_p)
{
	return param_p -> sp_default_values_ss;
}


bool SetStringArrayParameterDefaultValues (StringArrayParameter *param_p, const char **values_ss)
{
	bool success_flag = false;
	char **copied_values_ss = CopyStringsArray (values_ss);

	if (copied_values_ss)
		{
			if (param_p -> sp_default_values_ss)
				{
					FreeStringArray (param_p -> sp_default_values_ss);
				}

			param_p -> sp_default_values_ss = copied_values_ss;
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



/*
 * STATIC DEFINITIONS
 */

static StringArrayParameter *GetNewStringArrayParameter (const char **current_values_ss, const char **default_values_ss)
{
	StringArrayParameter *param_p = (StringArrayParameter *) AllocMemory (sizeof (StringParameter));

	if (param_p)
		{
			param_p -> sp_current_values_ss = NULL;
			param_p -> sp_default_values_ss = NULL;

			if (SetStringArrayParameterValue (& (param_p -> sp_current_values_ss), current_values_ss))
				{
					if (SetStringParameterValue (& (param_p -> sp_default_values_ss), default_values_ss))
						{
							return param_p;
						}

					if (param_p -> sp_current_values_ss)
						{
							FreeStringArray (param_p -> sp_current_values_ss);
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
			char **values_ss = AllocMemoryArray ((lines_p -> ll_size) + 1, sizeof (char *));

			if (values_ss)
				{
					StringListNode *node_p = (StringListNode *) (lines_p -> ll_head_p);
					char **value_pp = values_ss;

					while (node_p)
						{
							*value_pp = DetachStringFromStringListNode (node_p);;

							node_p = (StringListNode *) (node_p -> sln_node.ln_next_p);
							++ value_pp;
						}

					if (SetStringArrayParameterCurrentValue (string_param_p, values_ss))
						{
							success_flag = true;
						}
					else
						{
							FreeStringArray (values_ss);
						}

				}

			FreeLinkedList (lines_p);
		}

	return success_flag;
}



static char **CopyStringsArray (const char **src_ss)
{
	uint32 num_src_values = 1;		/* for the final NULL entry */
	const char **src_pp = src_ss;
	char **dest_ss = NULL;

	while (*src_pp)
		{
			++ src_pp;
			++ num_src_values;
		}

	dest_ss = (char **) AllocMemoryArray (num_src_values, sizeof (char *));

	if (dest_ss)
		{
			char **dest_pp = dest_ss;
			bool success_flag = true;

			src_pp = src_ss;


			while (success_flag && (*src_pp))
				{
					char *dest_s = EasyCopyToNewString (*src_pp);

					if (dest_s)
						{
							*dest_pp = dest_s;
							++ dest_pp;
							++ src_pp;
						}
					else
						{
							success_flag = false;
						}
				}

			if (success_flag)
				{
					return dest_ss;
				}
			else
				{
					FreeStringArray (dest_ss);
				}

		}		/* if (dest_ss) */

	return NULL;
}



static bool SetStringArrayParameterValue (char **param_value_ss, const char **new_values_ss)
{
	bool success_flag = false;

	if (new_values_ss)
		{
			char *copied_values_ss = CopyStringsArray (new_values_ss);

			if (copied_values_ss)
				{
					if (*param_value_ss)
						{
							FreeStringArray (*param_value_ss);
						}

					*param_value_ss = copied_values_ss;
					success_flag = true;
				}
		}
	else
		{
			if (*param_value_ss)
				{
					FreeStringArray (*param_value_ss);
					*param_value_ss = NULL;
				}

			success_flag = true;
		}

	return success_flag;
}


static bool AddStringArrayParameterDetailsToJSON (const Parameter *param_p, json_t *param_json_p, const bool full_definition_flag)
{
	bool success_flag = false;

	return success_flag;
}


static void ClearStringArrayParameter (Parameter *param_p)
{
	StringArrayParameter *string_array_param_p = (StringArrayParameter *) param_p;

	if (string_array_param_p -> sp_current_values_ss)
		{
			FreeStringArray (string_array_param_p -> sp_current_values_ss);
		}


	if (string_array_param_p -> sp_default_values_ss)
		{
			FreeStringArray (string_array_param_p -> sp_default_values_ss);
		}
}
