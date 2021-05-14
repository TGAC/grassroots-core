/*
 * uuid_util.c
 *
 *  Created on: 30 Apr 2021
 *      Author: billy
 */

#include <ctype.h>
#include <string.h>

#include "uuid_util.h"
#include "memory_allocations.h"
#include "string_linked_list.h"
#include "streams.h"


bool IsUUIDSet (uuid_t id)
{
	return (!uuid_is_null (id));
}


void ConvertUUIDToString (const uuid_t id, char *uuid_s)
{
	uuid_unparse_lower (id, uuid_s);
	* (uuid_s + (UUID_STRING_BUFFER_SIZE - 1)) = '\0';
}


bool ConvertStringToUUID (char *id_s, uuid_t id)
{
	return (uuid_parse (id_s, id) == 0);
}


char *GetUUIDAsString (const uuid_t id)
{
	char *uuid_s = (char *) AllocMemory (UUID_STRING_BUFFER_SIZE * sizeof (char));

	if (uuid_s)
		{
			ConvertUUIDToString (id, uuid_s);
			* (uuid_s + (UUID_STRING_BUFFER_SIZE - 1)) = '\0';
		}

	return uuid_s;
}


void FreeUUIDString (char *uuid_s)
{
	FreeMemory (uuid_s);
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

