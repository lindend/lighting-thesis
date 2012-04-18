#pragma once
#include <memory>

#include "Effect/LightVolumeEffects.h"

//#define CRAZE_USE_SH_LV

#ifdef CRAZE_USE_SH_LV
#define CRAZE_NUM_LV 3
#else
#define CRAZE_NUM_LV 6
#endif

namespace Craze
{
	namespace Graphics2
	{
		class Texture;
		class Scene;
		class RenderTarget;
		class DepthStencil;
		class UAVBuffer;
		class SRVBuffer;
		class Light;
		class ComputeShaderResource;
		class LVFirstBounceEffect;
		class LVInjectRaysEffect;
		class Camera;
		class Renderer;

		class LightVolumeInjector
		{
		public:
			LightVolumeInjector(Renderer* renderer) : m_renderer(renderer), m_active(0) {}

			bool initialize();

			std::shared_ptr<RenderTarget>* getLightingVolumes(Scene* scene);

			void setTriangles(Vec3* triangles, int numTriangles);

			void destroy() {}
			std::shared_ptr<UAVBuffer> getCollidedRays();

			const LightVolumeInfo getLVInfo(const Camera* cam) const;

		private:
			void injectAll(const Camera* cams, int numCams, Scene* scene);
			void renderRSMs(Scene* scene, const Camera* c, const Matrix4& viewProj);
			void spawnRays(const Matrix4& viewProj, const Camera* cam);
			void traceRays();
			void tessellateRays();
			void injectToLV(const Camera* c);
			void mergeToTarget();

			static const int RSMResolution = 128;
			static const int LightVolumeResolution = 16;
			static const int MaxPhotonRays = RSMResolution * RSMResolution * 4;

			static const int NumRSMs = 2;
			std::shared_ptr<RenderTarget> m_RSMs[NumRSMs];
			std::shared_ptr<DepthStencil> m_RSMDS;

			std::shared_ptr<RenderTarget> m_dummy;

			//One lighting volume for each color
			std::shared_ptr<RenderTarget> m_lightVolumes[2][CRAZE_NUM_LV];
			int m_active;
			Vector3 m_prevLVPos;

			int m_numTriangles;
			std::shared_ptr<SRVBuffer> m_triangleBuffer;

			std::shared_ptr<UAVBuffer> m_toTestRays;
			std::shared_ptr<UAVBuffer> m_tessellatedRays;

			std::shared_ptr<const ComputeShaderResource> m_rayTraceCS;
			std::shared_ptr<const ComputeShaderResource> m_tessellateCS;

			std::unique_ptr<LVFirstBounceEffect> m_fxFirstBounce;
			std::unique_ptr<LVInjectRaysEffect> m_fxInjectRays;
			std::unique_ptr<IEffect> m_fxMergeLV;

			ID3D11BlendState* m_addBS;
			ID3D11BlendState* m_mergeBS;
			ID3D11RasterizerState* m_AALinesRS;

			ID3D11SamplerState* m_lowMipSampler;

			Renderer* m_renderer;
		};
	}
}