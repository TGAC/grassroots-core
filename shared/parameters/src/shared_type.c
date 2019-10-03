/*
 * shared_type.c
 *
 *  Created on: 2 Oct 2019
 *      Author: billy
 */


#include <string.h>

#include "shared_type.h"
#include "streams.h"
#include "string_utils.h"
#include "memory_allocations.h"
#include "time_util.h"
#include "json_tools.h"
#include "json_util.h"


bool SetSharedTypeFromString (SharedType * const value_p, const ParameterType pt, const char *value_s)
{
	bool success_flag = false;

	if (value_s)
		{
			switch (pt)
				{
					case PT_BOOLEAN:
						{
							if (Stricmp (value_s, "true") == 0)
								{
									success_flag = SetSharedTypeBooleanValue (value_p, true);
								}
							else if (Stricmp (value_s, "false") == 0)
								{
									success_flag = SetSharedTypeBooleanValue (value_p, false);
								}
						}
						break;

					case PT_SIGNED_INT:
					case PT_NEGATIVE_INT:
						{
							int32 value;

							if (sscanf (value_s, INT32_FMT, &value) > 0)
								{
									success_flag = SetSharedTypeSignedIntValue (value_p, value, NULL);
								}
						}
						break;

					case PT_UNSIGNED_INT:
						{
							uint32 value;

							if (sscanf (value_s, UINT32_FMT, &value) > 0)
								{
									success_flag = SetSharedTypeUnsignedIntValue (value_p, value, NULL);
								}
						}
						break;

					case PT_SIGNED_REAL:
					case PT_UNSIGNED_REAL:
						{
							double64 value;

							if (sscanf (value_s, DOUBLE64_FMT, &value) > 0)
								{
									success_flag = SetSharedTypeRealValue (value_p, value, NULL);
								}
						}
						break;

					case PT_DIRECTORY:
					case PT_FILE_TO_READ:
					case PT_FILE_TO_WRITE:
						{
							Resource *resource_p = ParseStringToResource (value_s);

							if (resource_p)
								{
									success_flag = SetSharedTypeResourceValue (value_p, resource_p);
									FreeResource (resource_p);
								}
						}
						break;

					case PT_TABLE:
					case PT_LARGE_STRING:
					case PT_STRING:
					case PT_PASSWORD:
					case PT_KEYWORD:
					case PT_FASTA:
						success_flag = SetSharedTypeStringValue (value_p, value_s);
						break;


					case PT_CHAR:
						break;

					case PT_JSON:
						break;

					case PT_TIME:
						break;

					case PT_NUM_TYPES:
						break;
					//default:
					//	break;
				}		/* switch (pt) */

		}		/* if (value_s) */
	else
		{
			ClearSharedType (value_p, pt);
			success_flag = true;
		}

	return success_flag;
}


void FreeSharedTypeNode (ListItem *node_p)
{
	FreeMemory (node_p);
}




void InitSharedType (SharedType *st_p)
{
	memset (st_p, 0, sizeof (SharedType));
}


