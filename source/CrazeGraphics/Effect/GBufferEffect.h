#pragma once
#include "IEffect.h"

namespace Craze
{
	namespace Graphics2
	{
		class Device;
		class Camera;

		class GBufferEffect : public IEffect
		{
		public:
			GBufferEffect();
			~GBufferEffect();

			bool initialize();

			void set(const Camera* cam) { m_camera = cam; return IEffect::set(); }
			void setObjectProperties(const Matrix4& world, const Material& material);

			void destroy();

		private:
			ID3D11VertexShader* m_vertexShaderTerrain;
			ID3D11PixelShader* m_pixelShader;
			const Camera* m_camera;
		};
	}
}