#include "CrazeGraphicsPCH.h"
#include "kdTreeRayTrace.h"
#include "Intersection/Intersection.h"

#include "kdTree.h"

using namespace Craze;
using namespace Craze::Graphics2;

#define INFINITY 1000000000000.f
#define STACKSIZE 3

struct StackItem
{
	kdNode n;
	float tMin;
	float tMax;
	unsigned int idx;
};
struct ShortStack
{
	StackItem elems[STACKSIZE];
	unsigned short top;
	unsigned short numElems;
};

ShortStack create()
{
	ShortStack s;
	s.top = 0;
	s.numElems = 0;
	return s;
}
void push(ShortStack& st, kdNode n, float tMin, float tMax, unsigned int idx)
{
	st.top = (st.top + 1) % STACKSIZE;
	st.numElems = Min<unsigned short>(STACKSIZE, st.numElems + 1);
	st.elems[st.top].n = n;
	st.elems[st.top].tMin = tMin;
	st.elems[st.top].tMax = tMax;
	st.elems[st.top].idx = idx;
}
bool isEmpty(ShortStack& st)
{
	return st.numElems == 0;
}
void pop(ShortStack& st, kdNode& n, float& tMin, float& tMax, unsigned int& idx)
{
	n = st.elems[st.top].n;
	tMin = st.elems[st.top].tMin;
	tMax = st.elems[st.top].tMax;
	idx = st.elems[st.top].idx;
	st.numElems--;
	st.top = (st.top - 1) % STACKSIZE;
}

bool isLeaf(kdNode n)
{
	return n.flags & 1;
}
unsigned int getAxis(kdNode n)
{
	return (n.flags & 6) >> 1;
}

unsigned int getRight(kdNode n)
{
	return (n.flags & 0xFFFFFFF8) >> 3;
}

float Craze::Graphics2::kdTreeRayTrace(const kdTree<>* pKdTree, const Ray& r)
{
	unsigned int numNodes;
	const kdNode* pNodes = pKdTree->GetKdNodes(numNodes);
	unsigned int numTris;
	const Triangle* pTris = (const Triangle*)pKdTree->GetTriangles(numTris);

	RayBoxResult rbt = Intersection::Test(r, *pKdTree->GetBBox());
	
	if (!rbt.intersects)
	{
		return -1.f;
	}
	
	float tMin = rbt.tMin, tMax = rbt.tMin;
	float tHit = INFINITY;

	ShortStack stack = create();

	kdNode node;
	unsigned int nodeIdx;
	kdNode root = pNodes[0];
	unsigned int rootIdx = 0;
	bool pushDown;
	while (tMax < rbt.tMax)
	{
		if (isEmpty(stack))
		{
			node = root;
			nodeIdx = rootIdx;
			tMin = tMax;
			tMax = rbt.tMax;
			pushDown = true;
		} else
		{
			pop(stack, node, tMin, tMax, nodeIdx);
			pushDown = false;
		}

		while (!isLeaf(node))
		{
			unsigned int axis = getAxis(node);
			float tSplit = (node.plane - r.m_Origin.vec.m128_f32[axis]) / r.m_Dir.vec.m128_f32[axis];

			unsigned int right = getRight(node);

			//KDTreeNode* nearNode = ray.point[axis]<splitPos?node->m_leftnode:node->m_rightnode;
//	KDTreeNode* farNode = ray.point[axis]<splitPos?node->m_rightnode:node->m_leftnode;

			unsigned int first = r.m_Origin.vec.m128_f32[axis] < node.plane ? nodeIdx + 1 : right;
			unsigned int second = r.m_Origin.vec.m128_f32[axis] < node.plane ? right : nodeIdx + 1;

			//r.m_Dir.vec.m128_f32[axis] >= 0.f 

			if (tSplit >= tMax || tSplit < 0.f)
			{
				node = pNodes[first];
				nodeIdx = first;
			} else if(tSplit <= tMin)
			{
				node = pNodes[second];
				nodeIdx = second;
			} else
			{
				push(stack, pNodes[second], tSplit, tMax, second);
				node = pNodes[first];
				nodeIdx = first;
				tMax = tSplit;
				pushDown = false;
			}

			if (pushDown)
			{
				root = node;
				rootIdx = nodeIdx;
			}
		}

		unsigned int numTris = getRight(node);
		unsigned int triIdx = node.triIndex;

		for (unsigned int i = 0; i < numTris; ++i)
		{
			Vector3 n;
			float intersect = Intersection::Test(r, pTris[triIdx + i]);
			tHit = Min(intersect, tHit);

			if (tHit < tMax)
			{
				tMax = rbt.tMax;
			}
		}
	}


	return tHit;
}