#pragma once
#include <memory>

#include "Effect/LightVolumeEffects.h"

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

		class LightVolumeInjector
		{
		public:
			LightVolumeInjector() {}

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
			void injectToLV(const Camera* c);

			static const int RSMResolution = 100;
			static const int LightVolumeResolution = 16;
			static const int MaxPhotonRays = RSMResolution * RSMResolution * 4;

			static const int NumRSMs = 2;
			std::shared_ptr<RenderTarget> m_RSMs[NumRSMs];
			std::shared_ptr<DepthStencil> m_RSMDS;

			std::shared_ptr<RenderTarget> m_dummy;

			//One lighting volume for each color
			std::shared_ptr<RenderTarget> m_lightingVolumes[3];

			int m_numTriangles;
			std::shared_ptr<SRVBuffer> m_triangleBuffer;

			std::shared_ptr<UAVBuffer> m_toTestRays;
			std::shared_ptr<UAVBuffer> m_collidedRays;

			std::shared_ptr<const ComputeShaderResource> m_rayTraceCS;

			std::unique_ptr<LVFirstBounceEffect> m_fxFirstBounce;
			std::unique_ptr<LVInjectRaysEffect> m_fxInjectRays;
		};
	}
}