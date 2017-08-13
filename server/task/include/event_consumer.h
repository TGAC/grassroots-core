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

/**
 * event_consumer.h
 *
 *  Created on: 24 May 2017
 *      Author: billy
 *
 * @file
 * @brief
 */

#ifndef CORE_SERVER_TASK_INCLUDE_EVENT_CONSUMER_H_
#define CORE_SERVER_TASK_INCLUDE_EVENT_CONSUMER_H_


#include "grassroots_task_library.h"
#include "typedefs.h"



/* forward declaration */
struct EventConsumer;
struct AsyncTask;

typedef struct EventConsumer
{
	void (*at_consumer_fn) (struct EventConsumer *consumer_p, struct AsyncTask *task_p);
} EventConsumer;


#ifdef __cplusplus
extern "C"
{
#endif


/**
 * Allocate an EventConsumer to receive notification when an AsyncTask completes.
 *
 * @param consumer_fn The callback function to be called when the AsyncTask completes.
 * @return The newly-allocated EventCustomer or <code>NULL</code> upon error.
 * @memberof EventConsumer
 */
GRASSROOTS_TASK_API EventConsumer *AllocateEventConsumer (void (*consumer_fn) (EventConsumer *consumer_p, struct AsyncTask *task_p));


/**
 * Initialise an EventConsumer with a given callback function.
 *
 * @param consumer_p The EventConsumer to initialise.
 * @param consumer_fn The callback function to be called when the AsyncTask completes.
 * @return <code>true</code> if the EventConsumer was initialiseD successfully, <code>false</code> otherwise.
 * @memberof EventConsumer
 */
GRASSROOTS_TASK_API bool InitEventConsumer (EventConsumer *consumer_p, void (*consumer_fn) (EventConsumer *consumer_p, struct AsyncTask *task_p));


/**
 * Clear an EventConsumer.
 *
 * @param consumer_p The EventConsumer to clear.
 * @memberof EventConsumer
 */
GRASSROOTS_TASK_API void ClearEventConsumer (EventConsumer *consumer_p);


/**
 * Free an EventConsumer.
 *
 * @param consumer_p The EventConsumer to free.
 * @memberof EventConsumer
 */
GRASSROOTS_TASK_API void FreeEventConsumer (EventConsumer *consumer_p);


/**
 * Fire an EventConsumer's callback function to notify that the given AsyncTask has completed.
 *
 * @param consumer_p The EventConsumer whose callback function will be ran.
 * @param task_p The AsyncTask that has completed.
 * @memberof EventConsumer
 */
GRASSROOTS_TASK_API void RunEventConsumer (EventConsumer *consumer_p, struct AsyncTask *task_p);


#ifdef __cplusplus
}
#endif


#endif /* CORE_SERVER_TASK_INCLUDE_EVENT_CONSUMER_H_ */
