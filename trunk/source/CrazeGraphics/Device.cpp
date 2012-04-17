#include "CrazeGraphicsPCH.h"
#include "Device.h"

#include "D3DCommon.h"

#include "Memory/MemoryManager.h"

#include "Graphics.h"
#include "Buffer/Buffer.h"
#include "Geometry/InputLayout.h"
#include "Texture/RenderTarget.h"
#include "Texture/DepthStencil.h"
#include "LayoutManager.h"
#include "Effect/IEffect.h"
#include "VertexStreams.h"

using namespace Craze;
using namespace Craze::Graphics2;

namespace Craze
{
	namespace Graphics2
	{
		Device* gpDevice;
	}
}

void SetDebugName(ID3D11DeviceChild* pItem, const char* pName)
{
#ifdef _DEBUG
	if (pItem && pName)
	{
		pItem->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(pName) + 1, pName);
	}
#endif
}


Device::Device()
{
	m_pDepthStencil = 0;
	m_pBackBuffer = 0;
	m_pDepthStencilBuffer = 0;
	m_pDevice = 0;
	m_pDeviceContext = 0;
	m_pSwapChain = 0;
}

bool Craze::Graphics2::InitGraphicsDevice(int width, int height, bool fullScreen, HWND hWnd)
{
	gpDevice = new Device();
	
	if (gpDevice)
	{
		return gpDevice->Init(width, height, fullScreen, hWnd);
	}

	return false;
}

void Craze::Graphics2::ShutdownGraphicsDevice()
{
	if (gpDevice)
	{
		gpDevice->Shutdown();
		delete gpDevice;
		gpDevice = 0;
	}
}

bool Device::Init(int width, int height, bool fullScreen, HWND hWnd)
{
	DXGI_SWAP_CHAIN_DESC swapDesc;
	ZeroMemory(&swapDesc, sizeof(swapDesc));
	swapDesc.BufferCount = 1;
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapDesc.BufferDesc.RefreshRate.Denominator = 60;
	swapDesc.BufferDesc.RefreshRate.Numerator = 1;
	swapDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapDesc.BufferDesc.Height = height;
	swapDesc.BufferDesc.Width = width;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_BACK_BUFFER;
	swapDesc.OutputWindow = hWnd;
	swapDesc.SampleDesc.Count = 1;
	swapDesc.SampleDesc.Quality = 0;
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapDesc.Windowed = !fullScreen;
	swapDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;


	unsigned int flags = 0;
#ifdef _DEBUG__
	flags = D3D11_CREATE_DEVICE_DEBUG;
#endif
	const D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0 };

	if (FAILED(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, flags, featureLevels, 3, D3D11_SDK_VERSION, &swapDesc, &m_pSwapChain, 
		&m_pDevice, &m_FeatureLevel, &m_pDeviceContext)))
	{
		LOG_ERROR("Unable to create graphics device");
		return false;
	}

	if (!GetBackBufferFromSwap())
	{
		return false;
	}

	if (!CreateDepthStencil(width, height))
	{
		return false;
	}

	SetBackBufferVP(width, height);

	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_CBufferManager.Initialize(this);

	return true;
}

Device::~Device()
{
	Shutdown();
}

void Device::Shutdown()
{
	if (m_pDeviceContext)
	{
		m_pDeviceContext->ClearState();
		m_pDeviceContext->Flush();
	}

	m_CBufferManager.Shutdown();
	m_pCurrentIndices = nullptr;
	m_CurrentStreams = nullptr;

	SAFE_RELEASE(m_pDepthShaderResource);
	SAFE_RELEASE(m_pDepthStencilBuffer);
	SAFE_RELEASE(m_pDepthStencil);
	SAFE_RELEASE(m_pBackBuffer);
	SAFE_RELEASE(m_pSwapChain);
	SAFE_RELEASE(m_pDeviceContext);
	SAFE_RELEASE(m_pDevice);
}

void Device::Swap()
{
	PROFILEF();
	m_pSwapChain->Present(0, 0);
}

void Device::Clear(const Vector4& color)
{
	PROFILEF();
	m_pDeviceContext->ClearRenderTargetView(m_pBackBuffer, (float*)&color);
	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilBuffer, D3D11_CLEAR_DEPTH, 1.0f, 0);

}

bool Device::GetBackBufferFromSwap()
{
	ID3D11Texture2D* pBackBuffer;
	if (FAILED(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer))))
	{
		LOG_ERROR("Could not get swap chain back buffer");
		return false;
	}

	if (FAILED(m_pDevice->CreateRenderTargetView(pBackBuffer, NULL, &m_pBackBuffer)))
	{
		LOG_ERROR("Could not create back buffer render target view");
		return false;
	}

	SAFE_RELEASE(pBackBuffer);

	return true;
}

