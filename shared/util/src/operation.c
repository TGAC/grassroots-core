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
 * operation.c
 *
 *  Created on: 21 Aug 2015
 *      Author: billy
 */

#include <string.h>

#include "operation.h"
#include "streams.h"
#include "schema_keys.h"
#include "math_utils.h"
#include "json_util.h"


static const char * const S_FAILED_S = "Failed";
static const char * const S_FAILED_TO_START_S = "Failed to start";
static const char * const S_ERROR_S = "Error";
static const char * const S_IDLE_S = "Idle";
static const char * const S_PENDING_S = "Pending";
static const char * const S_STARTED_S = "Started";
static const char * const S_FINISHED_S = "Finished";
static const char * const S_PARTIALLY_SUCCEEDED_S = "Partially succeeded";
static const char * const S_SUCCEEDED_S = "Succeeded";
static const char * const S_CLEANED_UP_S = "Cleaned up";


static const char *S_OPERATIONS_SS [OP_NUM_OPERATIONS] =
{
	"get_all_services",
	"get_schema_version",
	"get_interested_services",
	"run_keyword_services",
	"get_named_service",
	"get_service_results",
	"get_resource",
	"get_server_status",
	"get_service_info"
};




Operation GetOperationFromJSON (const json_t *ops_p)
{
	Operation op = OP_NONE;
	const char *op_s = GetJSONString (ops_p, OPERATION_S);

	if (op_s)
		{
			op = GetOperationFromString (op_s);

			if (op == OP_NONE)
				{
					PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, ops_p, "Failed to get valid operation value");
				}
		}
	else
		{
			PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, ops_p, "%s value is NULL", OPERATION_S);
		}

	if (op == OP_NONE)
		{
			json_t *op_p = json_object_get (ops_p, OPERATION_ID_S);

			if (!op_p)
				{
					op_p = json_object_get (ops_p, OPERATION_ID_OLD_S);
				}

			if (op_p)
				{
					if (json_is_integer (op_p))
						{
							op = json_integer_value (op_p);
						}
					else if (json_is_string (op_p))
						{
							const char *value_s = json_string_value (op_p);
							int i;

							if (GetValidInteger (&value_s, &i))
								{
									op = i;
								}
						}
				}

		}

	return op;
}


const char *GetOperationAsString (const Operation op)
{
	const char *op_s = NULL;

	if ((op > OP_NONE) && (op < OP_NUM_OPERATIONS))
		{
			op_s = * (S_OPERATIONS_SS + op);
		}

	return op_s;
}



Operation GetOperationFromString (const char *op_s)
{
	Operation op;

	for (op = OP_NONE + 1; op < OP_NUM_OPERATIONS; ++ op)
		{
			if (strcmp (* (S_OPERATIONS_SS + op), op_s) == 0)
				{
					return op;
				}
		}

	return OP_NONE;
}


const char *GetOperationStatusAsString (const OperationStatus status)
{
	const char *result_s = NULL;

	switch (status)
		{
			case OS_FAILED:
				result_s = S_FAILED_S;
				break;

			case OS_FAILED_TO_START:
				result_s = S_FAILED_TO_START_S;
				break;

			case OS_ERROR:
				result_s = S_ERROR_S;
				break;

			case OS_IDLE:
				result_s = S_IDLE_S;
				break;

			case OS_PENDING:
				result_s = S_PENDING_S;
				break;

			case OS_STARTED:
				result_s = S_STARTED_S;
				break;

			case OS_FINISHED:
				result_s = S_FINISHED_S;
				break;

			case OS_PARTIALLY_SUCCEEDED:
				result_s = S_PARTIALLY_SUCCEEDED_S;
				break;

			case OS_SUCCEEDED:
				result_s = S_SUCCEEDED_S;
				break;

			case OS_CLEANED_UP:
				result_s = S_CLEANED_UP_S;
				break;

			default:
				break;
		}

	return result_s;
}


OperationStatus GetOperationStatusFromString (const char *status_s)
{
	OperationStatus status = OS_NUM_STATUSES;

	if (status_s)
		{
			if (strcmp (status_s, S_FAILED_S) == 0)
				{
					status = OS_FAILED;
				}
			else if (strcmp (status_s, S_FAILED_TO_START_S) == 0)
				{
					status = OS_FAILED_TO_START;
				}
			else if (strcmp (status_s, S_ERROR_S) == 0)
				{
					status = OS_ERROR;
				}
			else if (strcmp (status_s, S_IDLE_S) == 0)
				{
					status = OS_IDLE;
				}
			else if (strcmp (status_s, S_PENDING_S) == 0)
				{
					status = OS_PENDING;
				}
			else if (strcmp (status_s, S_STARTED_S) == 0)
				{
					status = OS_STARTED;
				}
			else if (strcmp (status_s, S_FINISHED_S) == 0)
				{
					status = OS_FINISHED;
				}
			else if (strcmp (status_s, S_PARTIALLY_SUCCEEDED_S) == 0)
				{
					status = OS_PARTIALLY_SUCCEEDED;
				}
			else if (strcmp (status_s, S_SUCCEEDED_S) == 0)
				{
					status = OS_SUCCEEDED;
				}
			else if (strcmp (status_s, S_CLEANED_UP_S) == 0)
				{
					status = OS_CLEANED_UP;
				}
		}		/* if (status_s) */

	return status;
}
