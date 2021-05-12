

#include "windows_uuid.h"


static void ConvertFromUUIDT (const uuid_t src, UUID *dest_p);
static void ConvertToUUIDT (const UUID src, uuid_t *dest_p);


int uuid_parse (char *in, uuid_t uu)
{
	int ret = -1;
	UUID uuid;
	RPC_STATUS status = UuidFromString (in, uuid);

	if (status == RPC_S_OK)
		{
			ConvertToUUIDT (uuid, uu);
			ret = 0;
		}

	return ret;
}


static void ConvertFromUUIDT (const uuid_t src, UUID *dest_p)
{
	memcpy (& (dest_p -> Data1), src, 4);

	memcpy (& (dest_p -> Data2), src + 4, 2);
	memcpy (& (dest_p -> Data2), src + 6, 2);
	memcpy (dest_p -> Data4, src + 8, 8);
}


static void ConvertToUUIDT (const UUID src, uuid_t *dest_p)
{
	memcpy (dest_p -> uu_data, src -> Data1, 4);

	memcpy (((dest_p -> uu_data) + 4), src -> Data2, 2);
	memcpy (((dest_p -> uu_data) + 6), src -> Data3, 2);

	memcpy (((dest_p -> uu_data) + 8), src -> Data4, 8);
}