void ClearSharedType (SharedType *st_p, const ParameterType pt)
{
	switch (pt)
		{
			case PT_DIRECTORY:
			case PT_FILE_TO_READ:
			case PT_FILE_TO_WRITE:
				if (st_p -> st_resource_value_p)
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
}



bool SetSharedTypeFromJSON (SharedType *value_p, const json_t *json_p, const ParameterType pt)
{
	bool success_flag = false;

	if (json_is_null (json_p))
		{
			ClearSharedType (value_p, pt);
			success_flag = true;
		}
	else
		{
			switch (pt)
				{
					case PT_BOOLEAN:
						{
							success_flag = SetBooleanFromJSON (json_p, value_p -> st_boolean_value_p);
						}
						break;

					case PT_SIGNED_INT:
					case PT_NEGATIVE_INT:
						{
							success_flag = SetIntegerFromJSON (json_p, value_p -> st_long_value_p);
						}
						break;

					case PT_UNSIGNED_INT:
						{
							int32 i;

							if (SetIntegerFromJSON (json_p, &i))
								{
									success_flag = SetSharedTypeUnsignedIntValue (value_p, (uint32) i, NULL);
								}
						}
						break;

					case PT_SIGNED_REAL:
					case PT_UNSIGNED_REAL:
						{
							success_flag = SetRealFromJSON (json_p, value_p -> st_data_value_p);
						}
						break;

					case PT_STRING:
					case PT_FILE_TO_WRITE:
					case PT_FILE_TO_READ:
					case PT_DIRECTORY:
					case PT_KEYWORD:
					case PT_LARGE_STRING:
					case PT_PASSWORD:
					case PT_FASTA:
					case PT_TABLE:
						{
							char *value_s = NULL;

							success_flag = SetStringFromJSON (json_p, &value_s);

							if (success_flag)
								{
									if (value_p -> st_string_value_s)
										{
											FreeCopiedString (value_p -> st_string_value_s);
										}

									value_p -> st_string_value_s = value_s;
								}
						}
						break;

					case PT_CHAR:
						{
							char *value_s = NULL;

							if (SetStringFromJSON (json_p, &value_s))
								{
									if (strlen (value_s) == 1)
										{
											success_flag = SetSharedTypeCharValue (value_p, *value_s);
										}
									else
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "\"%s\" is too long for single char value", value_s);
										}
								}
						}
						break;

					case PT_JSON:
						{
							json_t *dest_p = json_deep_copy (json_p);

							if (dest_p)
								{
									if (value_p -> st_json_p)
										{
											json_decref (value_p -> st_json_p);
										}

									value_p -> st_json_p = dest_p;
								}
							else
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, json_p, "Failed to copy JSON value");
								}
						}
						break;

					case PT_TIME:
						{
							if (json_is_string (json_p))
								{
									const char *json_value_s = json_string_value (json_p);
									struct tm* time_p = GetTimeFromString (json_value_s);

									if (time_p)
										{
											if (value_p -> st_time_p)
												{
													FreeTime (value_p -> st_time_p);
												}

											value_p -> st_time_p = time_p;
											success_flag = true;
										}
								}
						}
						break;

					case PT_NUM_TYPES:
						PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, json_p, "Parameter has invalid type");
						break;
				}		/* switch (pt) */

		}

	return success_flag;
}






bool SetSharedTypeBooleanValue (SharedType * value_p, const bool b)
{
	if (! (value_p -> st_boolean_value_p))
		{
			value_p -> st_boolean_value_p = (bool *) AllocMemory (sizeof (bool));

			if (! (value_p -> st_boolean_value_p))
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate bool value");
					return false;
				}
		}

	* (value_p -> st_boolean_value_p) = b;

	return true;
}


bool SetSharedTypeCharValue (SharedType * value_p, const char c)
{
	if (! (value_p -> st_char_value_p))
		{
			value_p -> st_char_value_p = (char *) AllocMemory (sizeof (char));

			if (! (value_p -> st_char_value_p))
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate char value");
					return false;
				}
		}

	* (value_p -> st_char_value_p) = c;

	return true;
}



bool SetSharedTypeUnsignedIntValue (SharedType * value_p, const uint32 i, const ParameterBounds * const bounds_p)
{
	if (bounds_p)
		{
			if ((bounds_p -> pb_lower.st_ulong_value_p) && (i < * (bounds_p -> pb_lower.st_ulong_value_p)))
				{
					PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Value " UINT32_FMT " is lower than bound " UINT32_FMT);
					return false;
				}

			if ((bounds_p -> pb_upper.st_ulong_value_p) && (i > * (bounds_p -> pb_upper.st_ulong_value_p)))
				{
					PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Value " UINT32_FMT " is higher than bound " UINT32_FMT);
					return false;
				}
		}


	if (! (value_p -> st_ulong_value_p))
		{
			value_p -> st_ulong_value_p = (uint32 *) AllocMemory (sizeof (uint32));

			if (! (value_p -> st_ulong_value_p))
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate unsigned int value");
					return false;
				}
		}

	* (value_p -> st_ulong_value_p) = i;

	return true;
}


