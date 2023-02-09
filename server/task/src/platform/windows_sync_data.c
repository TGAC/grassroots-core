/*
 * unix_sync_data.c
 *
 *  Created on: 19 May 2017
 *      Author: billy
 */

#include <errno.h>

#include "windows_sync_data.h"
#include "streams.h"
#include "memory_allocations.h"


#ifdef _DEBUG
	#define UNIX_SYNC_DATA_DEBUG	(STM_LEVEL_FINEST)
#else
	#define UNIX_SYNC_DATA_DEBUG	(STM_LEVEL_NONE)
#endif




SyncData *AllocateSyncData (void)
{
	SyncData *data_p = (SyncData *) AllocMemory (sizeof (SyncData));

	if (data_p)
		{
			InitializeConditionVariable (& (data_p -> sd_cond));
			InitializeCriticalSection (& (data_p -> sd_lock));

			return data_p;
		}		/* if (data_p) */

	return NULL;
}



void FreeSyncData (struct SyncData *sync_data_p)
{
	WakeAllConditionVariable (& (sync_data_p -> sd_cond));


	FreeMemory (sync_data_p);
}




bool AcquireSyncDataLock (struct SyncData *sync_data_p)
{
	EnterCriticalSection (& (sync_data_p -> sd_lock));

	return true;
}


bool ReleaseSyncDataLock (struct SyncData *sync_data_p)
{
	LeaveCriticalSection (& (sync_data_p -> sd_lock));

	return true;
}


void WaitOnSyncData (struct SyncData *sync_data_p, bool (*continue_fn) (void *data_p), void *data_p)
{
	if (AcquireSyncDataLock (sync_data_p))
		{
			bool b = true;

			while (b && continue_fn (data_p))
				{
					b = SleepConditionVariableCS (& (sync_data_p -> sd_cond), & (sync_data_p -> sd_lock), INFINITE);
				}

			#if WINDOWS_SYNC_DATA_DEBUG >= STM_LEVEL_FINER
			PrintLog (STM_LEVEL_FINER, __FILE__, __LINE__, "WaitOnSyncData finished");
			#endif


			if (!ReleaseSyncDataLock (sync_data_p))
				{
					DWORD res = GetLastError ();
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "ReleaseSyncDataLock () failed: %d", res);
				}
		}
}


void SendSyncData (struct SyncData *sync_data_p)
{
	if (AcquireSyncDataLock (sync_data_p))
		{
			/* send signal */
			WakeConditionVariable (& (sync_data_p -> sd_cond));
	
			if (!ReleaseSyncDataLock (sync_data_p))
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to unlock SyncData lock");
				}
		}
}
