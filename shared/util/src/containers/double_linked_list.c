/*
 * double_linked_list.c
 *
 *  Created on: 19 Nov 2019
 *      Author: billy
 */

#include "double_linked_list.h"
#include "streams.h"


DoubleListNode *AllocateDoubleListNode (const double64 value)
{
	DoubleListNode *node_p = (DoubleListNode *) AllocMemory (sizeof (DoubleListNode));

	if (node_p)
		{
			InitListItem (& (node_p -> dln_node));
			node_p -> dln_value = value;
		}

	return node_p;
}



void FreeDoubleListNode (ListItem * const node_p)
{
	DoubleListNode *double_node_p = (DoubleListNode *) node_p;

	FreeMemory (double_node_p);
}




LinkedList *AllocateDoubleLinkedList (void)
{
	return AllocateLinkedList (FreeDoubleListNode);
}



bool AddDoubleegerToDoubleLinkedList (LinkedList *list_p, const double64 value)
{
	bool success_flag = false;
	DoubleListNode *node_p = AllocateDoubleListNode (value);

	if (node_p)
		{
			LinkedListAddTail (list_p, & (node_p -> dln_node));
			success_flag = true;
		}

	return success_flag;
}


