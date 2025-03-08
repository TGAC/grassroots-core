/*
 * time_array_parameter.c
 *
 *  Created on: 18 Feb 2023
 *      Author: billy
 */





#include "time_array_parameter.h"
#include "string_utils.h"
#include "time_util.h"


/*
 * STATIC DECLARATIONS
 */
static struct tm **CopyTimeArray (const struct tm **src_array_pp, const size_t num_values);

static void ClearTimeArrayParameter (Parameter *param_p);

static TimeArrayParameter *GetNewTimeArrayParameter (const struct tm **current_values_pp, const struct tm **default_values_pp, const size_t num_values);

static bool SetTimeArrayParameterValue (struct tm ***param_values_ppp, const size_t num_existing_values, const struct tm **new_values_pp, const size_t num_new_values);

static bool AddTimeArrayParameterDetailsToJSON (const Parameter *param_p, json_t *param_json_p, const bool full_definition_flag);

static Parameter *CloneTimeArrayParameter (const Parameter *param_p, const ServiceData *service_data_p);


static bool SetTimeArrayParameterCurrentValueFromString (Parameter *param_p, const char *value_s);


static json_t *ConvertTimeArrayToJSON (struct tm **times_pp, const size_t num_values);

static struct tm **GetTimeArrayFromJSON (const json_t *times_json_p);

static bool SetTimeArrayParameterValues (const struct tm **src_values_pp, struct tm ***dest_ppp, const size_t num_values);


static bool AddNonTrivialTimeArrayValuesToJSON (struct tm **times_pp, json_t *param_json_p, const char *key_s, const uint32 num_values);

/*
 * API DEFINITIONS
 */



TimeArrayParameter *AllocateTimeArrayParameter (const struct ServiceData *service_data_p,  const char * const name_s,
																								const char * const display_name_s, const char * const description_s,
																								struct tm **default_values_pp, struct tm **current_values_pp,
																								const size_t num_values, ParameterLevel level)
{
	TimeArrayParameter *param_p = GetNewTimeArrayParameter (current_values_pp, default_values_pp, num_values);

	if (param_p)
		{
			if (InitParameter (& (param_p -> tap_base_param), service_data_p, PT_TIME_ARRAY, name_s, display_name_s, description_s, level,
												 ClearTimeArrayParameter, AddTimeArrayParameterDetailsToJSON,
												 CloneTimeArrayParameter, SetTimeArrayParameterCurrentValueFromString))
				{
					if (service_data_p)
						{
//									GetParameterDefaultValueFromConfig (service_data_p, name_s, param_p -> pa_type, &default_value);
						}

					return param_p;
				}

			ClearParameter (& (param_p -> tap_base_param));
		}

	return NULL;
}





TimeArrayParameter *AllocateTimeArrayParameterFromJSON (const json_t *param_json_p, const struct Service *service_p, const bool concise_flag, const ParameterType *pt_p)
{
	TimeArrayParameter *param_p = NULL;
	struct tm **current_values_pp = NULL;
	struct tm **default_values_pp = NULL;
	size_t num_values = 0;

	const json_t *value_p = json_object_get (param_json_p, PARAM_CURRENT_VALUE_S);

	if (value_p)
		{
			if (json_is_array (value_p))
				{
					num_values = json_array_size (value_p);

					if (num_values > 0)
						{
							current_values_pp = GetTimeArrayFromJSON (value_p);
						}
				}
		}


	if (!concise_flag)
		{
			value_p = json_object_get (param_json_p, PARAM_DEFAULT_VALUE_S);

			if (value_p)
				{
					if (json_is_array (value_p))
						{
							size_t t = json_array_size (value_p);

							if (num_values == 0)
								{
									num_values = t;
								}

							if (t == num_values)
								{
									default_values_pp = GetTimeArrayFromJSON (value_p);
								}
						}
				}
		}

	param_p = GetNewTimeArrayParameter (current_values_pp, default_values_pp, num_values);

	if (default_values_pp)
		{
			FreeTimeArray (default_values_pp, num_values);
		}

	if (current_values_pp)
		{
			FreeTimeArray (current_values_pp, num_values);
		}


	if (param_p)
		{
			Parameter *base_param_p = & (param_p -> tap_base_param);
			ParameterType pt = pt_p ? *pt_p : PT_TIME_ARRAY;

			if (InitParameterFromJSON (base_param_p, param_json_p, service_p, concise_flag, &pt))
				{
					SetParameterCallbacks (base_param_p, ClearTimeArrayParameter, AddTimeArrayParameterDetailsToJSON,
																 CloneTimeArrayParameter, SetTimeArrayParameterCurrentValueFromString);

					return param_p;
				}
			else
				{
					ClearParameter (& (param_p -> tap_base_param));
					FreeMemory (param_p);
				}
		}

	return NULL;
}


