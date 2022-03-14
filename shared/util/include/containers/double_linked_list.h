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
 * @file
 * @brief
 */
/** @file doubl_linked_list.h
 *  @brief A doubly-linked list of dpubles
 *
 *
 */
#ifndef DOUBLE_LINKED_LIST_H
#define DOUBLE_LINKED_LIST_H


#include "grassroots_util_library.h"
#include "linked_list.h"
#include "memory_allocations.h"
#include "typedefs.h"


#ifdef __cplusplus
	extern "C" {
#endif


/**
 * A ListNode for LinkedLists that also stores
 * an double value.
 *
 * @extends ListItem
 *
 * @ingroup utility_group
 */
typedef struct DoubleListNode
{
	/** The ListNode. */
	ListItem dln_node;

	/** The double value. */
	double64 dln_value;

} DoubleListNode;


/**
 * Create a new DoubleListNode.
 *
 * @param str_p The string to store in the newly-created DoubleListNode.
 * @param mem_flag How the DoubleListNode should store its string.
 * @return The new DoubleListNode or <code>NULL</code> upon error.
 * @memberof DoubleListNode
 */
GRASSROOTS_UTIL_API DoubleListNode *AllocateDoubleListNode (const double64 value);


/**
 * Free a DoubleListNode.
 *
 * @param node_p The ListNode to free.
 * @memberof DoubleListNode
 */
GRASSROOTS_UTIL_API void FreeDoubleListNode (ListItem * const node_p);




/**
 * Create a LinkedList designed to hold DoubleListNodes.
 *
 * @return The new LinkedList or <code>NULL</code> upon error.
 * @memberof DoubleListNode
 */
GRASSROOTS_UTIL_API LinkedList *AllocateDoubleLinkedList (void);



/**
 * Create and add a new DoubleListNode to the tail of a LinkedList.
 *
 * @param list_p The List to add the node to the end of.
 * @param value The value to store in the newly-created DoubleListNode.
 * @return <code>true</code> upon success, <code>false</code> on error.
 * @memberof DoubleListNode
 */
GRASSROOTS_UTIL_API bool AddDoubleToDoubleLinkedList (LinkedList *list_p, const double64 value);



#ifdef __cplusplus
}
#endif

#endif	/* #ifndef INT_LINKED_LIST_H */

