#pragma once
#include <memory>

#include <string>
#include "../CrazeMath/MathTypes.h"

namespace Craze
{
	class Resource;

    class ResourceDataLoader
    {
    public:
        virtual bool has(u64 resId) = 0;
        //The type is the hash of the file ending (without the dot)
        virtual u32 getType(u64 resId) = 0;
        virtual int getSize(u64 resId) = 0;
        virtual bool read(u64 resId, char* pDestBuf, int destSize) = 0;

        virtual void add(std::shared_ptr<Resource> pRes) = 0;
        virtual void remove(std::shared_ptr<Resource> pRes) = 0;
    };
}
