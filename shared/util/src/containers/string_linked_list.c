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
#include <ctype.h>
#include <string.h>

#include "string_linked_list.h"
#include "string_utils.h"
#include "streams.h"


LinkedList *AllocateStringLinkedList (void)
{
	return AllocateLinkedList (FreeStringListNode);
}


StringListNode *AllocateStringListNode (const char * const value_s, const MEM_FLAG mem_flag)
{
	StringListNode *node_p = (StringListNode *) AllocMemory (sizeof (StringListNode));

	if (node_p)
		{
			if (InitStringListNode (node_p, value_s, mem_flag))
				{
					return node_p;
				}

			FreeMemory (node_p);
		}

	return NULL;
}


bool InitStringListNode (StringListNode *node_p, const char * const value_s, const MEM_FLAG mem_flag)
{
	bool success_flag = false;
	char *dest_s = NULL;

	switch (mem_flag)
		{
			case MF_DEEP_COPY:
				dest_s = CopyToNewString (value_s, 0, false);

				if (dest_s)
					{
						node_p -> sln_string_s = dest_s;
						node_p -> sln_string_flag = mem_flag;
						success_flag = true;
					}
				else
					{
						FreeMemory (node_p);
						node_p = NULL;
					}
				break;

			case MF_SHALLOW_COPY:
			case MF_SHADOW_USE:
				node_p -> sln_string_s = (char *) value_s;
				node_p -> sln_string_flag = mem_flag;
				success_flag = true;
				break;


			case MF_ALREADY_FREED:
			default:
			 	break;
		}

	return success_flag;
}



void FreeStringListNode (ListItem * const node_p)
{
	StringListNode * const str_node_p = (StringListNode * const) node_p;

	ClearStringListNode (str_node_p);

	FreeMemory (str_node_p);
}


void ClearStringListNode (StringListNode *node_p)
{
	switch (node_p -> sln_string_flag)
		{
			case MF_DEEP_COPY:
			case MF_SHALLOW_COPY:
				FreeMemory (node_p -> sln_string_s);
				break;

			case MF_SHADOW_USE:
			case MF_ALREADY_FREED:
			default:
				break;
		}
}


char *DetachStringFromStringListNode (StringListNode *node_p)
{
	char *res_s = node_p -> sln_string_s;

	node_p -> sln_string_s = NULL;
	node_p -> sln_string_flag = MF_ALREADY_FREED;

	return res_s;
}



LinkedList *CopyStringLinkedList (const LinkedList * const src_p)
{
	LinkedList *dest_p = AllocateStringLinkedList ();

	if (dest_p)
		{
			StringListNode *src_node_p = (StringListNode *) (src_p -> ll_head_p);

			while (src_node_p)
				{
					StringListNode *dest_node_p = AllocateStringListNode (src_node_p -> sln_string_s, src_node_p -> sln_string_flag == MF_SHADOW_USE ? MF_SHADOW_USE : MF_DEEP_COPY);
					
					if (dest_node_p)
						{
							LinkedListAddTail (dest_p, (ListItem * const) dest_node_p);
							src_node_p = (StringListNode *) (src_node_p -> sln_node.ln_next_p);
						}
					else
						{
							FreeLinkedList (dest_p);
							dest_p = NULL;

							/* force exit from loop */
							src_node_p = NULL;
						} 

				}		/* while (src_node_p) */

		}		/* if (dest_p) */

	return dest_p;
}


char *GetStringLinkedListAsString (const LinkedList * const src_p)
{
	char *value_s = NULL;
	
	if ((src_p != NULL) && (src_p -> ll_size > 0))
		{
			size_t *sizes_p = (size_t *) AllocMemory ((src_p -> ll_size) * sizeof (size_t));
			
			if (sizes_p)
				{			
					size_t *size_p = sizes_p;
					/* initialise it to 1 for the terminating \0 */
					size_t total_size = 1;
					const StringListNode *node_p = (const StringListNode *) (src_p -> ll_head_p);
				
					/*
					* Get the total length needed
					*/
					while (node_p)
						{
							*size_p = strlen (node_p -> sln_string_s);
							total_size += *size_p;
							
							node_p = (const StringListNode *) (node_p -> sln_node.ln_next_p);
							++ size_p;
						}
					
					
					if (total_size > 1)
						{
							value_s = (char *) AllocMemory (total_size);
							
							if (value_s)
								{
									char *current_position_p = value_s;
									node_p = (const StringListNode *) (src_p -> ll_head_p);
									size_p = sizes_p;
									
									/* Build the string */
									while (node_p)
										{
											strncpy (current_position_p, node_p -> sln_string_s, *size_p);
											current_position_p += *size_p;

											node_p = (const StringListNode *) (node_p -> sln_node.ln_next_p);
											++ size_p;
										}
			
									/* Terminate the string */
									*current_position_p = '\0';
								}
						}
											
					FreeMemory (sizes_p);
				}
		}		/* if ((src_p != NULL) && (src_p -> ll_size > 0)) */
	 
	return value_s;
}



bool AddStringToStringLinkedList (LinkedList *list_p, const char * const str_p, const MEM_FLAG mem_flag)
{
	bool success_flag = false;
	StringListNode *node_p = AllocateStringListNode (str_p, mem_flag);
	
	if (node_p)
		{
			LinkedListAddTail (list_p, (ListItem * const) node_p);
			success_flag = true;
		}
		
	return success_flag;
}



LinkedList *GetUUIDSList (const char *ids_s)
{
	LinkedList *ids_p = AllocateLinkedList (FreeStringListNode);

	if (ids_p)
		{
			bool loop_flag = true;
			const char *start_p = ids_s;
			const char *end_p = NULL;

			while (loop_flag)
				{
					/* scroll to the start of the token */
					while (isspace (*start_p))
						{
							++ start_p;
						}

					if (*start_p != '\0')
						{
							end_p = start_p;

							/* scroll to the end of the token */
							while ((isalnum (*end_p)) || (*end_p == '-'))
								{
									++ end_p;
								}

							/* Is the region the correct size for a uuid? */
							if (end_p - start_p == UUID_STRING_BUFFER_SIZE - 1)
								{
									StringListNode *node_p = NULL;
									char buffer_s [UUID_STRING_BUFFER_SIZE];

									memcpy (buffer_s, start_p, UUID_STRING_BUFFER_SIZE - 1);
									* (buffer_s + (UUID_STRING_BUFFER_SIZE - 1)) = '\0';

									node_p = AllocateStringListNode (buffer_s, MF_DEEP_COPY);

									if (node_p)
										{
											LinkedListAddTail (ids_p, (ListItem *) node_p);
										}
									else
										{
											PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to add uuid \"%s\" to list", buffer_s);
										}

								}		/* if (end_p - start_p == UUID_STRING_BUFFER_SIZE - 1) */
							else
								{
									char *c_p =  (char *) end_p;
									char c = *c_p;

									*c_p = '\0';
									PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "token \"%s\" is " SIZET_FMT " bytes long, not %d", start_p, end_p - start_p, UUID_STRING_BUFFER_SIZE - 1);
									*c_p = c;
								}

							if (*end_p != '\0')
								{
									start_p = end_p + 1;
								}
							else
								{
									loop_flag = false;
								}

						}		/* if (*start_p != '\0') */
					else
						{
							loop_flag = false;
						}

				}		/* while (loop_flag) */

			if (ids_p -> ll_size == 0)
				{
					FreeLinkedList (ids_p);
					ids_p = NULL;
				}		/* if (ids_p -> ll_size == 0) */

		}		/* if (ids_p) */

	return ids_p;
}

