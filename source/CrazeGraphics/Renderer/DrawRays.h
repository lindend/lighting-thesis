#pragma once
#include <memory>

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
		};
	}
}