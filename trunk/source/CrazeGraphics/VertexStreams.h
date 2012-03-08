#pragma once
#include "d3d11.h"

#include "CrazeMath.h"

namespace Craze
{
	namespace Graphics2
	{
		class GeometryBuffer;

		struct Vertex
		{
			Vec3 position;
			Vec3 normal;
			Vector2 uv;

			Vertex()
			{
			}

			Vertex(const Vec3& pos, const Vec3& nrm, Vector2 tex)
			{
				position = pos;
				normal = nrm;
				uv = tex;
			}
		};

		struct LightMapVertex
		{
			Vector2 lightMapUv;

			LightMapVertex()
			{
			}

			LightMapVertex(Vector2 lightTex)
			{
	
				lightMapUv = lightTex;
			}
		};

		struct SkinnedVertex
		{
			Vec4 blendIndices;
			Vec4 blendWeights;
		};

		class VertexStreams
		{
		public:
			enum STREAM
			{
				POSNORMALUV,
				LIGHTMAP,
				SKINNING,
				NUMSTREAMS
			};

			VertexStreams();

			ID3D11Buffer** GetStreams() const { return (ID3D11Buffer**)m_pStreams; }

			void SetStreamPosNormalUv(std::shared_ptr<GeometryBuffer> pBuffer);
			void SetStreamLightMap(std::shared_ptr<GeometryBuffer> pBuffer);
			void SetStreamSkinning(std::shared_ptr<GeometryBuffer> pBuffer);

			unsigned int GetFirstIndex() const;
			unsigned int GetNumElems() const;

			static const D3D11_INPUT_ELEMENT_DESC* getStreamLayout(int& count)
			{
				count = 6;
				static const D3D11_INPUT_ELEMENT_DESC pDesc[] = 
				{
					{"POSITION",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
					{"NORMAL",			0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
					{"TEXCOORD",		0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
					{"TEXCOORD",		1, DXGI_FORMAT_R32G32_FLOAT,		1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
					{"BLENDINDICES",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,	2, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
					{"BLENDWEIGHT",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	2, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
				};

				return pDesc;
			}

		private:
			void SetStream(STREAM stream, std::shared_ptr<GeometryBuffer> pBuffer);

			std::shared_ptr<GeometryBuffer> m_pBuffers[NUMSTREAMS];
			ID3D11Buffer* m_pStreams[NUMSTREAMS];
		};
	}
}