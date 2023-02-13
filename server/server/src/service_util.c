/*
 * service_util.c
 *
 *  Created on: 13 Feb 2023
 *      Author: billy
 */

#include "service_util.h"



bool IsServiceLive (Service *service_p)
{
	bool is_live_flag = false;

	if (service_p -> se_jobs_p)
		{
			int32 num_jobs = GetNumberOfLiveJobs (service_p);

			is_live_flag = (num_jobs != 0);
		}

	return is_live_flag;
}



int32 GetNumberOfLiveJobs (Service *service_p)
{
	int32 num_live_jobs = 0;

	if (service_p -> se_jobs_p)
		{
			if ((!IsServiceLockable (service_p)) || (LockService (service_p)))
				{
					ServiceJobNode *node_p = (ServiceJobNode *) (service_p -> se_jobs_p -> sjs_jobs_p -> ll_head_p);

					while (node_p)
						{
							OperationStatus status = GetCachedServiceJobStatus (node_p -> sjn_job_p);

							if ((status == OS_PENDING) || (status == OS_STARTED))
								{
									++ num_live_jobs;
								}

							node_p = (ServiceJobNode *) (node_p -> sjn_node.ln_next_p);
						}

					if (! ((!IsServiceLockable (service_p)) || (UnlockService (service_p))))
						{
							num_live_jobs = -1;
						}
				}
			else
				{
					num_live_jobs = -1;
				}

		}		/* if (service_p -> se_jobs_p) */

	return num_live_jobs;
}




bool IsServiceLockable (const Service *service_p)
{
	return (service_p -> se_sync_data_p != NULL);
}


bool LockService (Service *service_p)
{
	bool success_flag = true;

	if (service_p -> se_sync_data_p)
		{
			success_flag = AcquireSyncDataLock (service_p -> se_sync_data_p);
		}

	return success_flag;
}


bool UnlockService (Service *service_p)
{
	bool success_flag = true;

	if (service_p -> se_sync_data_p)
		{
			success_flag = ReleaseSyncDataLock (service_p -> se_sync_data_p);
		}

	return success_flag;
}



