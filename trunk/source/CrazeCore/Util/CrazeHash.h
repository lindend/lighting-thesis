#pragma once

#include <cstring>

#include "../CrazeMath/MathTypes.h"

namespace Craze
{
	u32 hash32(const char* data, u32 len);
	u64 hash64(const char* data, u32 len);

	inline u32 hash32(const char* str) { return hash32(str, strlen(str)); }
	inline u64 hash64(const char* str) { return hash64(str, strlen(str)); }
}
