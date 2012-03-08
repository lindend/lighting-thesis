#pragma once

#include "../CrazeMath/Intersection/Triangle.h"
#include "../CrazeMath/CrazeMath.h"

#include "../Geometry/MeshData.h"
#include "../VertexStreams.h"

struct Evt;

namespace Craze
{
	class BoundingBox;

	namespace Graphics2
	{
		class DefaultTriangle : public Triangle
		{
		public:
			Vector3 GetV0() const { return v0; }
			Vector3 GetV1() const { return v1; }
			Vector3 GetV2() const { return v2; }

			static DefaultTriangle CreateFromVerts(const Vertex& vtx0, const Vertex& vtx1, const Vertex& vtx2)
			{
				DefaultTriangle t;
				t.v0 = vtx0.position;
				t.v1 = vtx1.position;
				t.v2 = vtx2.position;

				return t;
			}
		};

		class UvTriangle : public DefaultTriangle
		{
			static UvTriangle CreateFromVerts(const Vertex& vtx0, const Vertex& vtx1, const Vertex& vtx2)
			{
				UvTriangle t;
				t.v0 = vtx0.position;
				t.v1 = vtx1.position;
				t.v2 = vtx2.position;
				t.uv0 = vtx0.uv;
				t.uv1 = vtx1.uv;
				t.uv2 = vtx2.uv;
				return t;
			}
			Vector2 uv0;
			Vector2 uv1;
			Vector2 uv2;
		};

		struct kdNode
		{
			//Splitting plane location.
			//If the node is a leaf, it contains the triangle index
			union
			{
				float plane;
				unsigned int triIndex;
			};

			void SetRight(unsigned int right) { flags = (flags & 0x7) | (right << 3); }
			void SetNumTris(unsigned int numTris) { SetRight(numTris); }
			void SetLeaf(bool leaf) { flags = (flags & 0xFFFFFFFE) | (leaf ? 1 : 0); }
			void SetAxis(char axis) { flags = (flags & 0xFFFFFFF9) | ((axis & 0x3) << 1); }

			unsigned int flags;	
		};

		template <class T = DefaultTriangle>
		class kdTree
		{
		public:
			void Create(std::shared_ptr<MeshData> pMesh)
			{
				PROFILEF();

				const unsigned short* pIndices = pMesh->GetIndices();
				unsigned int numIndices = pMesh->GetNumIndices();
				unsigned int numTris = numIndices / 3;
				Vertex* pVerts = (Vertex*)pMesh->GetVertices();

				BoundingBox bb;

				T* pTris = new T[numTris];
				for (unsigned int i = 0; i < numTris; ++i)
				{
					pTris[i] = T::CreateFromVerts(	pVerts[pIndices[i * 3 + 0]],
						pVerts[pIndices[i * 3 + 1]],
						pVerts[pIndices[i * 3 + 2]]);

					bb.IncludePoint(pTris[i].GetV0());
					bb.IncludePoint(pTris[i].GetV0());
					bb.IncludePoint(pTris[i].GetV0());
				}

				Create(pTris, numTris, bb);
			}

			void Create(const T* pTris, unsigned int numTris, const BoundingBox& bb)
			{
				PROFILEF();

				std::vector<kdNode> nodes;
				nodes.reserve(16000);

				std::vector<T> tris;
				//The paper "On building fast kd-Trees for Ray Tracing..." by Wald and Havran suggests that usually at most sqrt(numTris) will intersect with splitting planes
				tris.reserve(numTris + (int)Sqrt((float)numTris));

				std::vector<kdInternal::Evt>* pEvents = kdInternal::buildEvents<T>(pTris, numTris, bb);

				kdInternal::recursiveBuild<T>(pEvents, bb, pTris, numTris, 0, nodes, tris);

				m_NumTris = tris.size();
				m_pTriangles = new T[m_NumTris];
				memcpy(m_pTriangles, &tris[0], sizeof(T) * m_NumTris);

				m_NumNodes = nodes.size();
				m_pNodes = new kdNode[m_NumNodes];
				memcpy(m_pNodes, &nodes[0], sizeof(kdNode) * m_NumNodes);

				m_pBBox = CrNew BoundingBox();
				*m_pBBox = bb;
			}

			const T* GetTriangles(unsigned int& numTriangles) const { numTriangles = m_NumTris; return m_pTriangles; }
			const kdNode* GetKdNodes(unsigned int& numNodes) const { numNodes = m_NumNodes; return m_pNodes; }
			const BoundingBox* GetBBox() const { return m_pBBox; }
		private:
			BoundingBox* m_pBBox;
			T* m_pTriangles;
			kdNode* m_pNodes;
			unsigned int m_NumTris;
			unsigned int m_NumNodes;
		};

		namespace kdInternal
		{
			enum EVTTYPE : char
			{
				END,
				PLANAR,
				START,
			};

			enum SIDE : char
			{
				BOTH,
				LEFT,
				RIGHT,
			};

			struct Evt
			{

				float plane;
				unsigned int triangle;
				EVTTYPE type;
				char axis;

				Evt() {}
				Evt(unsigned int t, EVTTYPE _type, char k, float p) : triangle(t), type(_type), axis(k), plane(p) {}

				bool operator<(const Evt& o) const
				{
					return plane < o.plane || (plane == o.plane && type < o.type);
				}
			};
		}
	}
}

#include "kdTree.inl"