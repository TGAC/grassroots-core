/*
 * shared_type.c
 *
 *  Created on: 11 Oct 2019
 *      Author: billy
 */

#include <string.h>

#include "shared_type.h"
#include "streams.h"
#include "string_utils.h"
#include "time_util.h"
#include "parameter.h"
#include "json_tools.h"


void InitSharedType (SharedType *st_p, const ParameterType pt)
{
	memset (st_p, 0, sizeof (SharedType));
}


void ClearSharedType (SharedType *st_p)
{
	switch (st_p -> st_active_type)
		{
			case PT_DIRECTORY:
			case PT_FILE_TO_READ:
			case PT_FILE_TO_WRITE:
				if (st_p -> st_value.st_resource_value_p)
					{
						FreeResource (st_p -> st_resource_value_p);
						st_p -> st_resource_value_p = NULL;
					}
				break;

			case PT_TABLE:
			case PT_STRING:
			case PT_LARGE_STRING:
			case PT_PASSWORD:
			case PT_KEYWORD:
			case PT_FASTA:
				if (st_p -> st_string_value_s)
					{
						FreeCopiedString (st_p -> st_string_value_s);
						st_p -> st_string_value_s = NULL;
					}
				break;

			case PT_JSON:
			case PT_JSON_TABLE:
				if (st_p -> st_json_p)
					{
	#if PARAMETER_DEBUG >= STM_LEVEL_FINER
						PrintJSONRefCounts (st_p -> st_json_p, "freeing param json", STM_LEVEL_FINER, __FILE__, __LINE__);
	#endif

						json_decref (st_p -> st_json_p);
						st_p -> st_json_p = NULL;
					}
				break;

			case PT_TIME:
				if (st_p -> st_time_p)
					{
						FreeTime (st_p -> st_time_p);
						st_p -> st_time_p = NULL;
					}
				break;

			case PT_NEGATIVE_INT:
			case PT_SIGNED_INT:
				if (st_p -> st_long_value_p)
					{
						FreeMemory (st_p -> st_long_value_p);
						st_p -> st_long_value_p = NULL;

					}
				break;

			case PT_UNSIGNED_INT:
				if (st_p -> st_ulong_value_p)
					{
						FreeMemory (st_p -> st_ulong_value_p);
						st_p -> st_ulong_value_p = NULL;
					}
				break;

			case PT_SIGNED_REAL:
			case PT_UNSIGNED_REAL:
				if (st_p -> st_data_value_p)
					{
						FreeMemory (st_p -> st_data_value_p);
						st_p -> st_data_value_p = NULL;
					}
				break;

			case PT_BOOLEAN:
				if (st_p -> st_boolean_value_p)
					{
						FreeMemory (st_p -> st_boolean_value_p);
						st_p -> st_boolean_value_p = NULL;
					}
				break;

			case PT_CHAR:
				if (st_p -> st_char_value_p)
					{
						FreeMemory (st_p -> st_char_value_p);
						st_p -> st_char_value_p = NULL;
					}
				break;

			case PT_NUM_TYPES:
				PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Invalid ParameterType");
				break;

		}

	InitSharedType (st_p);
}


