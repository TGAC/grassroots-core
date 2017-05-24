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
 * event_consumer.c
 *
 *  Created on: 24 May 2017
 *      Author: billy
 *
 * @file
 * @brief
 */

#include "event_consumer.h"
#include "memory_allocations.h"


EventConsumer *AllocateEventConsumer (void (*consumer_fn) (EventConsumer *consumer_p, AsyncTask *task_p))
{
	EventConsumer *consumer_p = (EventConsumer *) AllocMemory (sizeof (EventConsumer));

	if (consumer_p)
		{
			consumer_p -> at_consumer_fn = consumer_fn;
		}

	return consumer_p;
}


void FreeeEventConsumer (EventConsumer *consumer_p)
{
	FreeMemory (consumer_p);
}


void RunEventConsumer (EventConsumer *consumer_p, AsyncTask *task_p)
{
	consumer_p -> at_consumer_fn (consumer_p, task_p);
}
