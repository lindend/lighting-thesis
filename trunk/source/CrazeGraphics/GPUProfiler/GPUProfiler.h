#pragma once

#include "TimingBlock.h"

namespace Craze
{
	namespace Graphics2
	{
		class Frame;

		class GPUProfiler
		{
		public:
			GPUProfiler();
			~GPUProfiler();

			void beginFrame();
			void endFrame();

			int beginBlock(const char* name);
			void endBlock(int b);

			const std::vector<TimingBlock>* getTimings();
		private:
            int m_prevFrame;
			int m_currentFrame;
			Frame* m_frames;
            const std::vector<TimingBlock>** m_timings;
		};
	}
}