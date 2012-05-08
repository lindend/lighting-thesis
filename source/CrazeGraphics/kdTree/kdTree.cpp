#include "CrazeGraphicsPCH.h"
#include "kdTree.h"

using namespace Craze;
using namespace Craze::Graphics2;
using namespace Craze::Graphics2::kdInternal;
namespace Craze
{
	namespace Graphics2
	{
		namespace kdInternal
		{
			const float Cost_Trav = 20.f;
			const float Cost_Test = 20.f;
			const unsigned int TerminateNumTris = 10;
			const unsigned int TerminateDepth = 10;
        }
    }
}

float cost(float PL, float PR, unsigned int NL, unsigned int NR)
{
	assert(PL >= 0.f);
	assert(PR >= 0.f);

	return /*(NR == 0 || NL == 0 ? 0.8f : 1.f) * */(Cost_Trav + Cost_Test * (PL * NL + PR * NR));
}

bool terminate(unsigned int numTris, unsigned short depth)
{
	return numTris <= TerminateNumTris || depth >= TerminateDepth;
}

void split(const BoundingBox& bb, BoundingBox& left, BoundingBox& right, float p, char axis)
{
	assert(axis >= 0 && axis <= 2);

	left = bb;
	right = bb;
	//left.m_Max.vec.m128_f32[axis] = p;
	//left.m_Min.vec.m128_f32[axis] = p;
	switch (axis)
	{
	case 0:
		left.m_Max.v.x = p;
		right.m_Min.v.x = p;
		break;
	case 1:
		left.m_Max.v.y = p;
		right.m_Min.v.y = p;
		break;
	case 2:
		left.m_Max.v.z = p;
		right.m_Min.v.z = p;
		break;
	};
}

float SA(const BoundingBox& bb)
{
	Vector3 d = bb.m_Max - bb.m_Min;
	float sa = 2.f * (d.v.x * d.v.y + d.v.x * d.v.z + d.v.y * d.v.z);
	assert(sa >= 0.f);
	return sa;
}

void SAH(float p, char axis, const BoundingBox& bb, unsigned int NL, unsigned int NR, unsigned int NP, float& outCost, SIDE& outSide)
{
	PROFILEF();

	BoundingBox VL;
	BoundingBox VR;
	split(bb, VL, VR, p, axis);

	float totArea = SA(bb);
	
	if (totArea == 0.f)
	{
		outCost = 0;
		outSide = LEFT;
		return;
	}

	float pl = SA(VL) / totArea;
	float pr = SA(VR) / totArea;

	float cl = cost(pl, pr, NL + NP, NR);
	float cr = cost(pl, pr, NL, NR + NP);

	assert(cl >= 0.f && cr >= 0.f);

	if (cl < cr)
	{
		outCost = cl;
		outSide = LEFT;
	} else
	{
		outCost = cr;
		outSide = RIGHT;
	}
}

void findBestPlane(const std::vector<kdInternal::Evt>& E, unsigned int numTris, const BoundingBox& bbox, float& outPlane, char& outAxis, SIDE &outSide)
{
	PROFILEF();

	unsigned int NL[3] = {0, 0, 0}, NP[3] = {0, 0, 0}, NR[3] = {numTris, numTris, numTris};

	float Cbest = std::numeric_limits<float>::max();

	assert(E.size() > 0);

	for (unsigned int i = 0; i < E.size(); ++i)
	{
		unsigned int pbegin = 0, pend = 0, pplanar = 0;
		char pk = E[i].axis;
		float p = E[i].plane;

		while (i < E.size() && E[i].axis == pk && FloatCmp(p, E[i].plane) && E[i].type == END)
		{
			++pend;
			++i;
		}

		while (i < E.size() && E[i].axis == pk && FloatCmp(p, E[i].plane) && E[i].type == PLANAR)
		{
			++pplanar;
			++i;
		}

		while (i < E.size() && E[i].axis == pk && FloatCmp(p, E[i].plane) && E[i].type == START)
		{
			++pbegin;
			++i;
		}

		NP[pk] = pplanar;
		NR[pk] -= pplanar + pend;

		float cost;
		SIDE s;
		SAH(p, pk, bbox, NL[pk], NR[pk], NP[pk], cost, s);

		if (cost < Cbest)
		{
			Cbest = cost;
			outPlane = p;
			outAxis = pk;
			outSide = s;
		}

		NL[pk] += pbegin + pplanar;
		NP[pk] = 0;
	}
}

