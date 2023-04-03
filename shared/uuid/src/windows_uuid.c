

#include "windows_uuid.h"

#include "string_utils.h"

#include "uuid_defs.h"
#include "streams.h"
#include "uuid_util.h"

static void ConvertFromUUIDT (const uuid_t *src_p, UUID *dest_p);
static void ConvertToUUIDT (const UUID *src_p, uuid_t *dest_p);
static void PrintIds (const uuid_t l, const UUID w);



int uuid_compare (uuid_t uu1, uuid_t uu2)
{
	return memcmp (uu1.uu_data, uu2.uu_data, UUID_RAW_SIZE * sizeof (char));
}


void uuid_generate (uuid_t uu)
{
	UUID win_uuid;
	RPC_STATUS status = UuidCreate (&win_uuid);

	if (status == RPC_S_OK)
		{
			PrintUUID (&win_uuid, "uuid_generate ()");
			ConvertToUUIDT (&win_uuid, &uu);	
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "UuidCreate () failed");
		}


	PrintUUIDT (&uu, "Leaving uuid_generate ()");
}


void uuid_copy (uuid_t dst, uuid_t src)
{
	memcpy (dst.uu_data, src.uu_data, UUID_RAW_SIZE * sizeof (char));

	PrintUUIDT (&src, "uuid_copy () SRC");
	PrintUUIDT (&dst, "uuid_copy () DEST");

}


void uuid_clear (uuid_t uu)
{
	memset (uu.uu_data, 0, UUID_RAW_SIZE);
}

int uuid_parse (char *in, uuid_t uu)
{
	int ret = -1;
	UUID win_uuid;
	RPC_STATUS status = UuidFromStringA (in, &win_uuid);

	PrintLog (STM_LEVEL_INFO, __FILE__, __LINE__, "uuid_parse () \"%s\"", in);

	if (status == RPC_S_OK)
		{
			ConvertToUUIDT (&win_uuid, &uu);
			ret = 0;
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "UuidFromStringA () failed for \"%s\"", in);
		}

	return ret;
}


int uuid_is_null (uuid_t uu)
{
	int ret = 0;
	UUID win_uuid;
	RPC_STATUS status;

	ConvertFromUUIDT (&uu, &win_uuid);

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
	char *uuid_s = NULL;

	PrintLog (STM_LEVEL_INFO, __FILE__, __LINE__, "uuid_unparse_lower ()");
	ConvertFromUUIDT (&uu, &win_uuid);

	status = UuidToStringA (&win_uuid, (RPC_CSTR *) &uuid_s);

	if (status == RPC_S_OK)
		{
			PrintLog (STM_LEVEL_INFO, __FILE__, __LINE__, "uuid: \"%s\"\n", uuid_s);

			memcpy (out_s, uuid_s, UUID_STRING_BUFFER_SIZE);

			RpcStringFreeA ((RPC_CSTR *) (&uuid_s));
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "UuidToString () failed");
		}

}


static void ConvertToUUIDT (const UUID *src_p, uuid_t *dest_p)
{
	* ((dest_p -> uu_data) + 3) = (uint8_t) (src_p -> Data1 >> 24);
	* ((dest_p -> uu_data) + 2) = (uint8_t) (src_p -> Data1 >> 16);
	* ((dest_p -> uu_data) + 1) = (uint8_t) (src_p -> Data1 >> 8);
	* (dest_p -> uu_data) = (uint8_t) (src_p -> Data1);

	* ((dest_p -> uu_data) + 5) = (uint8_t) (src_p -> Data2 >> 8);
	* ((dest_p -> uu_data) + 4) = (uint8_t) (src_p -> Data2);

	* ((dest_p -> uu_data) + 7) = (uint8_t) (src_p -> Data3 >> 8);
	* ((dest_p -> uu_data) + 6) = (uint8_t) (src_p -> Data3);

	memcpy ((dest_p -> uu_data) + 8, src_p -> Data4, 8 * sizeof (unsigned char));


	PrintUUID (src_p, "SRC UUID");
	PrintUUIDT (dest_p, "DEST uuid_t");
}

static void ConvertFromUUIDT (const uuid_t *src_p, UUID *dest_p)
{
	dest_p -> Data1 = (src_p -> uu_data[3] << 24) + (src_p -> uu_data[2] << 16) + (src_p -> uu_data[1] << 8) + (src_p -> uu_data[0]);

	dest_p -> Data2 = (src_p -> uu_data[5] << 8) + (src_p -> uu_data[4]);
	dest_p -> Data3 = (src_p -> uu_data[7] << 8) + (src_p -> uu_data[6]);

	memcpy (dest_p -> Data4, (src_p -> uu_data) + 8, 8 * sizeof (unsigned char));

	PrintUUIDT (src_p, "SRC uuid_t");
	PrintUUID (dest_p, "DEST UUID");
}


