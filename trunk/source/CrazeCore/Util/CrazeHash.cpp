#include "CrazeHash.h"

#include <emmintrin.h>
#include <memory>

using namespace Craze;

u32 Craze::hash32(const char* data, u32 len)
{
	//A large prime
	u32 hash = 1317313771;

	for (u32 i = 0; i < len; ++i)
	{
		hash = hash * 31 + data[i];
	}

	return hash;

	/*unsigned long hash = 2508234;

	__declspec(align(16)) static u32 mulConst[4] = {0x0F98A402, 0x50A89EC7, 0x8362A7DE, 0xEF83C901};
	__declspec(align(16)) u32 result[4] = {0};
	__declspec(align(16)) u32 shortBuf[4] = {0};

	__m128i dataReg;
	__m128i res;
	__m128i hashConstant = _mm_load_si128((__m128i*)mulConst);
	__m128i sum = _mm_load_si128((__m128i*)result);

	while (len >= 16)
	{
		dataReg = _mm_loadu_si128((__m128i*)&data[len - 16]);
		res = _mm_madd_epi16(dataReg, hashConstant);
		sum = _mm_add_epi32(sum, res);
		len -= 16;
	}
	if (len < 16 && len > 4)
	{
		memcpy(shortBuf, data, len);

		dataReg = _mm_load_si128((__m128i*)shortBuf);
		res = _mm_madd_epi16(dataReg, hashConstant);
		sum = _mm_add_epi32(sum, res);

	} else
	{
		hash += (int)data[0] * 0x390482F;
	}

	_mm_store_si128((__m128i*)result, sum);

	hash += result[0];
	hash += result[1];
	hash += result[2];
	hash += result[3];

	return hash;*/
}

u64 Craze::hash64(const char* data, u32 len)
{
    //A large prime
    u64 hash = 0xFFFFFFFFFFFFFFFF - 362;

    for (u32 i = 0; i < len; ++i)
    {
        hash = hash * 31 + data[i];
    }
    return hash;
}
