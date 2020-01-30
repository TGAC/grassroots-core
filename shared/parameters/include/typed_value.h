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
 * typed_value.h
 *
 *  Created on: 29 Jan 2020
 *      Author: billy
 */

#ifndef CORE_SHARED_PARAMETERS_INCLUDE_TYPED_VALUE_H_
#define CORE_SHARED_PARAMETERS_INCLUDE_TYPED_VALUE_H_

#include "grassroots_params_library.h"

#include "shared_type.h"
#include "parameter_type.h"


typedef struct TypedValue
{
	SharedType tv_value;

	ParameterType tv_param_type;
} TypedValue;


#ifdef __cplusplus
extern "C"
{
#endif


/**
 * Allocate a TypedValue with the given ParameterType.
 *
 * @param pt The ParameterType for this TypedValue
 * @memberof TypedValue
 */
GRASSROOTS_PARAMS_API TypedValue *AllocateTypedValue (const ParameterType pt);


/**
 * Free a TypedValue
 *
 * @param tv_p The TypedValue to free
 * @memberof TypedValue
 */
GRASSROOTS_PARAMS_API void FreeTypedValue (TypedValue *tv_p);


/**
 * Change the ParameterType used by a TypedValue
 *
 * @param tv_p The TypedValue to change type for
 * @param pt The ParameterType to change the given TypedValue to.
 * @memberof TypedValue
 */
GRASSROOTS_PARAMS_API bool SetTypedValueType (TypedValue *tv_p, const ParameterType pt);


/**
 * Change the ParameterType used by a TypedValue and set a new value.
 *
 * @param tv_p The TypedValue to change type for
 * @param pt The ParameterType to change the given TypedValue to.
 * @memberof TypedValue
 */
GRASSROOTS_PARAMS_API bool SetTypedValueTypeAndValue (TypedValue *tv_p, const ParameterType pt, void *value_p);



#ifdef __cplusplus
}
#endif




#endif /* CORE_SHARED_PARAMETERS_INCLUDE_TYPED_VALUE_H_ */
