/*
 * int_linked_list.c
 *
 *  Created on: 19 Nov 2019
 *      Author: billy
 */

#include "int_linked_list.h"
#include "streams.h"


IntListNode *AllocateIntListNode (const int32 value)
{
	IntListNode *node_p = (IntListNode *) AllocMemory (sizeof (IntListNode));

	if (node_p)
		{
			InitListItem (& (node_p -> iln_node));
			node_p -> iln_value = value;
		}

	return node_p;
}



void FreeIntListNode (ListItem * const node_p)
{
	IntListNode *int_node_p = (IntListNode *) node_p;

	FreeMemory (int_node_p);
}




LinkedList *AllocateIntLinkedList (void)
{
	return AllocateLinkedList (FreeIntListNode);
}



bool AddIntegerToIntLinkedList (LinkedList *list_p, const int32 value)
{
	bool success_flag = false;
	IntListNode *node_p = AllocateIntListNode (value);

	if (node_p)
		{
			LinkedListAddTail (list_p, & (node_p -> iln_node));
			success_flag = true;
		}

	return success_flag;
}