void PrintUUID (const UUID *w_p, const char *prefix_s)
{
	const uint8 *c_p = w_p -> Data4;
	char *uuid_s = NULL;
	RPC_STATUS status = UuidToStringA (w_p, (RPC_CSTR *) &uuid_s);

	if (status == RPC_S_OK)
		{
			PrintLog (STM_LEVEL_INFO, __FILE__, __LINE__, "%s: UuidToStringA (): \"%s\"", prefix_s, uuid_s);
			RpcStringFreeA ((RPC_CSTR *) (&uuid_s));
		}

	PrintLog (STM_LEVEL_INFO, __FILE__, __LINE__, "%s: %.8X %.4X %.4X %.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X",
		prefix_s,
		w_p -> Data1,
		w_p -> Data2,
		w_p -> Data3, 
		*c_p, 
		* (c_p + 1), 
		* (c_p + 2),
		* (c_p + 3),
		* (c_p + 4),
		* (c_p + 5),
		* (c_p + 6),
		* (c_p + 7)
		);


	printf ("\n");

}


void PrintUUIDT (const uuid_t *u_p, const char *prefix_s)
{
	uint32 *a_p = (uint32 *) (u_p -> uu_data);
	uint16 *b_p = (uint16 *) (u_p -> uu_data + 4);
	uint16 *c_p = (uint16 *) (u_p -> uu_data + 6);
	const uint8 *d_p = (u_p -> uu_data) + 8;

	PrintLog (STM_LEVEL_INFO, __FILE__, __LINE__, "%s: %.8X %.4X %.4X %.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X",
		prefix_s,
		*a_p,
		*b_p,
		*c_p, 
		*d_p, 
		* (d_p + 1), 
		* (d_p + 2),
		* (d_p + 3),
		* (d_p + 4),
		* (d_p + 5),
		* (d_p + 6),
		* (d_p + 7)
	);

}


static void PrintIds (const uuid_t l, const UUID w)
{
	char a = (char) (((w.Data1) & 0xff000000) >> 24);
	char b = (char) (((w.Data1) & 0x00ff0000) >> 16);
	char c = (char) (((w.Data1) & 0x0000ff00) >> 8);
	char d = (char) ((w.Data1) & 0x000000ff);
	char e = (char) (((w.Data2) & 0xff00) >> 8);
	char f = (char) ((w.Data2) && 0x00ff);
	char g = (char) (((w.Data3) & 0xff00) >> 8);
	char h = (char) ((w.Data3) && 0x00ff);

	PrintLog (STM_LEVEL_INFO, __FILE__, __LINE__, 
		"0: %.2X=%.2X  1: %.2X=%.2X  2: %.2X=%.2X  3: %.2X=%.2X  4: %.2X=%.2X  5: %.2X=%.2X  6: %.2X=%.2X  7: %.2X=%.2X  "
		"8: %.2X=%.2X  9: %.2X=%.2X  10: %.2X=%.2X  11: %.2X=%.2X  12: %.2X=%.2X  13: %.2X=%.2X  14: %.2X=%.2X  15: %.2X=%.2X\n",
		l.uu_data[0], a,
		l.uu_data[1], b,
		l.uu_data[2], c,
		l.uu_data[3], d,

		l.uu_data[4], e,
		l.uu_data[5], f,

		l.uu_data[6], g,
		l.uu_data[7], h,

		l.uu_data[8], (w.Data4 [0]),
		l.uu_data[9], (w.Data4 [1]),
		l.uu_data[10], (w.Data4 [2]),
		l.uu_data[11], (w.Data4 [3]),
		l.uu_data[12], (w.Data4 [4]),
		l.uu_data[13], (w.Data4 [5]),
		l.uu_data[14], (w.Data4 [6]),
		l.uu_data[15], (w.Data4 [7])
	);

}

/*
static void ConvertFromUUIDT (const uuid_t src, UUID *dest_p)
{
memcpy (& (dest_p -> Data1), src.uu_data, 4);

memcpy (& (dest_p -> Data2), src.uu_data + 4, 2);
memcpy (& (dest_p -> Data3), src.uu_data + 6, 2);
memcpy (dest_p -> Data4, src.uu_data + 8, 8);

PrintIds (src, *dest_p);
}



static void ConvertToUUIDT (const UUID *src_p, uuid_t *dest_p)
{
memcpy (dest_p -> uu_data, & (src_p -> Data1), 4);

memcpy (((dest_p -> uu_data) + 4), & (src_p -> Data2), 2);
memcpy (((dest_p -> uu_data) + 6), & (src_p -> Data3), 2);

memcpy (((dest_p -> uu_data) + 8), src_p -> Data4, 8);

PrintIds (*dest_p, *src_p);
}

*/