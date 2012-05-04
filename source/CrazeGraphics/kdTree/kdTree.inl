//#include "CrazeGraphicsPCH.h"
#include "Intersection/BoundingBox.h"
#include "Profiling/Profiler.h"
#include "CrazeMath.h"
#include "UtilMath.h"

#include <assert.h>
#include <limits>
#include <vector>

//float cost(float PL, float PR, unsigned int NL, unsigned int NR);
bool terminate(unsigned int numTris, unsigned short depth);
void split(const Craze::BoundingBox& bb, Craze::BoundingBox& left, Craze::BoundingBox& right, float p, char axis);
//float SA(const Craze::BoundingBox& bb);
//void SAH(float p, char axis, const Craze::BoundingBox& bb, unsigned int NL, unsigned int NR, unsigned int NP, float& outCost, Craze::Graphics2::kdInternal::SIDE& outSide);
void findBestPlane(const std::vector<Craze::Graphics2::kdInternal::Evt>& E, unsigned int numTris, const Craze::BoundingBox& bbox, float& outPlane, char& outAxis, Craze::Graphics2::kdInternal::SIDE &outSide);
void merge(std::vector<Craze::Graphics2::kdInternal::Evt>& mergeTo, const std::vector<Craze::Graphics2::kdInternal::Evt>& from1, const std::vector<Craze::Graphics2::kdInternal::Evt>& from2);
Craze::Graphics2::kdInternal::SIDE* classify(unsigned int numTris, const std::vector<Craze::Graphics2::kdInternal::Evt>& E, float p, char axis, Craze::Graphics2::kdInternal::SIDE s, unsigned int& outNL, unsigned int& outNR, unsigned int& outNB);
void splitEventList(std::vector<Craze::Graphics2::kdInternal::Evt>* pEvents, const unsigned int* pConvIdx, const Craze::Graphics2::kdInternal::SIDE* pClass, float p, char axis, unsigned int NL, unsigned int NR, std::vector<Craze::Graphics2::kdInternal::Evt>& outEL, std::vector<Craze::Graphics2::kdInternal::Evt>& outER, 
	std::vector<Craze::Graphics2::kdInternal::Evt>& outBL, std::vector<Craze::Graphics2::kdInternal::Evt>& outBR);

namespace Craze
{
	namespace Graphics2
	{
		namespace kdInternal
		{
			//This code is using the algorithm http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.61.7996&rep=rep1&type=pdf


			template <class T>
			std::vector<Evt>* buildEvents(const T* pTris, unsigned int numTris, const BoundingBox& bb)
			{
				PROFILEF();

				std::vector<Evt>* pEvents = new std::vector<Evt>();
				pEvents->reserve(numTris * 6);

				Vector3 planeNormals[3] = { Vector3::RIGHT, Vector3::UP, Vector3::FORWARD };

				for (unsigned int i = 0; i < numTris; ++i)
				{
					Vector3 v0 = pTris[i].GetV0();
					Vector3 v1 = pTris[i].GetV1();
					Vector3 v2 = pTris[i].GetV2();

					v0 = Clamp(v0, bb);
					v1 = Clamp(v1, bb);
					v2 = Clamp(v2, bb);

					float pmin[3];
					float pmax[3];

					pmin[0] = Max(Min(v0.x, Min(v1.x, v2.x)), bb.m_Min.v.x);
					pmax[0] = Min(Max(v0.x, Max(v1.x, v2.x)), bb.m_Max.v.x);
					pmin[1] = Max(Min(v0.y, Min(v1.y, v2.y)), bb.m_Min.v.y);
					pmax[1] = Min(Max(v0.y, Max(v1.y, v2.y)), bb.m_Max.v.y);
					pmin[2] = Max(Min(v0.z, Min(v1.z, v2.z)), bb.m_Min.v.z);
					pmax[2] = Min(Max(v0.z, Max(v1.z, v2.z)), bb.m_Max.v.z);

					for (char k = 0; k < 3; ++k)
					{
						if (pmax[k] - pmin[k] < FDELTA)
						{
							pEvents->push_back(Evt(i, PLANAR, k, pmin[k]));
						} else
						{
							pEvents->push_back(Evt(i, START, k, pmin[k]));
							pEvents->push_back(Evt(i, END, k, pmax[k]));
						}
					}
				}

				{
					PROFILE("buildEvents sorting");
					std::sort(pEvents->begin(), pEvents->end());
				}

				return pEvents;
			}

