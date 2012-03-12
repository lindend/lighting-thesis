#pragma once
#include <memory>

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

		class LightVolumeInjector
		{
		public:
			LightVolumeInjector();

			bool initialize();

			std::shared_ptr<RenderTarget>* getLightingVolumes(Scene* scene);

			void setTriangles(Vector3* triangles, int numTriangles);

			std::shared_ptr<UAVBuffer> getCollidedRays();

		private:
			void renderRSMs(Scene* scene, const Light& l);
			void injectRays();
			void traceRays();

			static const int RSMResolution = 128;
			static const int LightVolumeResolution = 32;
			static const int MaxPhotonRays = RSMResolution * RSMResolution;

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

			const ComputeShaderResource* m_rayTraceCS;
		};
	}
}