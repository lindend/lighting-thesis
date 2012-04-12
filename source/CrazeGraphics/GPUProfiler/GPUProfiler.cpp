#include "CrazeGraphicsPCH.h"
#include "GPUProfiler.h"

#include "Device.h"
#include "Frame.h"

using namespace Craze::Graphics2;

GPUProfiler::GPUProfiler() : m_currentFrame(0)
{
	m_frames = new Frame[2];
	m_frames[0].init(gpDevice->GetDevice());
	m_frames[1].init(gpDevice->GetDevice());

}
GPUProfiler::~GPUProfiler()
{
	delete m_frames;
}

void GPUProfiler::beginFrame()
{
	m_currentFrame = (m_currentFrame + 1) % 2;
	m_frames[m_currentFrame].begin(gpDevice->GetDeviceContext());
}

void GPUProfiler::endFrame()
{
	m_frames[m_currentFrame].end();
}

const std::vector<TimingBlock>* GPUProfiler::getTimings()
{
	return m_frames[(m_currentFrame + 1) % 2].getTimings();
}

int GPUProfiler::beginBlock(const char* name)
{
	return m_frames[m_currentFrame].beginBlock(name);
}
void GPUProfiler::endBlock(int block)
{
	m_frames[m_currentFrame].endBlock(block);
}