#pragma once
#include <memory>

#include "Mesh.h"
#include "Texture/RenderTarget.h"
#include "Texture/DepthStencil.h"
#include "Scene/Scene.h"
#include "EffectUtil/CBufferManager.h"
#include "LightVolumeInjector.h"

#include "Font/FontManager.h"

#include "Memory/MemoryManager.h"

namespace Craze
{
	namespace Graphics2
	{
		class SceneNode;
		class UserInterface;
		class Device;
		class SpotLight;
		class DirectionalLight;
		class Graphics;
		class DrawRays;
		struct Light;

		class Renderer
		{
			CRAZE_ALLOC_ALIGN(16);
		public:
			void Initialize();
			void Shutdown();

			void BindScene(Scene* pScene);
			void RenderScene(Scene* pScene);
			void RenderUI(UserInterface* pUI);
			void RenderTest();

			void InitFrame(Scene* pScene, CBPerFrame &cbuffer);

			class Face* m_pFontFace;
			StaticString m_Str;

			ID3D11SamplerState* getBilinearSampler() { return m_pSamplerState; }

			Renderer();
			~Renderer();

		private:

			void RenderLight(DirectionalLight* pLight, Scene* pScene);

			void drawShadowMaps(Scene* scene);
			void drawGBuffers(Scene* scene);

			LightVolumeInjector m_lightVolumeInjector;

			static const int NumGBuffers = 3;
			std::shared_ptr<RenderTarget> m_GBuffers[NumGBuffers];
			std::shared_ptr<RenderTarget> m_pOutputTarget;

			std::shared_ptr<RenderTarget> m_pShadowMap;
			std::shared_ptr<DepthStencil> m_pShadowDS;

			std::shared_ptr<Mesh> m_pScreenQuad;

			//std::auto_ptr<SpotLight> m_pSpotLight;
			std::auto_ptr<DirectionalLight> m_pDirLight;

			ID3D11DepthStencilState* m_pLightPassDSS;
			ID3D11DepthStencilState* m_pShadingPassDSS;
			ID3D11DepthStencilState* m_pNoDepthDSS;
			ID3D11BlendState* m_pLightBS;

			ID3D11SamplerState* m_pSamplerState;
			ID3D11SamplerState* m_pPointSampler;
			ID3D11SamplerState* m_pPointBorderSampler;
			ID3D11SamplerState* m_pBilinearBorderSampler;

			DrawRays* m_rayDrawer;
		};
	}
}