bool SetSharedTypeSignedIntValue (SharedType * value_p, const int32 i, const ParameterBounds * const bounds_p)
{
	if (bounds_p)
		{
			if ((bounds_p -> pb_lower.st_long_value_p) && (i < * (bounds_p -> pb_lower.st_long_value_p)))
				{
					PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Value " INT32_FMT " is lower than bound " INT32_FMT);
					return false;
				}

			if ((bounds_p -> pb_upper.st_long_value_p) && (i > * (bounds_p -> pb_upper.st_long_value_p)))
				{
					PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Value " INT32_FMT " is higher than bound " INT32_FMT);
					return false;
				}
		}


	if (! (value_p -> st_long_value_p))
		{
			value_p -> st_long_value_p = (int32 *) AllocMemory (sizeof (int32));

			if (! (value_p -> st_long_value_p))
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate signed int value");
					return false;
				}
		}

	* (value_p -> st_long_value_p) = i;

	return true;
}



bool SetSharedTypeRealValue (SharedType * value_p, const double64 d, const ParameterBounds * const bounds_p)
{
	if (bounds_p)
		{
			if ((bounds_p -> pb_lower.st_data_value_p) && (d < * (bounds_p -> pb_lower.st_data_value_p)))
				{
					PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Value " DOUBLE64_FMT " is lower than bound " DOUBLE64_FMT);
					return false;
				}

			if ((bounds_p -> pb_upper.st_data_value_p) && (d > * (bounds_p -> pb_upper.st_data_value_p)))
				{
					PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Value " DOUBLE64_FMT " is higher than bound " DOUBLE64_FMT);
					return false;
				}
		}


	if (! (value_p -> st_data_value_p))
		{
			value_p -> st_data_value_p = (double64 *) AllocMemory (sizeof (double64));

			if (! (value_p -> st_data_value_p))
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate double64 value");
					return false;
				}
		}

	* (value_p -> st_data_value_p) = d;

	return true;
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