bool SetSharedTypeValue (SharedType *st_p, const ParameterType pt, void *value_p, const ParameterBounds *bounds_p)
{
	bool success_flag = false;

	switch (pt)
		{
			case PT_BOOLEAN:
				{
					const bool b = * ((bool *) value_p);
					success_flag = SetSharedTypeBooleanValue (st_p, b);
				}
				break;

			case PT_CHAR:
				{
					const char c = * ((char *) value_p);
					success_flag = SetSharedTypeCharValue (st_p, c);
				}
				break;


			case PT_SIGNED_INT:
			case PT_NEGATIVE_INT:
				{
					const int32 i = * ((int32 *) value_p);
					success_flag = SetSharedTypeSignedIntValue (st_p, i, bounds_p);
				}
				break;

			case PT_UNSIGNED_INT:
				{
					const uint32 i = * ((uint32 *) value_p);
					success_flag = SetSharedTypeUnsignedIntValue (st_p, i, bounds_p);
				}
				break;

			case PT_SIGNED_REAL:
			case PT_UNSIGNED_REAL:
				{
					const double d = * ((double *) value_p);
					success_flag = SetSharedTypeRealValue (st_p, d, bounds_p);
				}
				break;

			case PT_LARGE_STRING:
			case PT_STRING:
			case PT_TABLE:
			case PT_PASSWORD:
			case PT_KEYWORD:
			case PT_FASTA:
				{
					const char * const value_s = (const char *) value_p;
					success_flag = SetSharedTypeStringValue (st_p, value_s);
				}
				break;

			case PT_FILE_TO_WRITE:
			case PT_FILE_TO_READ:
			case PT_DIRECTORY:
				{
					const Resource * const res_p = (const Resource *) value_p;
					success_flag = SetSharedTypeResourceValue (st_p, res_p);
				}
				break;

			case PT_JSON:
			case PT_JSON_TABLE:
				{
					const json_t * const src_p = (const json_t *) value_p;
					success_flag = SetSharedTypeJSONValue (st_p, src_p);
				}
				break;

			case PT_TIME:
				{
					const struct tm * const src_p = (const struct tm *) value_p;
					success_flag = SetSharedTypeTimeValue (st_p, src_p);
				}
				break;

			case PT_NUM_TYPES:
				PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "SharedType has invalid type");
				break;
		}

	return success_flag;
}





bool SetSharedTypeBooleanValue (SharedType * value_p, const bool b)
{
	value_p -> st_boolean_value = b;

	return true;
}


bool SetSharedTypeCharValue (SharedType * value_p, const char c)
{
	value_p -> st_char_value = c;

	return true;
}



bool SetSharedTypeUnsignedIntValue (SharedType * value_p, const uint32 i, const ParameterBounds * const bounds_p)
{
	bool success_flag = false;

	if (bounds_p)
		{
			if ((i >= bounds_p -> pb_lower.st_ulong_value) &&
					(i <= bounds_p -> pb_upper.st_ulong_value))
				{
					value_p -> st_ulong_value = i;
					success_flag = true;
				}
		}
	else
		{
			value_p -> st_ulong_value = i;
			success_flag = true;
		}

	return success_flag;
}


bool SetSharedTypeSignedIntValue (SharedType * value_p, const int32 i, const ParameterBounds * const bounds_p)
{
	bool success_flag = false;

	if (bounds_p)
		{
			if ((i >= bounds_p -> pb_lower.st_long_value) &&
					(i <= bounds_p -> pb_upper.st_long_value))
				{
					value_p -> st_long_value = i;
					success_flag = true;
				}
		}
	else
		{
			value_p -> st_long_value = i;
			success_flag = true;
		}

	return success_flag;
}



bool SetSharedTypeRealValue (SharedType * value_p, const double64 d, const ParameterBounds * const bounds_p)
{
	bool success_flag = false;

	if (bounds_p)
		{
			if ((d >= bounds_p -> pb_lower.st_data_value) &&
					(d <= bounds_p -> pb_upper.st_data_value))
				{
					value_p -> st_data_value = d;
					success_flag = true;
				}
		}
	else
		{
			value_p -> st_data_value = d;
			success_flag = true;
		}

	return success_flag;
}


bool SetSharedTypeStringValue (SharedType *value_p, const char * const src_s)
{
	bool success_flag = false;

	if (src_s)
		{
			char *copied_value_s = EasyCopyToNewString (src_s);

			if (copied_value_s)
				{
					/* If we have a previous value, delete it */
					if (value_p -> st_string_value_s)
						{
							FreeCopiedString (value_p -> st_string_value_s);
						}

					value_p -> st_string_value_s = copied_value_s;
					success_flag = true;
				}
		}
	else
		{
			/* If we have a previous value, delete it */
			if (value_p -> st_string_value_s)
				{
					FreeCopiedString (value_p -> st_string_value_s);
					value_p -> st_string_value_s = NULL;
				}

			success_flag = true;
		}

	return success_flag;
}