void modifyCounter(SIDE s, unsigned int& nl, unsigned int& nr, unsigned int& nb)
{
	switch (s)
	{
	case BOTH:
		--nb;
		break;
	case LEFT:
		--nl;
		break;
	case RIGHT:
		--nr;
	}
}


SIDE* classify(unsigned int numTris, const std::vector<kdInternal::Evt>& E, float p, char axis, SIDE s, unsigned int& outNL, unsigned int& outNR, unsigned int& outNB)
{
	PROFILEF();

	SIDE* pClass = new SIDE[numTris];
	ZeroMemory(pClass, numTris);
	outNB = numTris;
	outNL = outNR = 0;

	for (auto i = E.cbegin(); i != E.cend(); ++i)
	{
		if (i->type == END && i->axis == axis && i->plane <= p)
		{
			modifyCounter(pClass[i->triangle], outNL, outNR, outNB);
			++outNL;
			pClass[i->triangle] = LEFT;
		} else if(i->type == START && i->axis == axis && i->plane >= p)
		{
			modifyCounter(pClass[i->triangle], outNL, outNR, outNB);
			++outNR;
			pClass[i->triangle] = RIGHT;
		} else if(i->type == PLANAR && i->axis == axis)
		{
			if (i->plane < p || (i->plane == p && s == LEFT))
			{
				modifyCounter(pClass[i->triangle], outNL, outNR, outNB);
				pClass[i->triangle] = LEFT;
				++outNL;
			} else if (i->plane > p || (i->plane == p && s == RIGHT))
			{
				modifyCounter(pClass[i->triangle], outNL, outNR, outNB);
				pClass[i->triangle] = RIGHT;
				++outNR;
			}
		}
	}

	assert(outNL + outNR + outNB == numTris);

	return pClass;
}


void merge(std::vector<kdInternal::Evt>& mergeTo, const std::vector<kdInternal::Evt>& from1, const std::vector<kdInternal::Evt>& from2)
{
	PROFILEF();
	mergeTo.reserve(from1.size() + from2.size());
	auto i = from1.cbegin();
	auto j = from2.cbegin();

	while (i != from1.cend() || j != from2.cend())
	{
		if (i != from1.cend())
		{
			if (j != from2.cend())
			{
				mergeTo.push_back(*(*i < *j ? i : j)++);
			} else
			{
				mergeTo.push_back(*i);
				++i;
			}
		} else
		{
			mergeTo.push_back(*j);
			++j;
		}
	}
}


void splitEventList(std::vector<kdInternal::Evt>* pEvents, const unsigned int* pConvIdx, const SIDE* pClass, float p, char axis, unsigned int NL, unsigned int NR, std::vector<kdInternal::Evt>& outEL, std::vector<kdInternal::Evt>& outER, 
	std::vector<kdInternal::Evt>& outBL, std::vector<kdInternal::Evt>& outBR)
{
	PROFILEF();

	for (auto i = pEvents->cbegin(); i != pEvents->cend(); ++i)
	{
		kdInternal::Evt e = *i;
		e.triangle = pConvIdx[e.triangle];

		switch (pClass[i->triangle])
		{
		case LEFT:
			outEL.push_back(e);
			assert(e.axis != axis || e.plane <= p);
			break;
		case RIGHT:
			outER.push_back(e);
			assert(e.axis != axis || e.plane >= p);
			break;
		case BOTH:
			{
				float lplane = e.axis != axis || e.plane < p ? e.plane : p;
				float rplane = e.axis != axis || e.plane > p ? e.plane : p;
				unsigned int tri = e.triangle;
				e.plane = lplane;
				e.triangle = tri + NL;
				outBL.push_back(e);
				e.triangle = tri + NR;
				e.plane = rplane;
				outBR.push_back(e);
			}
			break;
		};
	}
}