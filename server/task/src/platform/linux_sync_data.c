/*
 * unix_sync_data.c
 *
 *  Created on: 19 May 2017
 *      Author: billy
 */

#include <errno.h>

#include "linux_sync_data.h"
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
			int res = pthread_mutex_init (& (data_p -> sd_mutex), NULL);

			if (res == 0)
				{
					res = pthread_cond_init (& (data_p -> sd_cond), NULL);

					if (res == 0)
						{
							return data_p;
						}		/* if (res == 0) */
					else
						{
							switch (res)
								{
									case ENOMEM:
										PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create AsyncTaskProducer cond: "
											"Insufficient memory exists to initialise the condition variable.");
										break;

									case EAGAIN:
										PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create AsyncTaskProducer cond: The system"
											"lacked the necessary resources (other than memory) to initialise another condition variable");
										break;

									case EBUSY:
										PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create AsyncTaskProducer cond: "
											"The implementation has detected an attempt to re-initialise the object referenced by cond,"
											" a previously initialised, but not yet destroyed, condition variable.");
										break;

									case EINVAL:
										PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create AsyncTaskProducer cond: The value specified by attr is invalid.");
										break;

									default:
										PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create AsyncTaskProducer cond, %d", res);
										break;
								}
						}

					pthread_mutex_destroy (& (data_p -> sd_mutex));
				}		/* if (res == 0) */
			else
				{
					switch (res)
						{
							case EAGAIN:
								PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create AsyncTaskProducer mutex: "
									"The system lacked the necessary resources (other than memory) to initialise another mutex.");
								break;

							case ENOMEM:
								PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create AsyncTaskProducer mutex: The system"
									"Insufficient memory exists to initialise the mutex.");
								break;

							case EPERM:
								PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create AsyncTaskProducer mutex: "
									"The caller does not have the privilege to perform the operation.");
								break;

							case EBUSY:
								PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create AsyncTaskProducer mutex: "
									"The implementation has detected an attempt to re-initialise the object referenced by mutex, "
									"a previously initialised, but not yet destroyed, mutex.");
								break;

							case EINVAL:
								PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create AsyncTaskProducer mutex: The value specified by attr is invalid.");
								break;

							default:
								PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create AsyncTaskProducer mutex, %d", res);
								break;

						}		/* switch (res) */

				}

			FreeMemory (data_p);
		}		/* if (data_p) */

	return NULL;

}



void FreeSyncData (struct SyncData *sync_data_p)
{
	int res = pthread_cond_destroy (& (sync_data_p -> sd_cond));

	if (res != 0)
		{
			switch (res)
				{
					case ENOMEM:
						PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Error in FreeAsyncTaskProducer cond: "
							"Insufficient memory exists to initialise the condition variable.");
						break;

					case EAGAIN:
						PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Error in FreeAsyncTaskProducer cond: The system"
							"lacked the necessary resources (other than memory) to initialise another condition variable");
						break;

					case EBUSY:
						PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Error in FreeAsyncTaskProducer cond: "
							"The implementation has detected an attempt to re-initialise the object referenced by cond,"
							" a previously initialised, but not yet destroyed, condition variable.");
						break;

					case EINVAL:
						PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Error in FreeAsyncTaskProducer cond: The value specified by attr is invalid.");
						break;

					default:
						PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Error in FreeAsyncTaskProducer cond: %d", res);
						break;
				}
		}

	res = pthread_mutex_destroy (& (sync_data_p -> sd_mutex));

	if (res != 0)
		{
			switch (res)
				{
					case EBUSY:
						PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Error in FreeAsyncTaskProducer mutex: "
							"The implementation has detected an attempt to destroy the object referenced by mutex while"
							" it is locked or referenced (for example, while being used in a pthread_cond_wait() or "
							"pthread_cond_timedwait()) by another thread.");
						break;

					case EINVAL:
						PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Error in FreeAsyncTaskProducer mutex: The value specified by mutex is invalid.");
						break;

					default:
						PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Error in FreeAsyncTaskProducer mutex: %d", res);
						break;
				}
		}

	FreeMemory (sync_data_p);
}




bool AcquireSyncDataLock (struct SyncData *sync_data_p)
{
	bool success_flag = false;
	int res = pthread_mutex_lock (& (sync_data_p -> sd_mutex));

	if (res == 0)
		{
			success_flag = true;
		}
	else
		{
			switch (res)
				{
					case EINVAL:
						PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to lock AsyncTaskProducer: Either:\n"
							"The mutex was created with the protocol attribute having the value PTHREAD_PRIO_PROTECT and "
							"the calling thread's priority is higher than the mutex's current priority ceiling. or n"
							"The value specified by mutex does not refer to an initialised mutex object.");
						break;

					case EAGAIN:
						PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to lock AsyncTaskProducer: The mutex "
								"could not be acquired because the maximum number of recursive locks for mutex has been exceeded.");
						break;

					case EDEADLK:
						PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to lock AsyncTaskProducer: The current thread already owns the mutex.");
						break;

					default:
						PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to lock AsyncTaskProducer, %d", res);
						break;
				}
		}

	#if UNIX_SYNC_DATA_DEBUG >= STM_LEVEL_FINER
	PrintLog (STM_LEVEL_FINER, __FILE__, __LINE__, "AcquireSyncDataLock %d", success_flag);
	#endif


	return success_flag;

}


bool ReleaseSyncDataLock (struct SyncData *sync_data_p)
{
	bool success_flag = false;
	int res = pthread_mutex_unlock (& (sync_data_p -> sd_mutex));

	if (res == 0)
		{
			success_flag = true;
		}
	else
		{
			switch (res)
				{
					case EBUSY:
						PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to unlock AsyncTaskProducer: "
							"The implementation has detected an attempt to destroy the object referenced by cond while"
							" it is referenced (for example, while being used in a pthread_cond_wait() or "
							"pthread_cond_timedwait()) by another thread.");
						break;

					case EINVAL:
						PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to unlock AsyncTaskProducer: The value specified by cond is invalid.");
						break;

					default:
						PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to lock AsyncTaskProducer, %d", res);
						break;
				}
		}


	#if UNIX_SYNC_DATA_DEBUG >= STM_LEVEL_FINER
	PrintLog (STM_LEVEL_FINER, __FILE__, __LINE__, "ReleaseSyncDataLock %d", success_flag);
	#endif


	return success_flag;
}


void WaitOnSyncData (struct SyncData *sync_data_p, bool (*continue_fn) (void *data_p), void *data_p)
{
	if (AcquireSyncDataLock (sync_data_p))
		{
			while (continue_fn (data_p))
				{
					pthread_cond_wait (& (sync_data_p -> sd_cond), & (sync_data_p -> sd_mutex));
				}

			#if UNIX_SYNC_DATA_DEBUG >= STM_LEVEL_FINER
			PrintLog (STM_LEVEL_FINER, __FILE__, __LINE__, "WaitOnSyncData finished");
			#endif


			if (!ReleaseSyncDataLock (sync_data_p))
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to unlock SyncData lock");
				}
		}
}


void SendSyncData (struct SyncData *sync_data_p)
{
	if (AcquireSyncDataLock (sync_data_p))
		{
			/* send signal */
			pthread_cond_signal (& (sync_data_p -> sd_cond));

			if (!ReleaseSyncDataLock (sync_data_p))
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to unlock SyncData lock");
				}
		}
}
