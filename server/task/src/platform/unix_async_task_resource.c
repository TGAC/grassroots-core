/*
 * unix_async_task_resource.c
 *
 *  Created on: 9 May 2017
 *      Author: billy
 */


#include <errno.h>

#include "pthread.h"

#include "typedefs.h"
#include "streams.h"
#include "async_task_resource.h"
#include "memory_allocations.h"


typedef struct UnixAsyncTaskResource
{
	AsyncTaskResource uatr_base_resource;
	pthread_mutex_t uatr_task_status_mutex;
	pthread_cond_t uatr_task_status_cond;
} UnixAsyncTaskResource;




AsyncTaskResource *CreateAsyncTaskResource (void)
{
	UnixAsyncTaskResource *resource_p = (UnixAsyncTaskResource *) AllocMemory (sizeof (UnixAsyncTaskResource));

	if (resource_p)
		{
			if (InitAsyncTaskResource (& (resource_p -> uatr_base_resource)))
				{
					int res = pthread_mutex_init (& (resource_p -> uatr_task_status_mutex), NULL);

					if (res == 0)
						{
							res = pthread_cond_init (& (resource_p -> uatr_task_status_cond), NULL);

							if (res == 0)
								{
									return (& (resource_p -> uatr_base_resource));
								}		/* if (res == 0) */
							else
								{
									switch (res)
										{
											case ENOMEM:
												PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create AsyncTaskResource cond: "
													"Insufficient memory exists to initialise the condition variable.");
												break;

											case EAGAIN:
												PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create AsyncTaskResource cond: The system"
													"lacked the necessary resources (other than memory) to initialise another condition variable");
												break;

											case EBUSY:
												PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create AsyncTaskResource cond: "
													"The implementation has detected an attempt to re-initialise the object referenced by cond,"
													" a previously initialised, but not yet destroyed, condition variable.");
												break;

											case EINVAL:
												PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create AsyncTaskResource cond: The value specified by attr is invalid.");
												break;

											default:
												PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create AsyncTaskResource cond, %d", res);
												break;
										}
								}

							pthread_mutex_destroy (& (resource_p -> uatr_task_status_mutex));
						}		/* if (res == 0) */
					else
						{
							switch (res)
								{
									case EAGAIN:
										PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create AsyncTaskResource mutex: "
											"The system lacked the necessary resources (other than memory) to initialise another mutex.");
										break;

									case ENOMEM:
										PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create AsyncTaskResource mutex: The system"
											"Insufficient memory exists to initialise the mutex.");
										break;

									case EPERM:
										PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create AsyncTaskResource mutex: "
											"The caller does not have the privilege to perform the operation.");
										break;

									case EBUSY:
										PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create AsyncTaskResource mutex: "
											"The implementation has detected an attempt to re-initialise the object referenced by mutex, "
											"a previously initialised, but not yet destroyed, mutex.");
										break;

									case EINVAL:
										PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create AsyncTaskResource mutex: The value specified by attr is invalid.");
										break;

									default:
										PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create AsyncTaskResource mutex, %d", res);
										break;
								}

						}

				}		/* if (InitAsyncTaskResource (& (resource_p -> uatr_base_resource))) */

			FreeMemory (resource_p);
		}		/* if (resource_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate memory for AsyncTaskResource");
		}

	return NULL;
}


void FreeAsyncTaskResource (struct AsyncTaskResource *resource_p)
{
	UnixAsyncTaskResource *unix_resource_p = (UnixAsyncTaskResource *) resource_p;

	int res = pthread_cond_destroy (& (unix_resource_p -> uatr_task_status_cond));

	if (res != 0)
		{
			switch (res)
				{
					case ENOMEM:
						PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Error in FreeAsyncTaskResource cond: "
							"Insufficient memory exists to initialise the condition variable.");
						break;

					case EAGAIN:
						PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Error in FreeAsyncTaskResource cond: The system"
							"lacked the necessary resources (other than memory) to initialise another condition variable");
						break;

					case EBUSY:
						PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Error in FreeAsyncTaskResource cond: "
							"The implementation has detected an attempt to re-initialise the object referenced by cond,"
							" a previously initialised, but not yet destroyed, condition variable.");
						break;

					case EINVAL:
						PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Error in FreeAsyncTaskResource cond: The value specified by attr is invalid.");
						break;

					default:
						PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Error in FreeAsyncTaskResource cond: %d", res);
						break;
				}
		}


	res = pthread_mutex_destroy (& (unix_resource_p -> uatr_task_status_mutex));

	if (res != 0)
		{
			switch (res)
				{
					case EBUSY:
						PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Error in FreeAsyncTaskResource mutex: "
							"The implementation has detected an attempt to destroy the object referenced by mutex while"
							" it is locked or referenced (for example, while being used in a pthread_cond_wait() or "
							"pthread_cond_timedwait()) by another thread.");
						break;

					case EINVAL:
						PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Error in FreeAsyncTaskResource mutex: The value specified by mutex is invalid.");
						break;

					default:
						PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Error in FreeAsyncTaskResource mutex: %d", res);
						break;
				}
		}

	ClearAsyncTaskResource (resource_p);

	FreeMemory (unix_resource_p);
}


bool LockAsyncTaskResource (AsyncTaskResource *resource_p)
{
	bool success_flag = false;
	UnixAsyncTaskResource *unix_resource_p = (UnixAsyncTaskResource *) resource_p;
	int res = pthread_mutex_lock (& (unix_resource_p -> uatr_task_status_mutex));

	if (res == 0)
		{
			success_flag = true;
		}
	else
		{
			switch (res)
				{
					case EINVAL:
						PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to lock AsyncTaskResource: Either:\n"
							"The mutex was created with the protocol attribute having the value PTHREAD_PRIO_PROTECT and "
							"the calling thread's priority is higher than the mutex's current priority ceiling. or n"
							"The value specified by mutex does not refer to an initialised mutex object.");
						break;

					case EAGAIN:
						PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to lock AsyncTaskResource: The mutex "
								"could not be acquired because the maximum number of recursive locks for mutex has been exceeded.");
						break;

					case EDEADLK:
						PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to lock AsyncTaskResource: The current thread already owns the mutex.");
						break;

					default:
						PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to lock AsyncTaskResource, %d", res);
						break;
				}
		}

	return success_flag;
}


bool UnlockAsyncTaskResource (AsyncTaskResource *resource_p)
{
	bool success_flag = false;
	UnixAsyncTaskResource *unix_resource_p = (UnixAsyncTaskResource *) resource_p;
	int res = pthread_mutex_unlock (& (unix_resource_p -> uatr_task_status_mutex));

	if (res == 0)
		{
			success_flag = true;
		}
	else
		{
			switch (res)
				{
					case EBUSY:
						PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to unlock AsyncTaskResource: "
							"The implementation has detected an attempt to destroy the object referenced by cond while"
							" it is referenced (for example, while being used in a pthread_cond_wait() or "
							"pthread_cond_timedwait()) by another thread.");
						break;

					case EINVAL:
						PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to unlock AsyncTaskResource: The value specified by cond is invalid.");
						break;

					default:
						PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to lock AsyncTaskResource, %d", res);
						break;
				}
		}

	return success_flag;
}


void FireAsyncTaskResource (AsyncTaskResource *resource_p)
{
	UnixAsyncTaskResource *unix_resource_p = (UnixAsyncTaskResource *) resource_p;

	pthread_cond_signal (& (unix_resource_p -> uatr_task_status_cond));
}


void WaitOnAsyncTaskResource (AsyncTaskResource *resource_p)
{
	UnixAsyncTaskResource *unix_resource_p = (UnixAsyncTaskResource *) resource_p;

	if (LockAsyncTaskResource (resource_p))
		{
			while (resource_p -> atr_continue_fn (resource_p))
				{
					pthread_cond_wait (& (unix_resource_p -> uatr_task_status_cond), & (unix_resource_p -> uatr_task_status_mutex));
				}

			if (!UnlockAsyncTaskResource (resource_p))
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to unlock CountingAsyncTaskResource mutex");
				}
		}
}