bool Device::CreateDepthStencil(unsigned int width, unsigned int height)
{
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	depthStencilDesc.Height = height;
	depthStencilDesc.Width = width;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.MiscFlags = 0;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;


	if (FAILED(m_pDevice->CreateTexture2D(&depthStencilDesc, NULL, &m_pDepthStencil)))
	{
		LOG_ERROR("Could not create depth stencil buffer");
		return false;
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = 0;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	if (FAILED(m_pDevice->CreateDepthStencilView(m_pDepthStencil, &dsvDesc, &m_pDepthStencilBuffer)))
	{
		LOG_ERROR("Could not create depth stencil render target view");
		return false;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	if (FAILED(m_pDevice->CreateShaderResourceView(m_pDepthStencil, &srvDesc, &m_pDepthShaderResource)))
	{
		LOG_ERROR("Could not create shader resource view for depth buffer");
		return false;
	}
	return true;
}

void Device::SetBackBufferVP(unsigned int width, unsigned int height)
{
	m_pDeviceContext->OMSetRenderTargets(1, &m_pBackBuffer, m_pDepthStencilBuffer);

	D3D11_VIEWPORT viewport;
	viewport.Height = (float)height;
	viewport.Width = (float)width;
	viewport.MaxDepth = 1.0f;
	viewport.MinDepth = 0.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	m_pDeviceContext->RSSetViewports(1, &viewport);

	m_ViewPortSize = Vector2((float)width, (float)height);
}

void Device::SetResolution(unsigned int width, unsigned int height, bool fullScreen)
{
	m_pDeviceContext->ClearState();
	SAFE_RELEASE(m_pBackBuffer);
	SAFE_RELEASE(m_pDepthStencilBuffer);
	SAFE_RELEASE(m_pDepthStencil);

	m_pSwapChain->ResizeBuffers(1, width, height, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);

	GetBackBufferFromSwap();
	CreateDepthStencil(width, height);
	SetBackBufferVP(width, height);

	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_CBufferManager.ResetBuffers();

	m_pDeviceContext->VSSetShader(m_pCurrentVS, 0, 0);
	m_pDeviceContext->PSSetShader(m_pCurrentPS, 0, 0);

	m_CurrentStreams = nullptr;
	m_pCurrentIndices = nullptr;
}

ID3D11Device* Device::GetDevice()
{
	return m_pDevice;
}

ID3D11DeviceContext* Device::GetDeviceContext()
{
	return m_pDeviceContext;
}

void Device::SetVertexStreams(const VertexStreams* pStreams)
{
	PROFILEF();
	//assert(pVertexBuffer->GetType() == GeometryBuffer::VERTEX);

	if (pStreams != m_CurrentStreams)
	{
		static const unsigned int stride[] = { sizeof(Vertex), sizeof(Vector2), sizeof(SkinnedVertex) };
		static const unsigned int offset[] = { 0, 0, 0 };
		m_pDeviceContext->IASetVertexBuffers(0, VertexStreams::NUMSTREAMS, pStreams->GetStreams(), stride, offset);
		m_CurrentStreams = pStreams;
	}
}

void Device::SetIndexBuffer(std::shared_ptr<GeometryBuffer> pIndexBuffer)
{
	PROFILEF();
	assert(pIndexBuffer->GetType() == GeometryBuffer::INDEX);

	if (pIndexBuffer != m_pCurrentIndices)
	{
		m_pDeviceContext->IASetIndexBuffer(pIndexBuffer->GetBuffer(), DXGI_FORMAT_R16_UINT, 0);
		m_pCurrentIndices = pIndexBuffer;
	}
}

void Device::ClearCache()
{ 
	m_CurrentStreams = nullptr;
	m_pCurrentIndices = nullptr;
	m_pCurrentLayout = nullptr;
}

void Device::SetShader(ID3D11PixelShader* pPS)
{
	PROFILEF();
	if (pPS != m_pCurrentPS)
	{
		m_pCurrentPS = pPS;
		m_pDeviceContext->PSSetShader(pPS, NULL, 0);
	}
}

void Device::SetShader(ID3D11VertexShader* pVS)
{
	PROFILEF();
	if (pVS != m_pCurrentVS)
	{
		m_pCurrentVS = pVS;
		m_pDeviceContext->VSSetShader(pVS, NULL, 0);
	}
}

void Device::SetShader(ID3D11GeometryShader* pGS)
{
	PROFILEF();
	if (pGS != m_pCurrentGS)
	{
		m_pCurrentGS = pGS;
		m_pDeviceContext->GSSetShader(pGS, NULL, 0);
	}
}
void Device::SetShader(ID3D11HullShader* hs)
{
	PROFILEF();
	m_pDeviceContext->HSSetShader(hs, nullptr, 0);
}
void Device::SetShader(ID3D11DomainShader* ds)
{
	PROFILEF();
	m_pDeviceContext->DSSetShader(ds, nullptr, 0);
}

void Device::SetRenderTargets(std::shared_ptr<RenderTarget>* pRenderTargets, unsigned int numRenderTargets, ID3D11DepthStencilView* pDSV)
{
	PROFILEF();

	D3D11_VIEWPORT viewPorts[8];

	for(unsigned int i = 0; i < numRenderTargets && i < 8; ++i)
	{
		viewPorts[i].MaxDepth = 1.0f;
		viewPorts[i].MinDepth = 0.0f;
		viewPorts[i].TopLeftX = 0.0f;
		viewPorts[i].TopLeftY = 0.0f;

		if (pRenderTargets[i])
		{
			viewPorts[i].Height = (float)pRenderTargets[i]->GetHeight();
			viewPorts[i].Width = (float)pRenderTargets[i]->GetWidth();

			m_pRenderTargets[i] = pRenderTargets[i]->GetRenderTargetView();
		} else
		{
			m_pRenderTargets[i] = 0;
			viewPorts[i].Width = 0.0f;
			viewPorts[i].Height = 0.0f;
		}
	}

	m_pDeviceContext->OMSetRenderTargets(numRenderTargets, m_pRenderTargets, pDSV);
	m_pDeviceContext->RSSetViewports(numRenderTargets, viewPorts);
}

void Device::SetRenderTargets(const std::vector<std::shared_ptr<RenderTarget> > &pRenderTargets, std::shared_ptr<DepthStencil> pDS)
{
	PROFILEF();

	D3D11_VIEWPORT viewPorts[8];

	for(int i = 0; i < (int)pRenderTargets.size() && i < 8; ++i)
	{
		viewPorts[i].MaxDepth = 1.0f;
		viewPorts[i].MinDepth = 0.0f;
		viewPorts[i].TopLeftX = 0.0f;
		viewPorts[i].TopLeftY = 0.0f;

		if (pRenderTargets[i])
		{
			viewPorts[i].Height = (float)pRenderTargets[i]->GetHeight();
			viewPorts[i].Width = (float)pRenderTargets[i]->GetWidth();

			m_pRenderTargets[i] = pRenderTargets[i]->GetRenderTargetView();
		} else
		{
			if (i == 0)
			{
				viewPorts[i].Width = m_ViewPortSize.x;
				viewPorts[i].Height = m_ViewPortSize.y;

				m_pRenderTargets[i] = m_pBackBuffer;
			} else
			{
				m_pRenderTargets[i] = 0;
			}
		}
	}

	ID3D11DepthStencilView* pDSView = 0;

	if (pDS)
	{
		pDSView = pDS->GetDepthStencilView();
	}

	m_pDeviceContext->OMSetRenderTargets(pRenderTargets.size(), m_pRenderTargets, pDSView);
	m_pDeviceContext->RSSetViewports(pRenderTargets.size(), viewPorts);

}

void Device::SetRenderTarget(std::shared_ptr<RenderTarget> pRenderTarget, std::shared_ptr<DepthStencil> pDS)
{
	PROFILEF();

	D3D11_VIEWPORT viewPort;
	viewPort.MaxDepth = 1.0f;
	viewPort.MinDepth = 0.0f;
	viewPort.TopLeftX = 0.0f;
	viewPort.TopLeftY = 0.0f;

	ID3D11DepthStencilView* pDSView = 0;

	if (pDS)
	{
		pDSView = pDS->GetDepthStencilView();
	}

	if (pRenderTarget)
	{
		viewPort.Height = (float)pRenderTarget->GetHeight();
		viewPort.Width = (float)pRenderTarget->GetWidth();
		ID3D11RenderTargetView* pView = pRenderTarget->GetRenderTargetView();
		m_pDeviceContext->OMSetRenderTargets(1, &pView, pDSView);
	} else
	{
		viewPort.Height = m_ViewPortSize.y;
		viewPort.Width = m_ViewPortSize.x;
		m_pDeviceContext->OMSetRenderTargets(1, &m_pBackBuffer, pDSView);
	}

	m_pDeviceContext->RSSetViewports(1, &viewPort);
}

void Device::SetRenderTargets(const std::vector<std::shared_ptr<RenderTarget> > &pRenderTargets)
{
	PROFILEF();

	D3D11_VIEWPORT viewPorts[8];

	for(int i = 0; i < (int)pRenderTargets.size() && i < 8; ++i)
	{
		viewPorts[i].MaxDepth = 1.0f;
		viewPorts[i].MinDepth = 0.0f;
		viewPorts[i].TopLeftX = 0.0f;
		viewPorts[i].TopLeftY = 0.0f;

		if (pRenderTargets[i])
		{
			viewPorts[i].Height = (float)pRenderTargets[i]->GetHeight();
			viewPorts[i].Width = (float)pRenderTargets[i]->GetWidth();

			m_pRenderTargets[i] = pRenderTargets[i]->GetRenderTargetView();
		} else
		{
			if (i == 0)
			{
				viewPorts[i].Width = m_ViewPortSize.x;
				viewPorts[i].Height = m_ViewPortSize.y;

				m_pRenderTargets[i] = m_pBackBuffer;
			} else
			{
				m_pRenderTargets[i] = 0;
			}
		}
	}

	m_pDeviceContext->OMSetRenderTargets(pRenderTargets.size(), m_pRenderTargets, m_pDepthStencilBuffer);
	m_pDeviceContext->RSSetViewports(pRenderTargets.size(), viewPorts);

}

void Device::SetRenderTarget(std::shared_ptr<RenderTarget> pRenderTarget)
{
	PROFILEF();

	D3D11_VIEWPORT viewPort;
	viewPort.MaxDepth = 1.0f;
	viewPort.MinDepth = 0.0f;
	viewPort.TopLeftX = 0.0f;
	viewPort.TopLeftY = 0.0f;

	if (pRenderTarget)
	{
		viewPort.Height = (float)pRenderTarget->GetHeight();
		viewPort.Width = (float)pRenderTarget->GetWidth();
		ID3D11RenderTargetView* pView = pRenderTarget->GetRenderTargetView();
		m_pDeviceContext->OMSetRenderTargets(1, &pView, m_pDepthStencilBuffer);
	} else
	{
		viewPort.Height = m_ViewPortSize.y;
		viewPort.Width = m_ViewPortSize.x;
		m_pDeviceContext->OMSetRenderTargets(1, &m_pBackBuffer, m_pDepthStencilBuffer);
	}

	m_pDeviceContext->RSSetViewports(1, &viewPort);
}

void Device::SetObjectProperties(const Craze::Matrix4& world, const Material& material)
{
	PROFILEF();

	m_pCurrentEffect->setObjectProperties(world, material);
}

void Device::SetCurrentEffect(IEffect* pEffect)
{
	m_pDeviceContext->IASetInputLayout(pEffect->getInputLayout());
	m_pCurrentEffect = pEffect;
}

IEffect* Device::GetCurrentEffect()
{
	return m_pCurrentEffect;
}

void Device::DrawIndexed()
{
	PROFILEF();

	m_pDeviceContext->DrawIndexed(m_pCurrentIndices->GetNumElems(), m_pCurrentIndices->GetFirstIndex(), m_CurrentStreams->GetFirstIndex());
	m_TriCount += m_pCurrentIndices->GetNumElems() / 3;
	m_VertexCount += m_CurrentStreams->GetNumElems();

}

void Device::Draw()
{
	PROFILEF();
	
	int num = m_CurrentStreams->GetNumElems();
	int first = m_CurrentStreams->GetFirstIndex();
	m_pDeviceContext->Draw(num, first);
	m_VertexCount += m_CurrentStreams->GetNumElems();
}

void Device::drawBuffer(std::shared_ptr<GeometryBuffer> buffer)
{
	PROFILEF();

	unsigned int stride = buffer->GetStride();
	unsigned int offset = 0;
	ID3D11Buffer* vtxBuffer = buffer->GetBuffer();
	m_pDeviceContext->IASetVertexBuffers(0, 1, &vtxBuffer, &stride, &offset);

	m_pDeviceContext->Draw(buffer->GetNumElems(), buffer->GetFirstIndex());
	m_VertexCount += buffer->GetNumElems();

	m_CurrentStreams = nullptr;
}

D3D_FEATURE_LEVEL Device::GetFeatureLevel() const
{
	return m_FeatureLevel;
}

Craze::Vector2 Device::GetViewPort() const
{
	return m_ViewPortSize;
}