/*
 * async_tasks_manager.c
 *
 *  Created on: 19 May 2017
 *      Author: billy
 */

#include "async_tasks_manager.h"


bool InitialiseAsyncTasksManager (AsyncTasksManager *manager_p)
{
	bool success_flag = false;
	LinkedList *tasks_p = AllocateLinkedList (FreeAsyncTaskNode);

	if (tasks_p)
		{
			manager_p -> atm_tasks_p = tasks_p;
			success_flag = true;
		}

	return success_flag;
}


