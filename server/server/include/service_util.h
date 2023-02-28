/*
 * service_util.h
 *
 *  Created on: 13 Feb 2023
 *      Author: billy
 */

#ifndef CORE_SERVER_SERVER_INCLUDE_SERVICE_UTIL_H_
#define CORE_SERVER_SERVER_INCLUDE_SERVICE_UTIL_H_



#include "grassroots_service_manager_library.h"

#include "service.h"


#ifdef __cplusplus
extern "C"
{
#endif


/**
 * Has the Service got any jobs still running?
 *
 * @param service_p The Service to check.
 * @return <code>true</code> if the Service still has active jobs, <code>false</code> otherwise.
 */
GRASSROOTS_SERVICE_MANAGER_API  bool IsServiceLive (Service *service_p);


/**
 * @brief Check if any ServiceJobs within Service are still running
 *
 * @param service_p The Service to check.
 * @return <code>true</code> if there are still some ServiceJobs in the given service
 * that are still running, <code>false</code> otherwise.
 */
GRASSROOTS_SERVICE_MANAGER_API int32 GetNumberOfLiveJobs (Service *service_p);


/**
 * Is a given Service lockable with a mutex for use in a multi-threaded
 * situation.
 *
 * @param service_p The Service to check.
 * @return <code>true</code> if the Service is lockable, <code>false</code> otherwise.
 */
GRASSROOTS_SERVICE_MANAGER_API bool IsServiceLockable (const Service *service_p);


/**
 * Lock a given Service with a mutex for use in a multi-threaded
 * situation.
 *
 * @param service_p The Service to check.
 * @return <code>true</code> if the Service was locked successfully,
 * <code>false</code> otherwise.
 */
GRASSROOTS_SERVICE_MANAGER_API bool LockService (Service *service_p);


/**
 * Release the lock for a given Service in a multi-threaded
 * situation.
 *
 * @param service_p The Service to check.
 * @return <code>true</code> if the Service was unlocked successfully,
 * <code>false</code> otherwise.
 */
GRASSROOTS_SERVICE_MANAGER_API bool UnlockService (Service *service_p);





#ifdef __cplusplus
}
#endif


#endif /* CORE_SERVER_SERVER_INCLUDE_SERVICE_UTIL_H_ */
