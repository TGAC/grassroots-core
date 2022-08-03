/*
 * data_linked_list.c
 *
 *  Created on: 2 Aug 2022
 *      Author: billy
 */


#include "data_linked_list.h"



DataListNode *AllocateDataListNode (const char *data_p, const size_t data_size)
{
	char *copied_data_p = (char *) AllocMemory (sizeof (char) * data_size);

	if (copied_data_p)
		{
			DataListNode *node_p = (DataListNode *) AllocMemory (sizeof (DataListNode));

			if (node_p)
				{
					InitListItem (& (node_p -> dln_node));

					memcpy (copied_data_p, data_p, sizeof (char) * data_size);

					node_p -> dln_data_p = copied_data_p;
					node_p -> dln_data_size = data_size;

					return node_p;
				}

			FreeMemory (copied_data_p);
		}

	return NULL;
}



void FreeDataListNode (ListItem * const node_p)
{
	DataListNode *data_node_p = (DataListNode *) node_p;

	if (data_node_p -> dln_data_p)
		{
			FreeMemory (data_node_p -> dln_data_p);
		}

	FreeMemory (data_node_p);
}



LinkedList *AllocateDataLinkedList (void)
{
	return AllocateLinkedList (FreeDataListNode);
}




bool AddDataToDataLinkedList (LinkedList *list_p, const char *data_p, const size_t data_size)
{
	bool success_flag = false;
	DataListNode *node_p = AllocateDataListNode (data_p, data_size);

	if (node_p)
		{
			LinkedListAddTail (list_p, & (node_p -> dln_node));
			success_flag = true;
		}

	return success_flag;
}


char *GetAllDataFromDataLinkedList (LinkedList *list_p, size_t *size_p)
{
	char *data_p = NULL;

	return data_p;
}


bool WriteAllDataFromDataLinkedList (LinkedList *list_p, const char * const filename_s, size_t *size_p)
{
	bool success_flag = false;
	FILE *out_f = fopen (filename_s, "wb");

	if (out_f)
		{
			int res;
			DataListNode *node_p = (DataListNode *) (list_p -> ll_head_p);

			while (node_p && success_flag)
				{
					if (fwrite (node_p -> dln_data_p, node_p -> dln_data_size, 1, out_f) == 1)
						{
							node_p = (DataListNode *) (node_p -> dln_node.ln_next_p);
						}
					else
						{
							success_flag = false;
						}
				}

			res = fclose (out_f);
		}


	return success_flag;
}
