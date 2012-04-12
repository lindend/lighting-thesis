#pragma once

#include <vector>

#include "D3D11.h"

#include "TimingBlock.h"

namespace Craze
{
	namespace Graphics2
	{
		struct Block
		{
			ID3D11Query* start;
			ID3D11Query* end;
			const char* name;
			int level;
		};

		class Frame
		{
		public:
			Frame() : m_current(0), m_level(0) {}
			~Frame();

			void init(ID3D11Device* device);
			
			void begin(ID3D11DeviceContext* dc);
			void end();

			int beginBlock(const char* name);
			void endBlock(int block);

			const std::vector<TimingBlock>* getTimings();

		private:
			Frame(const Frame&);
			const Frame& operator=(const Frame&);

			int m_current;
			int m_level;

			std::vector<Block> m_blocks;
			std::vector<TimingBlock> m_timings;
			ID3D11DeviceContext* m_dc;
			ID3D11Query* m_disjoint;
		};
	}
}