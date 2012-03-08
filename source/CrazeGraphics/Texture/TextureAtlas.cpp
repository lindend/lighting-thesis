#include "CrazeGraphicsPCH.h"
#include "TextureAtlas.h"

using namespace Craze;
using namespace Craze::Graphics2;

using std::vector;

struct AtlasRect
{
    unsigned int top;
    unsigned int left;
    unsigned int width;
    unsigned int height;
};

struct AtlasNode
{
    AtlasNode() : m_children[0](nullptr), m_children[1](nullptr), m_used(false) {}

    AtlasRect m_rect;
    AtlasNode* m_children[2];
    bool m_used;
};

AtlasNode* Craze::Graphics2::atlasCreate(unsigned int w, unsigned int h)
{
    AtlasNode* node = new AtlasNode();
    node->m_rect.left = 0;
    node->m_rect.top = 0;
    node->m_rect.width = w;
    node->m_rect.height = h;
    return node;
}
void Craze::Graphics2::atlasFree(AtlasNode* node)
{
    if (node->m_children[0])
    {
        atlasFree(node->m_children[0]);
    }
    if (node->m_children[1])
    {
        atlasFree(node->m_children[1]);
    }
    delete node;
}
AtlasNode* Craze::Graphics2::atlasInsert(AtlasNode* node, unsigned int w, unsigned int h)
{
    if (node->m_children[0])
    {
        AtlasNode* res = nullptr;

        if (res = atlasInsert(node->m_children[0], w, h))
        {
            return res;
        }
        return atlasInsert(node->m_children[1], w, h);
    } else
    {
        if (node->m_used)
        {
            return nullptr;
        }
        if (node->m_rect.width < w || node->m_rect.height < h)
        {
            return nullptr;
        }

        if (node->m_rect.width == w && node->m_rect.height == h)
        {
            node->m_used = true;
            return node;
        }

        AtlasNode* left = node->m_children[0] = new AtlasNode();
        AtlasNode* right = node->m_children[1] = new AtlasNode();

        const unsigned int deltaWidth = node->m_rect.width - w;
        const unsigned int deltaHeight = node->m_rect.height - h;

        if (deltaWidth > deltaHeight)
        {
            left->m_rect = node->m_rect;
            left->m_rect.width = w;

            right->m_rect = node->m_rect;
            right->m_rect.left = node->m_rect.left + w;
            right->m_rect.width= node->m_rect.width - w;
        } else
        {
            left->m_rect = node->m_rect;
            left->m_rect.height = h;

            right->m_rect = node->m_rect;
            right->m_rect.top = node->m_rect.top + h;
            right->m_rect.height = node->m_rect.height - w;
        }

        return atlasInsert(left, w, h);
    }
}
