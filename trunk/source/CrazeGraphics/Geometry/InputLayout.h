#pragma once
#include "GfxExport.h"

#include "d3d11.h"

namespace Craze
{
	namespace Graphics2
	{
		class InputLayout
		{
		public:
			ID3D11InputLayout* GetLayout();
		private:
			ID3D11InputLayout* m_pLayout;
		};
	}
}