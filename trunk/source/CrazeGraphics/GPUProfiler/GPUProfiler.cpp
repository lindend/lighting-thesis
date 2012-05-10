#include "CrazeGraphicsPCH.h"
#include "GPUProfiler.h"

#include "Device.h"
#include "Frame.h"

using namespace Craze::Graphics2;

const int NumFrames = 5;

GPUProfiler::GPUProfiler() : m_currentFrame(0), m_prevFrame(0)
{
	m_frames = new Frame[NumFrames];
    for (int i = 0; i < NumFrames; ++i)
    {
	    m_frames[i].init(gpDevice->GetDevice());
    }

}
GPUProfiler::~GPUProfiler()
{
	delete m_frames;
}

void GPUProfiler::beginFrame()
{
    m_prevFrame = m_currentFrame;
	m_currentFrame = (m_currentFrame + 1) % NumFrames;
	m_frames[m_currentFrame].begin(gpDevice->GetDeviceContext());
}

void GPUProfiler::endFrame()
{
	m_frames[m_currentFrame].end();
    m_frames[m_prevFrame].buildTimings();
}

const std::vector<TimingBlock>* GPUProfiler::getTimings()
{
	return m_frames[m_prevFrame].getTimings();
}

int GPUProfiler::beginBlock(const char* name)
{
	return m_frames[m_currentFrame].beginBlock(name);
}
void GPUProfiler::endBlock(int block)
{
	m_frames[m_currentFrame].endBlock(block);
}