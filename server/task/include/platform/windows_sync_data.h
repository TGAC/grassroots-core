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
 * windows_sync_data.h
 *
 *  Created on: 07 Feb 2023
 *      Author: billy
 */

#ifndef CORE_SERVER_TASK_INCLUDE_PLATFORM_WINDOWS_SYNC_DATA_H_
#define CORE_SERVER_TASK_INCLUDE_PLATFORM_WINDOWS_SYNC_DATA_H_

#include <windows.h>

#include "sync_data.h"

struct SyncData
{
	/** @private */
	//HANDLE sd_mutex_p;
	CRITICAL_SECTION sd_lock;
	
	/** @private */
	CONDITION_VARIABLE sd_cond;
};

#endif /* CORE_SERVER_TASK_INCLUDE_PLATFORM_WINDOWS_SYNC_DATA_H_ */
