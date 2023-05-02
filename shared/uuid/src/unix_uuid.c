/*
 * unix_uuid.c
 *
 *  Created on: 2 May 2023
 *      Author: billy
 */

#include "uuid_util.h"
#include "streams.h"

void PrintUUIDT (const uuid_t *u_p, const char *prefix_s)
{
	uint32 *c_p = (uint32 *) u_p;

	PrintLog (STM_LEVEL_INFO, __FILE__, __LINE__, "%s: %.4X %.4X %.4X %.4X",
		prefix_s,
		*c_p,
		* (c_p + 4),
		* (c_p + 8),
		* (c_p + 12)
	);

}