bool CopySharedType (const SharedType src, SharedType *dest_p, const ParameterType pt)
{
	bool success_flag = false;

	switch (pt)
		{
			case PT_DIRECTORY:
			case PT_FILE_TO_READ:
			case PT_FILE_TO_WRITE:
				{
					if (src.st_resource_value_p)
						{
							Resource *dest_res_p = CloneResource (src.st_resource_value_p);

							if (dest_res_p)
								{
									if (dest_p -> st_resource_value_p)
										{
											FreeResource (dest_p -> st_resource_value_p);
										}

									dest_p -> st_resource_value_p = dest_res_p;
									success_flag = true;
								}
						}
					else
						{
							if (dest_p -> st_resource_value_p)
								{
									FreeResource (dest_p -> st_resource_value_p);
									dest_p -> st_resource_value_p = NULL;
								}

							success_flag = true;
						}

				}
				break;

			case PT_TABLE:
			case PT_STRING:
			case PT_LARGE_STRING:
			case PT_PASSWORD:
			case PT_KEYWORD:
			case PT_FASTA:
				{
					if (src.st_string_value_s)
						{
							char *copied_value_s = EasyCopyToNewString (src.st_string_value_s);

							if (copied_value_s)
								{
									if (dest_p -> st_string_value_s)
										{
											FreeCopiedString (dest_p -> st_string_value_s);
										}

									dest_p -> st_string_value_s = copied_value_s;
									success_flag = true;
								}
							else
								{

								}
						}
					else
						{
							if (dest_p -> st_string_value_s)
								{
									FreeCopiedString (dest_p -> st_string_value_s);
									dest_p -> st_string_value_s = NULL;
								}

							success_flag = true;
						}
				}
				break;

			case PT_JSON:
				{
					if (src.st_json_p)
						{
							json_t *copied_value_p = json_deep_copy (src.st_json_p);

							if (copied_value_p)
								{
									if (dest_p -> st_json_p)
										{
											json_decref (dest_p -> st_json_p);
										}

									dest_p -> st_json_p = copied_value_p;
									success_flag = true;
								}
						}
					else
						{
							if (dest_p -> st_json_p)
								{
									json_decref (dest_p -> st_json_p);
									dest_p -> st_json_p = NULL;
								}

							success_flag = true;
						}
				}
				break;

			case PT_SIGNED_INT:
			case PT_NEGATIVE_INT:
				success_flag = CopySharedTypeSignedIntValue (&src, dest_p);
				break;

			case PT_UNSIGNED_INT:
				success_flag = CopySharedTypeUnsignedIntValue (&src, dest_p);
				break;

			case PT_UNSIGNED_REAL:
			case PT_SIGNED_REAL:
				success_flag = CopySharedTypeRealValue (&src, dest_p);
				break;

			case PT_CHAR:
				success_flag = CopySharedTypeCharValue (&src, dest_p);
				break;

			case PT_BOOLEAN:
				success_flag = CopySharedTypeBooleanValue (&src, dest_p);
				break;


			case PT_TIME:
				if (dest_p -> st_time_p)
					{
						CopyTime (src.st_time_p, dest_p -> st_time_p);
					}
				else
					{
						if ((dest_p -> st_time_p = DuplicateTime (src.st_time_p)) != NULL)
							{
								success_flag = true;
							}
						else
							{
								PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to copy time value");
							}
					}
				break;

			case PT_NUM_TYPES:
				PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Parameter has invalid type");
				break;
		}


	return success_flag;
}



SharedTypeNode *AllocateSharedTypeNode (SharedType value)
{
	SharedTypeNode *node_p = (SharedTypeNode *) AllocMemory (sizeof (SharedTypeNode));

	if (node_p)
		{
			node_p -> stn_node.ln_prev_p = NULL;
			node_p -> stn_node.ln_next_p = NULL;

			memcpy (& (node_p -> stn_value), &value, sizeof (SharedType));
		}

	return node_p;
}


bool CopySharedTypeRealValue (const SharedType *src_p, SharedType *dest_p)
{
	bool success_flag = false;

	if (src_p -> st_data_value_p)
		{
			if (! (dest_p -> st_data_value_p))
				{
					dest_p -> st_data_value_p = (double64 *) AllocMemory (sizeof (double64));

					if (! (dest_p -> st_data_value_p))
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate double variable");
						}
				}

			if (dest_p -> st_data_value_p)
				{
					* (dest_p -> st_data_value_p) = * (src_p -> st_data_value_p);
					success_flag = true;
				}
		}
	else
		{
			if (dest_p -> st_data_value_p)
				{
					FreeMemory (dest_p -> st_data_value_p);
					dest_p -> st_data_value_p = NULL;
				}

			success_flag = true;
		}

	return success_flag;
}



bool CopySharedTypeUnsignedIntValue (const SharedType *src_p, SharedType *dest_p)
{
	bool success_flag = false;

	if (src_p -> st_ulong_value_p)
		{
			if (! (dest_p -> st_ulong_value_p))
				{
					dest_p -> st_ulong_value_p = (uint32 *) AllocMemory (sizeof (uint32));

					if (! (dest_p -> st_ulong_value_p))
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate uint32 variable");
						}
				}

			if (dest_p -> st_ulong_value_p)
				{
					* (dest_p -> st_ulong_value_p) = * (src_p -> st_ulong_value_p);
					success_flag = true;
				}
		}
	else
		{
			if (dest_p -> st_ulong_value_p)
				{
					FreeMemory (dest_p -> st_ulong_value_p);
					dest_p -> st_ulong_value_p = NULL;
				}

			success_flag = true;
		}

	return success_flag;
}