			template <class T>
			void copyTris(const T* pTris, const SIDE* pClass, unsigned int numTris, unsigned int NL, unsigned int NR, T* outpTL, T* outpTR, unsigned int* outpConvIdx)
			{
				PROFILEF();

				unsigned int l = 0, r = 0, b = 0;

				for (unsigned int i = 0; i < numTris; ++i)
				{
					if (pClass[i] == LEFT)
					{
						outpTL[l] = pTris[i];
						outpConvIdx[i] = l;
						++l;
					} else if(pClass[i] == RIGHT)
					{
						outpTR[r] = pTris[i];
						outpConvIdx[i] = r;
						++r;
					} else
					{
						outpTL[b + NL] = pTris[i];
						outpTR[b + NR] = pTris[i];
						outpConvIdx[i] = b;
						++b;
					}
				}
			}


			template <class T>
			void recursiveBuild(std::vector<Evt>* pEvents, const BoundingBox& bbox, const T* pTris, unsigned int numTris, unsigned short depth, std::vector<kdNode>& outNodes, std::vector<T>& outTris)
			{
				PROFILEF();
				assert(pEvents && pTris);

				if (terminate(numTris, depth))
				{
					kdNode n;
					n.SetLeaf(true);
					n.triIndex = outTris.size();
					n.SetNumTris(numTris);

					if (numTris != 0)
					{
						int ombreak = 5;
					}

					outNodes.push_back(n);
					for (unsigned int i = 0; i < numTris; ++i)
					{
						outTris.push_back(pTris[i]);
					}

					delete pEvents;
					delete [] pTris;
				} else
				{

					char axis = 0;
					float p = 0;
					SIDE s;
					findBestPlane(*pEvents, numTris, bbox, p, axis, s);
					unsigned int NL, NR, NB;
					SIDE* pClass = classify(numTris, *pEvents, p, axis, s, NL, NR, NB);

					T* pTL = new T[NL + NB];
					T* pTR = new T[NR + NB];
					unsigned int* pConvIdx = new unsigned int[numTris];

					if (!pTL || !pTR || !pConvIdx)
					{
						LOG_ERROR("kdTree - unable to allocate memory for triangle buffers");
					}

					copyTris<T>(pTris, pClass, numTris, NL, NR, pTL, pTR, pConvIdx);

					delete [] pTris;

					std::vector<Evt> EL, ER, BL, BR;
					EL.reserve(NL);
					ER.reserve(NR);
					BL.reserve(NB);
					BR.reserve(NB);
					splitEventList(pEvents, pConvIdx, pClass, p, axis, NL, NR, EL, ER, BL, BR);		

					delete [] pClass;
					delete pEvents;

					std::sort(BL.begin(), BL.end());
					std::sort(BR.begin(), BR.end());

					std::vector<Evt>* pEL = new std::vector<Evt>();
					std::vector<Evt>* pER = new std::vector<Evt>();

					if (!pEL || !pER)
					{
						LOG_ERROR("kdTree - unable to allocate memory for event buffers");
					}

					merge(*pEL, EL, BL);
					merge(*pER, ER, BR);

					BoundingBox VL, VR;
					split(bbox, VL, VR, p, axis);

					kdNode node;
					node.SetAxis(axis);
					node.SetLeaf(false);
					node.plane = p;
					outNodes.push_back(node);
					unsigned int nodeIdx = outNodes.size() - 1;
					recursiveBuild<T>(pEL, VL, pTL, NL + NB, depth + 1, outNodes, outTris);
					outNodes[nodeIdx].SetRight(outNodes.size());
					recursiveBuild<T>(pER, VR, pTR, NR + NB, depth + 1, outNodes, outTris);
				}
			}
		}
	}
}