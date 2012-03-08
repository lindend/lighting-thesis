#pragma once

#include "IEffect.h"

namespace Craze
{
	namespace Graphics2
	{
		class Device;
		class Texture;
		class SpotLight;
		class DirectionalLight;
		class RenderTarget;

		class LightAccumulationEffect : public IEffect
		{
		public:
			LightAccumulationEffect();
			~LightAccumulationEffect();

			bool initialize();

			void set(Texture* normSpec, Texture* depth, const std::vector<RenderTarget*>& LPV);
			void reset();

			void setLight(const SpotLight& light);
			void setLight(const DirectionalLight& light, const Matrix4& viewProj, Vec3 lightPos, Texture* shadowMap);
			void setLight(const DirectionalLight& light);

			void setObjectProperties(const Matrix4& world, const Material& material) {}

			void destroy();

		private:
			Texture* m_noise;

			ID3D11ShaderResourceView* m_textures[6];

			ID3D11VertexShader* m_vertexShader;
			ID3D11PixelShader* m_pixelShaderSpot;
			ID3D11PixelShader* m_pixelShaderDir;

		};
	}
}