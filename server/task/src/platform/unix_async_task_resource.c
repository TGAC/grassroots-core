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

struct AsyncTaskResource
{
	pthread_mutex_t atr_mutex;
	pthread_cond_t atr_cond;
	bool (*atr_test_fn) (struct AsyncTaskResource *resource_p, void *data_p);
	bool (*atr_fire_fn) (struct AsyncTaskResource *resource_p, void *data_p);
};


typedef struct CountingAsyncTaskResource
{
	struct AsyncTaskResource catr_base_resource;
	int32 catr_current_value;
	int32 catr_limit;
} CountingAsyncTaskResource;




bool IncrementCountingAsyncTaskResource (struct CountingAsyncTaskResource *resource_p)
{
	bool success_flag = false;

	if (LockAsyncTaskMonitorResource (& (resource_p -> catr_base_resource)))
		{
			++ (resource_p -> catr_current_value);
			success_flag = true;

			if (resource_p -> catr_current_value == resource_p -> catr_limit)
				{
					/* send signal */
				}

			if (!UnlockAsyncTaskMonitorResource (& (resource_p -> catr_base_resource)))
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to unlock CountingAsyncTaskResource mutex");
				}
		}

	return success_flag;
}



struct AsyncTaskResource *CreateAsyncTaskResource (void)
{
	struct AsyncTaskResource *resource_p = (struct AsyncTaskResource *) AllocMemory (sizeof (struct AsyncTaskResource));

	if (resource_p)
		{
			int res = pthread_mutex_init (& (resource_p -> atr_mutex), NULL);

			if (res == 0)
				{
					res = pthread_cond_init (& (resource_p -> atr_cond), NULL);

					if (res == 0)
						{
							return resource_p;
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

					pthread_mutex_destroy (& (resource_p -> atr_mutex));
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
	int res = pthread_cond_destroy (& (resource_p -> atr_cond));

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


	res = pthread_mutex_destroy (& (resource_p -> atr_mutex));

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

	FreeMemory (resource_p);
}





bool LockAsyncTaskMonitorResource (struct AsyncTaskResource *resource_p)
{
	bool success_flag = false;
	int res = pthread_mutex_lock (& (resource_p -> atr_mutex));

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


bool UnlockAsyncTaskMonitorResource (struct AsyncTaskResource *resource_p)
{
	bool success_flag = false;
	int res = pthread_mutex_unlock (& (resource_p -> atr_mutex));

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