bool SetSharedTypeResourceValue (SharedType *value_p, const Resource * const src_p)
{
	bool success_flag = false;

	if (src_p)
		{
			if (value_p -> st_resource_value_p)
				{
					success_flag = CopyResource (src_p, value_p -> st_resource_value_p);
				}
			else
				{
					value_p -> st_resource_value_p = AllocateResource (src_p -> re_protocol_s, src_p -> re_value_s, src_p -> re_title_s);

					success_flag = (value_p -> st_resource_value_p != NULL);
				}
		}
	else
		{
			if (value_p -> st_resource_value_p)
				{
					FreeResource (value_p -> st_resource_value_p);
					value_p -> st_resource_value_p = NULL;
				}

			success_flag = true;
		}

	return success_flag;
}


bool SetSharedTypeJSONValue (SharedType *value_p, const json_t * const src_p)
{
	bool success_flag = false;

	if (src_p)
		{
			json_t *json_value_p = json_deep_copy (src_p);

			if (json_value_p)
				{
					/* If we have a previous value, delete it */
					if (value_p -> st_json_p)
						{
							WipeJSON (value_p -> st_json_p);
						}

					value_p -> st_json_p = json_value_p;
					success_flag = true;
				}
		}
	else
		{
			/* If we have a previous value, delete it */
			if (value_p -> st_json_p)
				{
					WipeJSON (value_p -> st_json_p);
					value_p -> st_json_p = NULL;
				}

			success_flag = true;
		}

	return success_flag;
}



bool SetSharedTypeTimeValue (SharedType *value_p, const struct tm * const src_p)
{
	bool success_flag = false;

	if (src_p)
		{
			if (! (value_p -> st_time_p))
				{
					if (! (value_p -> st_time_p = AllocateTime ()))
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate time variable");
						}
				}

			if (value_p -> st_time_p)
				{
					CopyTime (src_p, value_p -> st_time_p);
					success_flag = true;
				}

		}
	else
		{
			/* If we have a previous value, delete it */
			if (value_p -> st_time_p)
				{
					FreeTime (value_p -> st_time_p);
					value_p -> st_time_p = NULL;
				}

			success_flag = true;
		}

	return success_flag;
}



bool GetSharedTypeBooleanValue (const SharedType *value_p, bool * const b_p)
{
	bool got_flag = false;

	if (value_p -> st_active_type == PT_BOOLEAN)
		{
			if (value_p -> st_value.st_boolean_value_p)
				{
					*b_p = * (value_p -> st_value.st_boolean_value_p);
					got_flag = true;
				}
		}
	else
		{
			const char *type_s = GetGrassrootsTypeAsString (value_p -> st_active_type);

			if (!type_s)
				{
					type_s = "Unknown type";
				}

			PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "GetSharedTypeBooleanValue failed, value of type %d: \%s\"", value_p -> st_active_type, type_s);
		}

	return got_flag;
}


bool GetSharedTypeCharValue (const SharedType *value_p, char * const c_p)
{
	bool got_flag = false;

	if (value_p -> st_active_type == PT_CHAR)
		{
			if (value_p -> st_value.st_char_value_p)
				{
					*c_p = * (value_p -> st_value.st_char_value_p);
					got_flag = true;
				}
		}
	else
		{
			const char *type_s = GetGrassrootsTypeAsString (value_p -> st_active_type);

			if (!type_s)
				{
					type_s = "Unknown type";
				}

			PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "GetSharedTypeCharValue failed, value of type %d: \%s\"", value_p -> st_active_type, type_s);
		}

	return got_flag;
}


bool GetSharedTypeUnsignedIntValue (const SharedType * value_p, uint32 * const i_p)
{
	bool got_flag = false;

	if (value_p -> st_active_type == PT_UNSIGNED_INT)
		{
			if (value_p -> st_value.st_ulong_value_p)
				{
					*i_p = * (value_p -> st_value.st_ulong_value_p);
					got_flag = true;
				}
		}
	else
		{
			const char *type_s = GetGrassrootsTypeAsString (value_p -> st_active_type);

			if (!type_s)
				{
					type_s = "Unknown type";
				}

			PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "GetSharedTypeUnsignedIntValue failed, value of type %d: \%s\"", value_p -> st_active_type, type_s);
		}

	return got_flag;
}


