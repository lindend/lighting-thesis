#pragma once
#include <vector>

#include "EventLogger.h"

#include "EffectUtil/CBufferManager.h"

namespace Craze
{
	namespace Graphics2
	{
		class IEffect;
		class Material;
		class GeometryBuffer;
		class RenderTarget;
		class DepthStencil;
		class LayoutManager;
		class VertexStreams;

		extern class Device* gpDevice;

		bool InitGraphicsDevice(int width, int height, bool fullScreen, HWND hWnd);
		void ShutdownGraphicsDevice();
		
		class Device
		{
			friend bool InitGraphicsDevice(int width, int height, bool fullScreen, HWND hWnd);
			friend void ShutdownGraphicsDevice();
		public:
			Device();
			~Device();
			bool Init(int width, int height, bool fullScreen, HWND hWnd);

			void SetVertexStreams(const VertexStreams* pStreams);
			void SetIndexBuffer(std::shared_ptr<GeometryBuffer> pIndexBuffer);
			void ClearCache();

			void SetCurrentEffect(IEffect* pEffect);
			IEffect* GetCurrentEffect();

			void SetRenderTargets(std::shared_ptr<RenderTarget>* pRenderTargets, unsigned int numRenderTargets, ID3D11DepthStencilView* pDSV);
			void SetRenderTargets(const std::vector<std::shared_ptr<RenderTarget>>& pRenderTargets, std::shared_ptr<DepthStencil> pDS);
			void SetRenderTarget(std::shared_ptr<RenderTarget> pRenderTarget, std::shared_ptr<DepthStencil> pDS);
			void SetRenderTargets(const std::vector<std::shared_ptr<RenderTarget>>& pRenderTargets);
			void SetRenderTarget(std::shared_ptr<RenderTarget> pRenderTarget);


			void SetObjectProperties(const Matrix4& world, const Material& material);

			void SetShader(ID3D11PixelShader* pPixelShader);
			void SetShader(ID3D11VertexShader* pVertexShader);
			void SetShader(ID3D11GeometryShader* pGeoShader);
			void SetShader(ID3D11HullShader* hullShader);
			void SetShader(ID3D11DomainShader* domainShader);

			void DrawIndexed();
			void Draw();
			void drawBuffer(std::shared_ptr<GeometryBuffer> buffer);

			void Swap();
			void Clear(const Vector4& color);

			void SetResolution(unsigned int width, unsigned int height, bool fullscreen);
			Vector2 GetViewPort() const;

			ID3D11Device* GetDevice();
			ID3D11DeviceContext* GetDeviceContext();
			D3D_FEATURE_LEVEL GetFeatureLevel() const;
			ID3D11DepthStencilView* GetDefaultDepthBuffer() const { return m_pDepthStencilBuffer; }
			ID3D11ShaderResourceView* GetDefaultDepthSRV() const { return m_pDepthShaderResource; }
			ID3D11RenderTargetView* getBackBufferRTV() const { return m_pBackBuffer; }

			CBufferManager* GetCbuffers() { return &m_CBufferManager; }

			unsigned long GetVertexCount() const { return m_VertexCount; }
			unsigned long GetTriangleCount() const { return m_TriCount; }
			void ResetCounters() { m_TriCount = 0; m_VertexCount = 0;}

			void Shutdown();

		private:
			Device(const Device& d);

			bool GetBackBufferFromSwap();
			bool CreateDepthStencil(unsigned int width, unsigned int height);
			void SetBackBufferVP(unsigned int width, unsigned int height);

			CBufferManager m_CBufferManager;

			Vector2 m_ViewPortSize;

			std::shared_ptr<GeometryBuffer> m_pCurrentIndices;
			const VertexStreams* m_CurrentStreams;

			ID3D11Device* m_pDevice;
			ID3D11DeviceContext* m_pDeviceContext;
			IDXGISwapChain* m_pSwapChain;

			ID3D11RenderTargetView* m_pBackBuffer;
			ID3D11DepthStencilView* m_pDepthStencilBuffer;
			ID3D11ShaderResourceView* m_pDepthShaderResource;
			ID3D11Texture2D* m_pDepthStencil;

			ID3D11RenderTargetView* m_pRenderTargets[8];

			D3D_FEATURE_LEVEL m_FeatureLevel;

			ID3D11InputLayout* m_pCurrentLayout;
			ID3D11PixelShader* m_pCurrentPS;
			ID3D11VertexShader* m_pCurrentVS;
			ID3D11GeometryShader* m_pCurrentGS;

			IEffect* m_pCurrentEffect;

			unsigned long m_TriCount;
			unsigned long m_VertexCount;
		};
	}
}