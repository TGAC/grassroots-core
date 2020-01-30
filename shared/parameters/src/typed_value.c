/*
** Copyright 2014-2020 The Earlham Institute
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
 * typed_value.c
 *
 *  Created on: 29 Jan 2020
 *      Author: billy
 */


#include "typed_value.h"
#include "memory_allocations.h"



TypedValue *AllocateTypedValue (const ParameterType pt)
{
	TypedValue *tv_p = (TypedValue *) AllocMemory (sizeof (TypedValue));

	if (tv_p)
		{
			InitSharedType (& (tv_p -> tv_value));
			tv_p -> tv_param_type = pt;

			return tv_p;
		}

	return NULL;
}



bool SetTypedValueTypeAndValue (TypedValue *tv_p, const ParameterType pt, void *value_p, ParameterBounds *bounds_p)
{
	bool success_flag = false;

	if (tv_p -> tv_param_type != pt)
		{
			ClearSharedType (& (tv_p -> tv_value), tv_p -> tv_param_type);
			tv_p -> tv_param_type = pt;
		}

	success_flag = SetSharedTypeValue (& (tv_p -> tv_value), pt, value_p, bounds_p);


	return success_flag;
}



void FreeTypedValue (TypedValue *tv_p)
{
	ClearSharedType (& (tv_p -> tv_value), tv_p -> tv_param_type);
	FreeMemory (tv_p);
}
