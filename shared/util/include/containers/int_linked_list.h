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
/** @file int_linked_list.h
 *  @brief A doubly-linked list of integers
 *
 *
 */
#ifndef INT_LINKED_LIST_H
#define INT_LINKED_LIST_H


#include "grassroots_util_library.h"
#include "linked_list.h"
#include "memory_allocations.h"
#include "typedefs.h"


#ifdef __cplusplus
	extern "C" {
#endif


/**
 * A ListNode for LinkedLists that also stores
 * an integer value.
 *
 * @extends ListItem
 *
 * @ingroup utility_group
 */
typedef struct IntListNode
{
	/** The ListNode. */
	ListItem iln_node;

	/** The integer value. */
	int32 iln_value;

} IntListNode;


/**
 * Create a new IntListNode.
 *
 * @param str_p The string to store in the newly-created IntListNode.
 * @param mem_flag How the IntListNode should store its string.
 * @return The new IntListNode or <code>NULL</code> upon error.
 * @memberof IntListNode
 */
GRASSROOTS_UTIL_API IntListNode *AllocateIntListNode (const int32 value);


/**
 * Free a IntListNode.
 *
 * @param node_p The ListNode to free.
 * @memberof IntListNode
 */
GRASSROOTS_UTIL_API void FreeIntListNode (ListItem * const node_p);




/**
 * Create a LinkedList designed to hold IntListNodes.
 *
 * @return The new LinkedList or <code>NULL</code> upon error.
 * @memberof IntListNode
 */
GRASSROOTS_UTIL_API LinkedList *AllocateIntLinkedList (void);



/**
 * Create and add a new IntListNode to the tail of a LinkedList.
 *
 * @param list_p The List to add the node to the end of.
 * @param value The value to store in the newly-created IntListNode.
 * @return <code>true</code> upon success, <code>false</code> on error.
 * @memberof IntListNode
 */
GRASSROOTS_UTIL_API bool AddIntegerToIntLinkedList (LinkedList *list_p, const int32 value);



#ifdef __cplusplus
}
#endif

#endif	/* #ifndef INT_LINKED_LIST_H */