bool CopySharedTypeSignedIntValue (const SharedType *src_p, SharedType *dest_p)
{
	bool success_flag = false;

	if (src_p -> st_long_value_p)
		{
			if (! (dest_p -> st_long_value_p))
				{
					dest_p -> st_long_value_p = (int32 *) AllocMemory (sizeof (int32));

					if (! (dest_p -> st_long_value_p))
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate int32 variable");
						}
				}

			if (dest_p -> st_long_value_p)
				{
					* (dest_p -> st_long_value_p) = * (src_p -> st_long_value_p);
					success_flag = true;
				}
		}
	else
		{
			if (dest_p -> st_long_value_p)
				{
					FreeMemory (dest_p -> st_long_value_p);
					dest_p -> st_long_value_p = NULL;
				}

			success_flag = true;
		}

	return success_flag;
}



bool CopySharedTypeBooleanValue (const SharedType *src_p, SharedType *dest_p)
{
	bool success_flag = false;

	if (src_p -> st_boolean_value_p)
		{
			success_flag = SetSharedTypeBooleanValue (dest_p, * (src_p -> st_boolean_value_p));
		}
	else
		{
			if (dest_p -> st_boolean_value_p)
				{
					FreeMemory (dest_p -> st_boolean_value_p);
					dest_p -> st_boolean_value_p = NULL;
				}

			success_flag = true;
		}

	return success_flag;
}


bool CopySharedTypeCharValue (const SharedType *src_p, SharedType *dest_p)
{
	bool success_flag = false;

	if (src_p -> st_char_value_p)
		{
			success_flag = SetSharedTypeCharValue (dest_p, * (src_p -> st_char_value_p));
		}
	else
		{
			if (dest_p -> st_char_value_p)
				{
					FreeMemory (dest_p -> st_char_value_p);
					dest_p -> st_char_value_p = NULL;
				}

			success_flag = true;
		}

	return success_flag;
}


bool CompareSharedTypeBooleanValue (const SharedType *value_p, const bool value_to_check)
{
	bool equals_flag = false;

	if (value_p -> st_boolean_value_p)
		{
			if ((* (value_p -> st_boolean_value_p)) == value_to_check)
				{
					equals_flag = true;
				}
		}

	return equals_flag;
}


bool ClearAndSetSharedTypeValue (SharedType *st_p, const void *value_p, const ParameterType pt)
{
	ClearSharedType (st_p, pt);

	return SetSharedTypeValue (st_p, value_p, pt);
}


bool SetSharedTypeValue (SharedType *st_p, const void *value_p, const ParameterType pt)
{
	bool success_flag = false;

	if (value_p)
		{
			switch (pt)
				{
					case PT_BOOLEAN:
						{
							const bool *b_p = (const bool *) value_p;
							success_flag = SetSharedTypeBooleanValue (st_p, *b_p);
						}
						break;

					case PT_CHAR:
						{
							const char *c_p = (const char *) value_p;
							success_flag = SetSharedTypeCharValue (st_p, *c_p);
						}
						break;


					case PT_SIGNED_INT:
					case PT_NEGATIVE_INT:
						{
							const int32 *i_p = (const int32 *) value_p;
							success_flag = SetSharedTypeSignedIntValue (st_p, *i_p, NULL);
						}
						break;

					case PT_UNSIGNED_INT:
						{
							const uint32 *i_p = (const uint32 *) value_p;
							success_flag = SetSharedTypeUnsignedIntValue (st_p, *i_p, NULL);
						}
						break;

					case PT_SIGNED_REAL:
					case PT_UNSIGNED_REAL:
						{
							const double *d_p = (const double *) value_p;
							success_flag = SetSharedTypeRealValue (st_p, *d_p, NULL);
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
						PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "SharedType has invalid type %d", pt);
						break;
				}

		}
	else
		{
			success_flag = true;
		}

	return success_flag;
}
