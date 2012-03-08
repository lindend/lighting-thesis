#include "CrazeGraphicsPCH.h"
#include "VertexStreams.h"

#include "Buffer/Buffer.h"

using namespace Craze;
using namespace Craze::Graphics2;

VertexStreams::VertexStreams()
{
	for (int i = 0; i < NUMSTREAMS; ++i)
	{
		m_pStreams[i] = nullptr;
	}
}

void VertexStreams::SetStreamPosNormalUv(std::shared_ptr<GeometryBuffer> pBuffer)
{
	SetStream(POSNORMALUV, pBuffer);
}
void VertexStreams::SetStreamLightMap(std::shared_ptr<GeometryBuffer> pBuffer)
{
	SetStream(LIGHTMAP, pBuffer);
}
void VertexStreams::SetStreamSkinning(std::shared_ptr<GeometryBuffer> pBuffer)
{
	SetStream(SKINNING, pBuffer);
}
void VertexStreams::SetStream(STREAM stream, std::shared_ptr<GeometryBuffer> pBuffer)
{
	m_pBuffers[stream] = pBuffer;
	m_pStreams[stream] = pBuffer->GetBuffer();
}

unsigned int VertexStreams::GetFirstIndex() const
{
	if (m_pBuffers[0])
	{
		return m_pBuffers[0]->GetFirstIndex();
	}
	return 0;
}
unsigned int VertexStreams::GetNumElems() const
{
	if (m_pBuffers[0])
	{
		return m_pBuffers[0]->GetNumElems();
	}
	return 0;
}