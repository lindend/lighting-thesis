#include "CrazeGraphicsPCH.h"
#include "Frame.h"

#include "Memory/MemoryManager.h"

#include "PIXHelper.h"

using namespace Craze::Graphics2;

#define DO_GPU_PROFILE 1

Block allocBlock(ID3D11Device* device)
{
	Block b;
	b.name = nullptr;

	D3D11_QUERY_DESC desc;
	desc.Query = D3D11_QUERY_TIMESTAMP;
	desc.MiscFlags = 0;
	device->CreateQuery(&desc, &b.start);
	device->CreateQuery(&desc, &b.end);
	
	return b;
}

void Frame::init(ID3D11Device* device)
{
	D3D11_QUERY_DESC desc;
	desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
	desc.MiscFlags = 0;
	device->CreateQuery(&desc, &m_disjoint);
}

Frame::~Frame()
{
	m_disjoint->Release();
	for (auto i = m_blocks.begin(); i != m_blocks.end(); ++i)
	{
		i->start->Release();
		i->end->Release();
	}
}

void Frame::begin(ID3D11DeviceContext* dc)
{
#if DO_GPU_PROFILE
	m_current = 0;
	m_dc = dc;
    m_level = 0;

	m_dc->Begin(m_disjoint);
#endif
}

void Frame::end()
{
#if DO_GPU_PROFILE
	m_dc->End(m_disjoint);
#endif
}

int Frame::beginBlock(const char* name)
{
#if DO_GPU_PROFILE
	int blockId = m_current++;
	if (blockId >= m_blocks.size())
	{
		ID3D11Device* device;
		m_dc->GetDevice(&device);
		m_blocks.push_back(allocBlock(device));
	}

	Block& block = m_blocks[blockId];
	block.name = name;
	block.level = m_level++;
	m_dc->End(block.start);
	return blockId;
#endif
    return 0;
}

void Frame::endBlock(int blockId)
{
#if DO_GPU_PROFILE
	m_dc->End(m_blocks[blockId].end);
    //while (m_dc->GetData(m_blocks[blockId].end, nullptr, 0, 0) == S_FALSE);
	--m_level;
#endif
}

void Frame::buildTimings()
{
#if DO_GPU_PROFILE
    PIXMARKER(L"GPU profiler get timings");
    m_timings.resize(0);

	if (m_current == 0)
	{
		return;
	}

	while (m_dc->GetData(m_disjoint, nullptr, 0, 0) == S_FALSE)
	{
		Sleep(0);
	}

	D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjoint;
	if (m_dc->GetData(m_disjoint, &disjoint, sizeof(disjoint), 0) != S_OK)
	{
		return;
	}

	if (disjoint.Disjoint)
	{
		return;
	}

	m_timings.resize(m_current);

	for (int i = 0; i < m_current; ++i)
	{
		TimingBlock t;
		t.name = m_blocks[i].name;
		t.level = m_blocks[i].level;
		
		UINT64 blockBegin, blockEnd;
		m_dc->GetData(m_blocks[i].start, &blockBegin, sizeof(UINT64), 0);
		m_dc->GetData(m_blocks[i].end, &blockEnd, sizeof(UINT64), 0);

		UINT64 delta = blockEnd - blockBegin;
		t.time = (float)delta / (float)disjoint.Frequency * 1000.f;

		m_timings[i] = t;
	}
#endif
}

const std::vector<TimingBlock>* Frame::getTimings()
{
    return &m_timings;
}