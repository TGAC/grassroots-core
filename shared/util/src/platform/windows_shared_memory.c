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

/*
 * unix_shared_memory.c
 *
 *  Created on: 20 Feb 2023
 *      Author: billy
 */

#include <windows.h>

#include "memory_allocations.h"
#include "streams.h"


struct MappedMemory
{
	HANDLE mm_handle_p;

	char *mm_file_view_p;

	size_t mm_size;
};



struct MappedMemory *AllocateSharedMemory (const char *id_s, size_t size, int flags)
{
	struct MappedMemory *mem_p = (struct MappedMemory *) AllocMemory (sizeof (struct MappedMemory));

	if (mem_p)
		{
			HANDLE map_p = CreateFileMapping (
				INVALID_HANDLE_VALUE,    // use paging file
				NULL,                    // default security
				PAGE_READWRITE,          // read/write access
				0,                       // maximum object size (high-order DWORD)
				size,                // maximum object size (low-order DWORD)
				id_s);

			if (map_p)
				{
					char *file_view_p = (char *) MapViewOfFile (map_p,   // handle to map object
																			 FILE_MAP_ALL_ACCESS, // read/write permission
																			 0,
																			 0,
																				size);

					if (file_view_p)
						{
							mem_p -> mm_handle_p = map_p;
							mem_p -> mm_file_view_p = file_view_p;
							mem_p -> mm_size = size;

							return mem_p;

						}

					CloseHandle (map_p);
				}

			FreeMemory (mem_p);
		}

	return NULL;
}


bool FreeSharedMemory (struct MappedMemory *mem_p)
{
	bool success_flag = true;

	if (!CloseSharedMemory (mem_p, mem_p->mm_file_view_p))
		{
			success_flag = false;
		}

	if (CloseHandle (mem_p->mm_handle_p))
		{
			success_flag = false;
		}

	FreeMemory (mem_p);

	return success_flag;
}


void *OpenSharedMemory (struct MappedMemory *mem_p, int flags)
{
	char *file_view_p = (char *) MapViewOfFile (mem_p -> mm_handle_p,   // handle to map object
																FILE_MAP_ALL_ACCESS, // read/write permission
																0,
																0,
																mem_p -> mm_size);

	if (file_view_p)
		{
			CloseSharedMemory (mem_p, NULL);
			mem_p -> mm_file_view_p = file_view_p;
		}

	return mem_p -> mm_file_view_p;
}


bool CloseSharedMemory (struct MappedMemory *mem_p, void *value_p)
{
	if (mem_p -> mm_file_view_p)
		{ 
			UnmapViewOfFile (mem_p -> mm_file_view_p);
			mem_p->mm_file_view_p = NULL;
		}

	return true;
}

