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

#include "grassroots_service_library.h"
#include "typedefs.h"


/* forward declaration */

struct SyncData;

/**
 * A datatype that allows data to be shared between tasks (e.g. threads) in
 * a platform-agnsotic way. It allows to you lock some data so you may access
 * it in a thread-safe manner.
 */
typedef struct SyncData SyncData;

#ifdef __cplusplus
extern "C"
{
#endif


/**
 * Allocate a SyncData.
 *
 * @return The newly-allocated SyncData or <code>NULL</code> upon error.
 * @memberof SyncData
 */
GRASSROOTS_SERVICE_API struct SyncData *AllocateSyncData (void);


/**
 * Free a SyncData.
 *
 * @param sync_data_p The SyncData to free.
 * @memberof SyncData
 */
GRASSROOTS_SERVICE_API void FreeSyncData (struct SyncData *sync_data_p);


/**
 * Lock a SyncData to allow for thread-safe access. After you have finished
 * accessing the SyncData, you must release it using ReleaseSyncDataLock().
 *
 * @param sync_data_p The SyncData to lock.
 * @return <code>true</code> if the lock was acquired successfully, <code>
 * false</code> otherwise.
 * @memberof SyncData
 */
GRASSROOTS_SERVICE_API bool AcquireSyncDataLock (struct SyncData *sync_data_p);


/**
 * Release the lock non a given SyncData
 *
 * @param sync_data_p The SyncData to release the lock for.
 * @return <code>true</code> if the lock was released successfully, <code>
 * false</code> otherwise.
 * @memberof SyncData
 */
GRASSROOTS_SERVICE_API bool ReleaseSyncDataLock (struct SyncData *sync_data_p);


/**
 * Wait for a SyncData condition to be met using a given function to check for the
 * condition.
 *
 * @param sync_data_p The SyncData to wait.
 * @param continue_fn The function used to check whether the condition has been met. This
 * will return <code>true</code> when the condition has not been met yet and the waiting
 * should continue. This function will be called with the custom parameter data_p
 * @param data_p The parameter to use as input for continue_fn if needed.
 * @memberof SyncData
 */
GRASSROOTS_SERVICE_API void WaitOnSyncData (struct SyncData *sync_data_p, bool (*continue_fn) (void *data_p), void *data_p);


/**
 * Signal that a condition has been met.
 *
 * @param sync_data_p The SyncData to send the signal from.
 * @memberof SyncData
 */
GRASSROOTS_SERVICE_API void SendSyncData (struct SyncData *sync_data_p);



#ifdef __cplusplus
}
#endif


#endif 	/* #ifndef GRASSROOTS_TASK_SYNC_DATA_H */


