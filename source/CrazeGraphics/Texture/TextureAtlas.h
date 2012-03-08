#pragma once
#include <vector>

namespace Craze
{
    namespace Graphics2
    {
        struct AtlasNode;

        AtlasNode* atlasCreate(unsigned int w, unsigned int h);
        AtlasNode* atlasInsert(AtlasNode* node, unsigned int w, unsigned int h);
        void atlasFree(AtlasNode* node);
    }
}
