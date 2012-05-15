#pragma once
#include <memory>

#include <D3D11.h>

#include "Texture/Texture.h"
#include "kdTree/kdTree.h"

/*
Controls what technique should be used for storing the light in the light volumes.
If CRAZE_USE_SH_LV is defined, spherical harmonics will be used. This uses only
three textures (one for each color) with four channels each (for the first two
bands of spherical harmonics). However, this introduces artifacts where a lot of
light shine away from a surface. These artifacts appear as dark areas and is due to
the lighting shining onto the surface is getting cancelled by the light from the surface.

If CRAZE_USE_SH_LV is not defined, light is stored in cube maps instead. Then, six
textures (one for each face) with three channels (one for each color) will be used.
Every texture represents a face, and this removes the artifacts we saw with using
spherical harmonics.

NOTE: This also needs to be changed in IndirectLighting.psh
*/
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
		class Camera;
		class Renderer;
        class IEffect;

        struct LightVolumeInfo
		{
			Vector3 start;
			Vector3 end;
			Vector3 cellSize;
			float numCells;
		};

		class LightVolumeInjector
		{
		public:
			LightVolumeInjector(Renderer* renderer) : m_renderer(renderer), m_activeLightVolume(0) {}

			bool initialize();

            /*
            Should be called between frame start and the first call to addLight. This
            function swaps the active light volume and clears it and prepares the
            system for a new frame.
            */
            void beginFrame();
            /*
            Performs the first bounce step on the reflective shadow maps passed to the function.

            Input:
                color - The color of the light source.
                lightDynamicity -A factor between 0 and 1 indicating how fast the light source changes.
                                 This is used to control how much the previous frames should contribute to
                                 the lighting. A high factor means that the light moves fast and that previous
                                 light should be taken less into account. A low factor means that the light
                                 can be interpolated over a longer time which gives smoother lighting.
                                 The value should in practice probably always be >= 0.03 unless the light source
                                 is completely static.
                lightViewProj - The view x proj matrix for the light that is being added.
                RSM - The reflective shadow maps.
                RSMdepth - The depth stencil of the reflective shadow maps.
                camera - The camera that is in use for the frame.
            State change:
                The rays calculated by the first bounce shader are added to a buffer containing all of the rays for this frame.
            */
            void addLight(const Vec3& color, float lightDynamicity, const Matrix4& lightViewProj, std::shared_ptr<RenderTarget> RSM[2], std::shared_ptr<DepthStencil> RSMdepth, Camera* camera);

            /*
            Uses the rays from all calls to addLight and traces them against the scene geometry. Then,
            the rays are tessellated and finally injected into the light volume. 
            */
			std::shared_ptr<RenderTarget>* buildLightingVolumes();

            /*
            Sets the triangles that should be used as collision geometry when ray tracing.
            Input:
                triangles - A triangle list, consisting of 3 * numTriangles Vec3. 
                numTriangles - The number of triangles used.
            */
			void setTriangles(const Vec3* triangles, int numTriangles);

			void destroy() {}
            /*
            Returns the rays as after they have been collided (and, currently, tessellated).
            */
			std::shared_ptr<UAVBuffer> getCollidedRays();

            /*
            Calculates the extent of the light volume based on the camera. The light volume
            is snapped to a grid to avoid artifacts when moving the camera.
            */
			const LightVolumeInfo getLVInfo(const Camera* cam) const;

		private:
            /*
            Traces all of the rays from the first bounce against the geometry. The result is stored in the same
            buffer, but the direction is scaled to match the first collision (or a very high value if no collision
            occured).
            */
            void traceRays();
            /*
            Copies the rays to match the number of slices they intersect in the light volume. The result is stored in
            a new buffer, and the slice that each copy is intended to copy is stored in the highest 8 bits of the color
            field.
            */
			void tessellateRays();
            /*
            Takes all the tessellated rays and rasterizes them into the light volume as lines.
            */
			void injectToLV();
            /*
            Merges the previous light volume into the active light volume to reduce flickering artifacts and smooth out
            the lighting.
            */
			void mergeToTarget();

            void copyRaysToVertexBuffer();

            bool initEffects();
            bool initGeometry();

			static const int LightVolumeResolution;
			static const int MaxPhotonRays;
            static const float MinDynamicity;

            //Two sets of light volumes
			std::shared_ptr<RenderTarget> m_lightVolumes[2][CRAZE_NUM_LV];
            //Which light volume that is currently being used (switches each frame)
			int m_activeLightVolume;
            //The previous position of the light volume. Used to calculate from where to sample the light
            //when merging the light volumes.
			Vector3 m_prevLVPos;

            //The number of lights for the current frame.
            int m_numLights;

            //Geometry that the ray tracer tests against.
			std::shared_ptr<SRVBuffer> m_triangleBuffer;
            std::shared_ptr<SRVBuffer> m_kdTreeBuffer;
            kdTree<> m_kdTree;
            BoundingBox* m_geometryBB;

            /*
            The buffer that contains all the rays before they are traced against the scene geometry.        
            */
			std::shared_ptr<UAVBuffer> m_rayBuffer;
            //In this buffer, all rays that have been ray traced are stored
            std::shared_ptr<UAVBuffer> m_tracedRays;
            //The buffer where all of the tessellated rays are stored.
			std::shared_ptr<UAVBuffer> m_tessellatedRays;

            std::shared_ptr<GeometryBuffer> m_rayVertices;
            std::shared_ptr<SRVBuffer> m_rayCountBuffer;

            //An argument buffer for the call to DrawInstancedIndirect when rendering the rays into the light volume
            std::shared_ptr<SRVBuffer> m_argBuffer;

			std::shared_ptr<const ComputeShaderResource> m_rayTraceCS;
			std::shared_ptr<const ComputeShaderResource> m_tessellateCS;
            //std::shared_ptr<const ComputeShaderResource> m_firstBounceCS;

            std::unique_ptr<IEffect> m_firstBounceFx;
            //Vertex, geometry and pixel shader for rendering the rays into the light volume
			std::unique_ptr<IEffect> m_fxInjectRays;
		    //Shaders for merging the light volumes
            std::unique_ptr<IEffect> m_fxMergeLV;

            TexturePtr m_random;
			ID3D11Buffer* m_frustumInfoCBuffer;
            ID3D11Buffer* m_kdSceneInfoCBuffer;

            //Additive blend state, used when rendering the rays into the light volume
			ID3D11BlendState* m_addBS;
            //Merge blend state that uses regular alpha blending
			ID3D11BlendState* m_mergeBS;
            //Antialized line rasterizer state
			ID3D11RasterizerState* m_AALinesRS;

			Renderer* m_renderer;
		};
	}
}