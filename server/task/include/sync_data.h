/*
** Copyright 2014-2017 The Earlham Institute
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

/**
 * @file
 * @brief
 */

#ifndef GRASSROOTS_TASK_SYNC_DATA_H
#define GRASSROOTS_TASK_SYNC_DATA_H

#include "grassroots_task_library.h"
#include "typedefs.h"


/* forward declaration */
struct SyncData;
typedef struct SyncData SyncData;

#ifdef __cplusplus
extern "C"
{
#endif



GRASSROOTS_TASK_API struct SyncData *AllocateSyncData (void);


GRASSROOTS_TASK_API void FreeSyncData (struct SyncData *sync_data_p);


GRASSROOTS_TASK_API bool AcquireSyncDataLock (struct SyncData *sync_data_p);


GRASSROOTS_TASK_API bool ReleaseSyncDataLock (struct SyncData *sync_data_p);


GRASSROOTS_TASK_API void WaitOnSyncData (struct SyncData *sync_data_p, bool (*continue_fn) (void *data_p), void *data_p);


GRASSROOTS_TASK_API void SendSyncData (struct SyncData *sync_data_p);



#ifdef __cplusplus
}
#endif


#endif 	/* #ifndef GRASSROOTS_TASK_SYNC_DATA_H */


