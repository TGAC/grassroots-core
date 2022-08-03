/*
 * data_linked_list.h
 *
 *  Created on: 2 Aug 2022
 *      Author: billy
 */

#ifndef CORE_SHARED_UTIL_INCLUDE_CONTAINERS_DATA_LINKED_LIST_H_
#define CORE_SHARED_UTIL_INCLUDE_CONTAINERS_DATA_LINKED_LIST_H_



#include "grassroots_util_library.h"
#include "linked_list.h"
#include "memory_allocations.h"
#include "typedefs.h"


#ifdef __cplusplus
	extern "C" {
#endif


/**
 * A ListNode for LinkedLists that also stores
 * arbitrary blocks of data.
 *
 * @extends ListItem
 *
 * @ingroup utility_group
 */
typedef struct DataListNode
{
	/** The ListNode. */
	ListItem dln_node;

	/** The data to store. */
	char *dln_data_p;

	/** The size in bytes of the stored data. */
	size_t dln_data_size;

} DataListNode;


/**
 * Create a new DataListNode.
 *
 * @param data_p The data to store in the newly-created DataListNode.
 * @param data_size The size in bytes of the data.
 * @return The new DataListNode or <code>NULL</code> upon error.
 * @memberof DataListNode
 */
GRASSROOTS_UTIL_API DataListNode *AllocateDataListNode (const char *data_p, const size_t data_size);


/**
 * Free a DataListNode.
 *
 * @param node_p The DataListNode to free.
 * @memberof DataListNode
 */
GRASSROOTS_UTIL_API void FreeDataListNode (ListItem * const node_p);




/**
 * Create a LinkedList designed to hold DataListNodes.
 *
 * @return The new LinkedList or <code>NULL</code> upon error.
 * @memberof DataListNodes
 */
GRASSROOTS_UTIL_API LinkedList *AllocateDataLinkedList (void);



/**
 * Create and add a new IntListNode to the tail of a LinkedList.
 *
 * @param list_p The List to add the node to the end of.
 * @param data_p The data to store in the newly-created DataListNode.
 * @param data_size The size in bytes of the data.

 * @return <code>true</code> upon success, <code>false</code> on error.
 * @memberof DataListNodes
 */
GRASSROOTS_UTIL_API bool AddDataToDataLinkedList (LinkedList *list_p, const char *data_p, const size_t data_size);


GRASSROOTS_UTIL_API char *GetAllDataFromDataLinkedList (LinkedList *list_p, size_t *size_p);


GRASSROOTS_UTIL_API bool WriteAllDataFromDataLinkedList (LinkedList *list_p, const char * const filename_s, size_t *size_p);


#ifdef __cplusplus
}
#endif


#endif /* CORE_SHARED_UTIL_INCLUDE_CONTAINERS_DATA_LINKED_LIST_H_ */
