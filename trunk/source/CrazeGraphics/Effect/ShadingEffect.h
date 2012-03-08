#pragma once
#include "IEffect.h"

namespace Craze
{
	namespace Graphics2
	{
		class Device;

		class ShadingEffect : public IEffect
		{

		public:
			ShadingEffect();
			~ShadingEffect();

			bool initialize();

			void set(Texture* lightAccumulation);

			void setObjectProperties(const Matrix4& world, const Material& material);

			void reset();

			void destroy();

		private:
			ID3D11VertexShader* m_vsDefault;
			ID3D11VertexShader* m_vsTerrain;

			ID3D11PixelShader* m_psDefault;
			ID3D11PixelShader* m_psTerrain;

		};
	}
}