Parameter *EasyCreateAndAddTimeArrayParameterToParameterSet (const ServiceData *service_data_p, ParameterSet *params_p, ParameterGroup *group_p,
																											const char * const name_s, const char * const display_name_s, const char * const description_s,
																											struct tm **default_values_pp, const size_t num_values, ParameterLevel level)
{
	return CreateAndAddTimeArrayParameterToParameterSet (service_data_p, params_p, group_p,
																										name_s, display_name_s, description_s,
																										default_values_pp, default_values_pp, num_values, level);
}

Parameter *CreateAndAddTimeArrayParameterToParameterSet (const ServiceData *service_data_p, ParameterSet *params_p, ParameterGroup *group_p,
																											const char * const name_s, const char * const display_name_s, const char * const description_s,
																											struct tm **default_values_pp, struct tm **current_values_pp,
																											const size_t num_values, ParameterLevel level)
{
	Parameter *base_param_p = NULL;
	TimeArrayParameter *time_array_param_p = AllocateTimeArrayParameter (service_data_p, name_s, display_name_s, description_s, default_values_pp, current_values_pp, num_values, level);

	if (time_array_param_p)
		{
			base_param_p = & (time_array_param_p -> tap_base_param);

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


const struct tm **GetTimeArrayParameterCurrentValues (const TimeArrayParameter *param_p)
{
	return (const struct tm **) (param_p -> tap_current_values_pp);
}


const struct tm **GetTimeArrayParameterDefaultValues (const TimeArrayParameter *param_p)
{
	return (const struct tm **) (param_p -> tap_default_values_pp);
}


const struct tm *GetTimeArrayParameterCurrentValueAtIndex (const TimeArrayParameter *param_p, const size_t index)
{
	if (index < param_p -> tap_num_values)
		{
			return * ((param_p -> tap_current_values_pp) + index);
		}

	return NULL;
}


bool SetTimeArrayParameterCurrentValues (TimeArrayParameter *param_p, const struct tm **values_pp, const size_t num_values)
{
	return SetTimeArrayParameterValues (values_pp, & (param_p -> tap_current_values_pp), num_values);
}


bool SetTimeArrayParameterDefaultValues (TimeArrayParameter *param_p, const struct tm **values_pp, const size_t num_values)
{
	return SetTimeArrayParameterValues (values_pp, & (param_p -> tap_default_values_pp), num_values);
}


static bool SetTimeArrayParameterValues (const struct tm **src_values_pp, struct tm ***dest_ppp, const size_t num_values)
{
	bool success_flag = false;
	struct tm **copied_values_pp = CopyTimeArray (src_values_pp, num_values);

	if (copied_values_pp)
		{
			if (*dest_ppp)
				{
					FreeTimeArray (*dest_ppp, num_values);
				}

			success_flag = true;
		}

	return success_flag;
}


bool IsTimeArrayParameter (const Parameter *param_p)
{
	bool match_flag = false;

	if (param_p -> pa_type == PT_TIME_ARRAY)
		{
			match_flag = true;
		}

	return match_flag;
}


size_t GetNumberOfTimeArrayCurrentParameterValues (const TimeArrayParameter *param_p)
{
	return (param_p -> tap_num_values);
}


char *GetTimeArrayParameterCurrentValuesAsFlattenedString (const TimeArrayParameter *param_p)
{
	char *flattened_s = NULL;

	if (param_p -> tap_current_values_pp)
		{
			ByteBuffer *buffer_p = AllocateByteBuffer (1024);

			if (buffer_p)
				{
					bool success_flag = true;
					struct tm **value_pp = param_p -> tap_current_values_pp;
					const char *delimiter_s = "\n";
					size_t i = param_p -> tap_num_values;

					while ((i > 0) && success_flag)
						{
							if (*value_pp)
								{
									char *time_s = GetTimeAsString (*value_pp, true, NULL);

									if (time_s)
										{
											if (!AppendStringsToByteBuffer (buffer_p, time_s, delimiter_s, NULL))
												{
													success_flag = false;
												}

											FreeTimeString (time_s);
										}
									else
										{
											success_flag = false;
										}
								}
							else
								{
									if (!AppendStringToByteBuffer (buffer_p, delimiter_s))
										{
											success_flag = false;
										}
								}

							if (success_flag)
								{
									++ value_pp;
									-- i;
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



/*
 * STATIC DEFINITIONS
 */

static TimeArrayParameter *GetNewTimeArrayParameter (const struct tm **current_values_pp, const struct tm **default_values_pp, const size_t num_values)
{
	TimeArrayParameter *param_p = (TimeArrayParameter *) AllocMemory (sizeof (TimeArrayParameter));

	if (param_p)
		{
			param_p -> tap_current_values_pp = NULL;
			param_p -> tap_default_values_pp = NULL;

			NullifyParameter (& (param_p -> tap_base_param));

			if (SetTimeArrayParameterValue (& (param_p -> tap_current_values_pp), 0, current_values_pp, num_values))
				{
					if (SetTimeArrayParameterValue (& (param_p -> tap_default_values_pp), 0, default_values_pp, num_values))
						{
							param_p -> tap_num_values = num_values;

							return param_p;
						}

					if (param_p -> tap_current_values_pp)
						{
							FreeTimeArray (param_p -> tap_current_values_pp, num_values);
						}
				}

			FreeMemory (param_p);
		}

	return NULL;
}



static Parameter *CloneTimeArrayParameter (const Parameter *param_p, const ServiceData *service_data_p)
{
	return NULL;
}


static bool SetTimeArrayParameterCurrentValueFromString (Parameter *param_p, const char *value_s)
{
	bool success_flag = false;
	TimeArrayParameter *time_array_param_p = (TimeArrayParameter *) param_p;
	LinkedList *lines_p = ParseStringToStringLinkedList (value_s, "\n", false);

	if (lines_p)
		{
			const uint32 num_values = lines_p -> ll_size;
			struct tm **times_pp = AllocMemoryArray (num_values, sizeof (struct tm *));

			if (times_pp)
				{
					StringListNode *node_p = (StringListNode *) (lines_p -> ll_head_p);
					struct tm **time_pp = times_pp;
					bool loop_flag = true;

					while (node_p && loop_flag)
						{
							char *time_s = DetachStringFromStringListNode (node_p);
							struct tm *time_p = GetTimeFromString (time_s);

							FreeCopiedString (time_s);

							if (time_p)
								{
									*time_pp = time_p;
									++ time_pp;

									node_p = (StringListNode *) (node_p -> sln_node.ln_next_p);
								}
							else
								{
									loop_flag = false;
								}
						}

					if (loop_flag)
						{
							if (SetTimeArrayParameterCurrentValues (time_array_param_p, times_pp, num_values))
								{
									success_flag = true;
								}
							else
								{
									FreeTimeArray (times_pp, num_values);
								}

						}

				}

			FreeLinkedList (lines_p);
		}

	return success_flag;
}



static struct tm **CopyTimeArray (const struct tm **src_array_pp, const size_t num_values)
{
	struct tm **dest_array_pp = (struct tm **) AllocMemoryArray (num_values, sizeof (struct tm *));

	if (dest_array_pp)
		{
			struct tm **dest_pp = dest_array_pp;
			bool success_flag = true;
			const struct tm **src_pp = src_array_pp;
			size_t i = num_values;

			while (success_flag && (i > 0))
				{
					if (*src_pp)
						{
							struct tm *dest_p = DuplicateTime (*src_pp);

							if (dest_p)
								{
									*dest_pp = dest_p;
								}
							else
								{
									success_flag = false;
								}
						}
					else
						{
							*dest_pp = NULL;
						}

					if (success_flag)
						{
							++ dest_pp;
							++ src_pp;
							-- i;
						}
				}

			if (success_flag)
				{
					return dest_array_pp;
				}
			else
				{
					FreeTimeArray (dest_pp, num_values);
				}

		}		/* if (dest_ss) */

	return NULL;
}



static bool SetTimeArrayParameterValue (struct tm ***param_values_ppp, const size_t num_existing_values, const struct tm **new_values_pp, const size_t num_new_values)
{
	bool success_flag = false;

	if (new_values_pp)
		{
			struct tm **copied_values_pp = CopyTimeArray (new_values_pp, num_new_values);

			if (copied_values_pp)
				{
					if (*param_values_ppp)
						{
							FreeTimeArray (*param_values_ppp, num_existing_values);
						}

					*param_values_ppp = copied_values_pp;
					success_flag = true;
				}
		}
	else
		{
			if (*param_values_ppp)
				{
					FreeTimeArray (*param_values_ppp, num_existing_values);
					*param_values_ppp = NULL;
				}

			success_flag = true;
		}

	return success_flag;
}


static json_t *ConvertTimeArrayToJSON (struct tm **times_pp, const size_t num_values)
{
	json_t *times_json_p = json_array ();

	if (times_json_p)
		{
			size_t i = num_values;
			struct tm **time_pp = times_pp;
			bool success_flag = true;

			while ((i > 0) && success_flag)
				{
					if (*time_pp)
						{
							char *time_s = GetTimeAsString (*time_pp, true, NULL);

							if (time_s)
								{
									json_t *time_json_p = json_string (time_s);

									if (time_json_p)
										{
											if (json_array_append_new (times_json_p, time_json_p) != 0)
												{
													json_decref (time_json_p);
													success_flag = false;
												}
										}
									else
										{
											success_flag = false;
										}

									FreeCopiedString (time_s);
								}
							else
								{
									success_flag = false;
								}
						}
					else
						{
							if (json_array_append_new (times_json_p, json_null ()) != 0)
								{
									success_flag = false;
								}
						}

					-- i;
					++ time_pp;
				}

			if (success_flag)
				{
					return times_json_p;
				}

			json_decref (times_json_p);
		}		/* if (times_json_p) */

	return NULL;
}


static bool AddNonTrivialTimeArrayValuesToJSON (struct tm **times_pp, json_t *param_json_p, const char *key_s, const uint32 num_values)
{
	bool success_flag = false;

	if (times_pp)
		{
			json_t *values_json_p = ConvertTimeArrayToJSON (times_pp, num_values);

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



static bool AddTimeArrayParameterDetailsToJSON (const Parameter *param_p, json_t *param_json_p, const bool full_definition_flag)
{
	bool success_flag = false;
	TimeArrayParameter *time_array_param_p = (TimeArrayParameter *) param_p;

	if (AddNonTrivialTimeArrayValuesToJSON (time_array_param_p -> tap_current_values_pp, param_json_p, PARAM_CURRENT_VALUE_S, time_array_param_p -> tap_num_values))
		{
			if (full_definition_flag)
				{
					if (AddNonTrivialTimeArrayValuesToJSON (time_array_param_p -> tap_default_values_pp, param_json_p, PARAM_DEFAULT_VALUE_S, time_array_param_p -> tap_num_values))
						{
							success_flag = true;
						}		/* if (AddNonTrivialTimeArrayValuesToJSON (time_array_param_p -> sp_default_values_ss, param_json_p, PARAM_DEFAULT_VALUE_S)) */

				}		/* if (full_definition_flag) */
			else
				{
					success_flag = true;
				}
		}		/* if (AddNonTrivialTimeArrayValuesToJSON (time_array_param_p -> sp_current_values_ss, param_json_p, PARAM_CURRENT_VALUE_S)) */

	return success_flag;
}


static void ClearTimeArrayParameter (Parameter *param_p)
{
	TimeArrayParameter *time_array_param_p = (TimeArrayParameter *) param_p;

	if (time_array_param_p -> tap_current_values_pp)
		{
			FreeTimeArray (time_array_param_p -> tap_current_values_pp, time_array_param_p -> tap_num_values);
			time_array_param_p -> tap_current_values_pp = NULL;
		}


	if (time_array_param_p -> tap_default_values_pp)
		{
			FreeTimeArray (time_array_param_p -> tap_default_values_pp, time_array_param_p -> tap_num_values);
			time_array_param_p -> tap_default_values_pp = NULL;
		}
}


static struct tm **GetTimeArrayFromJSON (const json_t *times_json_p)
{
	const size_t size = json_array_size (times_json_p);

	if (size > 0)
		{
			struct tm **times_pp = (struct tm **) AllocMemoryArray (size, sizeof (struct tm *));

			if (times_pp)
				{
					size_t i = 0;
					struct tm **time_pp = times_pp;
					bool success_flag = true;

					while ((i < size) && success_flag)
						{
							json_t *entry_p = json_array_get (times_json_p, i);

							if (json_is_string (entry_p))
								{
									const char *time_s = json_string_value (entry_p);
									struct tm *time_p = GetTimeFromString (time_s);

									if (time_p)
										{
											*time_pp = time_p;
										}
									else
										{
											success_flag = false;
											i = size; 		/* force exit from loop */
										}
								}
							else if (json_is_null (entry_p))
								{
									*time_pp = NULL;
								}
							else
								{
									success_flag = false;
									i = size; 		/* force exit from loop */
								}

							if (success_flag)
								{
									++ i;
									++ time_pp;
								}
						}

					if (success_flag)
						{
							return times_pp;
						}
					else
						{
							while (time_pp >= times_pp)
								{
									if (*time_pp)
										{
											FreeTime (*time_pp);
										}

									-- time_pp;
								}
						}

					FreeMemory (times_pp);
				}		/* if (times_pp) */

		}

	return NULL;
}
