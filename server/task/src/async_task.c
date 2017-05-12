/*
 * async_task.c
 *
 *  Created on: 12 May 2017
 *      Author: billy
 */


#include "async_task.h"

#include "string_utils.h"
#include "streams.h"


bool InitialiseAsyncTask (AsyncTask *task_p, const char *name_s)
{
	bool success_flag = true;
	char *copied_name_s = NULL;

	if (name_s)
		{
			copied_name_s = CopyToNewString (name_s, 0, false);

			if (!copied_name_s)
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to copy AsyncTask name \"%s\"", name_s);
					success_flag = false;
				}
		}

	if (success_flag)
		{
			if (task_p -> at_name_s)
				{
					FreeCopiedString (task_p -> at_name_s);
				}

			task_p -> at_name_s = copied_name_s;
		}

	return success_flag;
}


void ClearAsyncTask (AsyncTask *task_p)
{
	if (task_p -> at_name_s)
		{
			FreeCopiedString (task_p -> at_name_s);
			task_p -> at_name_s = NULL;
		}
}

