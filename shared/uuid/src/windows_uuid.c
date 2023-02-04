

#include "windows_uuid.h"

#include "string_utils.h"



static void ConvertFromUUIDT (const uuid_t src, UUID *dest_p);
static void ConvertToUUIDT (const UUID *src_p, uuid_t *dest_p);



int uuid_compare (uuid_t uu1, uuid_t uu2)
{
	return memcmp (uu1.uu_data, uu2.uu_data, UUID_SIZE * sizeof (char));
}


void uuid_generate (uuid_t uu)
{
	UUID win_uuid;
	RPC_STATUS status = UuidCreate (&win_uuid);

	ConvertToUUIDT (&win_uuid, &uu);
}


void uuid_copy (uuid_t dst, uuid_t src)
{
	memcpy (dst.uu_data, src.uu_data, UUID_SIZE * sizeof (char));
}


void uuid_clear (uuid_t uu)
{
	memset (uu.uu_data, 0, UUID_SIZE);
}

int uuid_parse (char *in, uuid_t uu)
{
	int ret = -1;
	UUID win_uuid;
	RPC_STATUS status = UuidFromString (in, &win_uuid);

	if (status == RPC_S_OK)
		{
			ConvertToUUIDT (&win_uuid, &uu);
			ret = 0;
		}

	return ret;
}


int uuid_is_null (uuid_t uu)
{
	int ret = 0;
	UUID win_uuid;
	RPC_STATUS status;

	ConvertFromUUIDT (uu, &win_uuid);

	if (UuidIsNil (&win_uuid, &status) == TRUE)
		{
			ret = 1;
		}

	return ret;
}

void uuid_unparse_lower (uuid_t uu, char *out_s)
{
	UUID win_uuid;
	RPC_STATUS status;
	RPC_CSTR str;

	ConvertFromUUIDT (uu, &win_uuid);

	status = UuidToString (&win_uuid, &str);

	if (status == RPC_S_OK)
		{
			out_s = EasyCopyToNewString ((char*) str);

			if (!out_s)
				{
				}


			RpcStringFree (&str);
		}
}


static void ConvertFromUUIDT (const uuid_t src, UUID *dest_p)
{
	memcpy (& (dest_p -> Data1), src.uu_data, 4);

	memcpy (& (dest_p -> Data2), src.uu_data + 4, 2);
	memcpy (& (dest_p -> Data2), src.uu_data + 6, 2);
	memcpy (dest_p -> Data4, src.uu_data + 8, 8);
}


static void ConvertToUUIDT (const UUID *src_p, uuid_t *dest_p)
{
	memcpy (dest_p -> uu_data, & (src_p -> Data1), 4);

	memcpy (((dest_p -> uu_data) + 4), & (src_p -> Data2), 2);
	memcpy (((dest_p -> uu_data) + 6), & (src_p -> Data3), 2);

	memcpy (((dest_p -> uu_data) + 8), src_p -> Data4, 8);
}