bool GetSharedTypeSignedIntValue (const SharedType * value_p, int32 * const i_p)
{
	bool got_flag = false;

	if ((value_p -> st_active_type == PT_SIGNED_INT) || (value_p -> st_active_type == PT_NEGATIVE_INT))
		{
			if (value_p -> st_value.st_long_value_p)
				{
					*i_p = * (value_p -> st_value.st_long_value_p);
					got_flag = true;
				}
		}
	else
		{
			const char *type_s = GetGrassrootsTypeAsString (value_p -> st_active_type);

			if (!type_s)
				{
					type_s = "Unknown type";
				}

			PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "GetSharedTypeSignedIntValue failed, value of type %d: \%s\"", value_p -> st_active_type, type_s);
		}

	return got_flag;
}


bool GetSharedTypeRealValue (const SharedType *value_p, double64 * const d_p)
{
	bool got_flag = false;

	if ((value_p -> st_active_type == PT_SIGNED_REAL) || (value_p -> st_active_type == PT_UNSIGNED_REAL))
		{
			if (value_p -> st_value.st_data_value_p)
				{
					*d_p = * (value_p -> st_value.st_data_value_p);
					got_flag = true;
				}
		}
	else
		{
			const char *type_s = GetGrassrootsTypeAsString (value_p -> st_active_type);

			if (!type_s)
				{
					type_s = "Unknown type";
				}

			PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "GetSharedTypeRealValue failed, value of type %d: \%s\"", value_p -> st_active_type, type_s);
		}

	return got_flag;
}


bool GetSharedTypeStringValue (const SharedType *value_p, char ** const src_ss)
{
	bool got_flag = false;

	switch (value_p -> st_active_type)
		{
			case PT_TABLE:
			case PT_LARGE_STRING:
			case PT_STRING:
			case PT_PASSWORD:
			case PT_KEYWORD:
			case PT_FASTA:
				{
					if (value_p -> st_value.st_string_value_s)
						{
							*src_ss = * (value_p -> st_value.st_string_value_s);
							got_flag = true;
						}
				}
				break;

			default:
				{
					const char *type_s = GetGrassrootsTypeAsString (value_p -> st_active_type);

					if (!type_s)
						{
							type_s = "Unknown type";
						}

					PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "GetSharedTypeStringValue failed, value of type %d: \%s\"", value_p -> st_active_type, type_s);
				}
				break;
		}

	return got_flag;

}


const Resource *GetSharedTypeResourceValue (const SharedType *value_p, bool *success_flag_p)
{
	Resource *res_p = NULL;

	if (value_p -> st_active_type == PT_FILE_TO_READ)
		{
			res_p = value_p -> st_value.st_resource_value_p;
			*success_flag_p = true;
		}
	else
		{
			const char *type_s = GetGrassrootsTypeAsString (value_p -> st_active_type);

			if (!type_s)
				{
					type_s = "Unknown type";
				}

			PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "GetSharedTypeResourceValue failed, value of type %d: \%s\"", value_p -> st_active_type, type_s);

			*success_flag_p = false;
		}

	return res_p;
}


bool GetSharedTypeJSONValue (const SharedType *value_p, json_t * const json_p);

const struct tm *GetSharedTypeTimeValue (const SharedType *value_p, bool *success_flag_p)
{
	struct tm *res_p = NULL;

	if (value_p -> st_active_type == PT_TIME)
		{
			res_p = value_p -> st_value.st_time_p;
			*success_flag_p = true;
		}
	else
		{
			const char *type_s = GetGrassrootsTypeAsString (value_p -> st_active_type);

			if (!type_s)
				{
					type_s = "Unknown type";
				}

			PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "GetSharedTypeTimeValue failed, value of type %d: \%s\"", value_p -> st_active_type, type_s);

			*success_flag_p = false;
		}

	return res_p;
}


const struct tm *GetSharedTypeTimeValue (const SharedType *value_p, bool *success_flag_p)
{
	struct tm *res_p = NULL;

	if (value_p -> st_active_type == PT_TIME)
		{
			res_p = value_p -> st_value.st_time_p;
			*success_flag_p = true;
		}
	else
		{
			const char *type_s = GetGrassrootsTypeAsString (value_p -> st_active_type);

			if (!type_s)
				{
					type_s = "Unknown type";
				}

			PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "GetSharedTypeTimeValue failed, value of type %d: \%s\"", value_p -> st_active_type, type_s);

			*success_flag_p = false;
		}

	return res_p;
}


