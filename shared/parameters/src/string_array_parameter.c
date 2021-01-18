


#include "string_array_parameter.h"
#include "string_utils.h"


static char **CopyStringsArray (const char **src_ss);

static void FreeStringsArray (char **values_ss);

static void ClearStringArrayParameter (Parameter *param_p);


//
//GRASSROOTS_PARAMS_API StringArrayParameter *AllocateStringArrayParameter (const struct ServiceData *service_data_p, const ParameterType pt, const char * const name_s, const char * const display_name_s, const char * const description_s, const char *default_value_p, const char *current_value_p, ParameterLevel level);
//
//
//GRASSROOTS_PARAMS_API StringArrayParameter *AllocateStringArrayParameterFromJSON (const json_t *param_json_p, const struct Service *service_p, const bool concise_flag);
//
//
//GRASSROOTS_PARAMS_API Parameter *EasyCreateAndAddStringArrayParameterToParameterSet (const struct ServiceData *service_data_p, ParameterSet *params_p, ParameterGroup *group_p, ParameterType type,
//																											const char * const name_s, const char * const display_name_s, const char * const description_s,
//																											const char *default_value_s, uint8 level);
//
//
//GRASSROOTS_PARAMS_API Parameter *CreateAndAddStringArrayParameterToParameterSet (const struct ServiceData *service_data_p, ParameterSet *params_p, ParameterGroup *group_p, ParameterType type,
//																											const char * const name_s, const char * const display_name_s, const char * const description_s,
//																											const char *default_value_s, const char *current_value_s, uint8 level);
//
//
//



Parameter *EasyCreateAndAddStringParameterToParameterSet (const ServiceData *service_data_p, ParameterSet *params_p, ParameterGroup *group_p, ParameterType type,
																											const char * const name_s, const char * const display_name_s, const char * const description_s,
																											const char *default_value_s, uint8 level)
{
	return CreateAndAddStringParameterToParameterSet (service_data_p, params_p, group_p, type,
																										name_s, display_name_s, description_s,
																										default_value_s, default_value_s, level);
}

Parameter *CreateAndAddStringParameterToParameterSet (const ServiceData *service_data_p, ParameterSet *params_p, ParameterGroup *group_p, ParameterType type,
																											const char * const name_s, const char * const display_name_s, const char * const description_s,
																											const char *default_value_s, const char *current_value_s, uint8 level)
{
	Parameter *base_param_p = NULL;
	StringParameter *string_param_p = AllocateStringParameter (service_data_p, type, name_s, display_name_s, description_s, default_value_s, current_value_s, level);

	if (string_param_p)
		{
			base_param_p = & (string_param_p -> sp_base_param);

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
					FreeStringsArray (param_p -> sp_current_values_ss);
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
					FreeStringsArray (param_p -> sp_default_values_ss);
				}

			param_p -> sp_default_values_ss = copied_values_ss;
			success_flag = true;
		}

	return success_flag;
}



/*
 * STATIC DEFINITIONS
 */

static void FreeStringsArray (char **values_ss)
{
	char **value_ss = values_ss;

	while (*value_ss)
		{
			FreeCopiedString (*value_ss);
			++ value_ss;
		}

	FreeMemory (values_ss);
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
					FreeStringsArray (dest_ss);
				}

		}		/* if (dest_ss) */

	return NULL;
}




static void ClearStringArrayParameter (Parameter *param_p)
{
	StringArrayParameter *string_array_param_p = (StringArrayParameter *) param_p;

	if (string_array_param_p -> sp_current_values_ss)
		{
			FreeStringsArray (string_array_param_p -> sp_current_values_ss);
		}


	if (string_array_param_p -> sp_default_values_ss)
		{
			FreeStringsArray (string_array_param_p -> sp_default_values_ss);
		}
}
