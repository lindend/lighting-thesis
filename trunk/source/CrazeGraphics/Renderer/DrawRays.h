#pragma once
#include <memory>

#include "Effect/DrawRaysEffect.h"

namespace Craze
{
	namespace Graphics2
	{
		class DrawRays
		{
		public:
			bool initialize();
			void render(std::shared_ptr<UAVBuffer> rays, const Matrix4& viewProj);
		
		private:
			std::shared_ptr<Buffer> m_argBuffer;
			DrawRaysEffect m_effect;
			ID3D11RasterizerState* m_rasterizerState;
			ID3D11DepthStencilState* m_dss;
		};
	}
}