#include "CrazeGraphicsPCH.h"
#include "CrazeGwenRenderer.h"

#include "gwen/Gwen.h"
#include "gwen/Texture.h"

#include "Resource/ResourceManager.h"
#include "Util/Type/StringHashType.h"

#include "../Graphics.h"
#include "../Effect/FontEffect.h"
#include "../Buffer/Buffer.h"
#include "../Font/FontManager.h"
#include "../EffectUtil/EffectHelper.h"

using namespace Craze;
using namespace Craze::Graphics2;

const D3D11_INPUT_ELEMENT_DESC UIVertDesc[] =  {
	{"OFFSET",	0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA,	0},
	{"POSITION",0, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
	{"UV",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
	{"COLOR",	0, DXGI_FORMAT_R8G8B8A8_UNORM,		1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1}
};
bool CrazeGwenRenderer::Initialize()
{
	CD3D11_BLEND_DESC desc = CD3D11_BLEND_DESC(CD3D11_DEFAULT());
	desc.RenderTarget[0].BlendEnable = true;
	desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	if (FAILED(gpDevice->GetDevice()->CreateBlendState(&desc, &m_pBlendState)))
	{
		LOG_ERROR("Unable to create blend state for UI renderer");
		return false;
	}

	CD3D11_DEPTH_STENCIL_DESC dsDesc = CD3D11_DEPTH_STENCIL_DESC(CD3D11_DEFAULT());
	dsDesc.DepthEnable = false;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	if (FAILED(gpDevice->GetDevice()->CreateDepthStencilState(&dsDesc, &m_pDepthState)))
	{
		LOG_ERROR("Unable to create depth stencil state for UI renderer");
		return false;
	}

	CD3D11_RASTERIZER_DESC rsDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());
	rsDesc.ScissorEnable = false;
	rsDesc.CullMode = D3D11_CULL_NONE;
	if (FAILED(gpDevice->GetDevice()->CreateRasterizerState(&rsDesc, &m_pRastState)))
	{
		LOG_ERROR("Unable to create rasterizer state for UI renderer");
		return false;
	}

	m_pVertexShader = EffectHelper::LoadShaderFromResource<VertexShaderResource>("UI.vsh");
	m_pPixelShader = EffectHelper::LoadShaderFromResource<PixelShaderResource>("UI.psh");

	if (!(m_pVertexShader && m_pPixelShader))
	{
		LOG_ERROR("Unable to create shaders for UI renderer");
		return false;
	}

	if (FAILED(gpDevice->GetDevice()->CreateInputLayout(UIVertDesc, 4, m_pVertexShader->m_shaderByteCode->GetBufferPointer(), m_pVertexShader->m_shaderByteCode->GetBufferSize(), &m_pInputLayout)))
	{
		LOG_ERROR("Unable to create input layout for UI shader");
		return false;
	}

	m_pInstanceBuffer = GeometryBuffer::Create(gpDevice, GeometryBuffer::VERTEX, nullptr, sizeof(UIQuadInstance), QuadBufferSize, true, "UI instance buffer");
	if (!m_pInstanceBuffer)
	{
		LOG_ERROR("Unable to create instance buffer for UI renderer");
		return false;
	}

	Vector2 quadData[] = {	Vector2(0.f, 0.f), Vector2(0.f, 1.f), Vector2(1.f, 1.f),
							Vector2(0.f, 0.f), Vector2(1.f, 1.f), Vector2(1.f, 0.f) };
	m_pQuad = GeometryBuffer::Create(gpDevice, GeometryBuffer::VERTEX, quadData, sizeof(Vector2), 6, false, "UI quad");
	if (!m_pQuad)
	{
		LOG_ERROR("Unable to create quad for UI renderer");
		return false;
	}

	m_ActiveColor = 0xFFFFFFFF;
	return true;
}

void CrazeGwenRenderer::Shutdown()
{
	m_pQuad = nullptr;
	m_pInstanceBuffer = nullptr;

	m_pInputLayout->Release();
	m_pRastState->Release();
	m_pDepthState->Release();
	m_pBlendState->Release();
}

void CrazeGwenRenderer::Begin()
{
	const float bf[4] = {1.f, 1.f, 1.f, 1.f};
	gpDevice->GetDeviceContext()->OMSetBlendState(m_pBlendState, bf, 0xFFFFFFFF);
	gpDevice->GetDeviceContext()->RSSetState(m_pRastState);
	gpDevice->GetDeviceContext()->OMSetDepthStencilState(m_pDepthState, 0);
	gpDevice->SetShader(m_pVertexShader->m_shader);
	gpDevice->SetShader(m_pPixelShader->m_shader);
	m_pCurrentTexture = 0;
}

void CrazeGwenRenderer::End()
{
	Flush();
	const float bf[4] = {1.f, 1.f, 1.f, 1.f};
	gpDevice->GetDeviceContext()->OMSetBlendState(nullptr, bf, 0xFFFFFFFF);
	gpDevice->GetDeviceContext()->RSSetState(nullptr);
	gpDevice->GetDeviceContext()->OMSetDepthStencilState(nullptr, 0);
}

void CrazeGwenRenderer::AddQuad(const UIQuadInstance& quad)
{
	m_Quads.push_back(quad);
	if (m_Quads.size() == QuadBufferSize)
	{
		Flush();
	}
}

void CrazeGwenRenderer::Flush()
{
	if (m_Quads.size() == 0)
	{
		return;
	}

	m_pInstanceBuffer->WriteData(&m_Quads[0], m_Quads.size());

	if (m_pCurrentTexture)
	{
		ID3D11ShaderResourceView* pSRV = m_pCurrentTexture->m_texture->GetResourceView();
		gpDevice->GetDeviceContext()->PSSetShaderResources(0, 1, &pSRV);
	}

	ID3D11Buffer* const pBuffers[] = { *m_pQuad, *m_pInstanceBuffer };
	const unsigned int strides[] = { m_pQuad->GetStride(), m_pInstanceBuffer->GetStride() };
	const unsigned int offsets[] = { 0, 0 };

	gpDevice->GetDeviceContext()->IASetInputLayout(m_pInputLayout);
	gpDevice->GetDeviceContext()->IASetVertexBuffers(0, 2, pBuffers, strides, offsets);
	//Since we are calling IASetVertexBuffers ourselves, we must clear the vertex buffer cache in the device
	gpDevice->ClearCache();
	gpDevice->GetDeviceContext()->DrawInstanced(6, m_Quads.size(), 0, m_pInstanceBuffer->GetFirstIndex());

	ID3D11Buffer *pNull = nullptr;
	gpDevice->GetDeviceContext()->IASetVertexBuffers(1, 1, &pNull, strides, offsets);

	m_Quads.clear();
}

void CrazeGwenRenderer::SetDrawColor(Gwen::Color color)
{
	m_ActiveColor = color.a << 24 | color.r << 16 | color.g << 8 | color.b;
}

void CrazeGwenRenderer::DrawFilledRect(Gwen::Rect rect)
{
	Translate(rect);
	AddQuad(UIQuadInstance(Vector4(rect.x, rect.y, rect.w, rect.h), Vector4(0.f, 0.f, 0.f, 0.f), m_ActiveColor));
}

void CrazeGwenRenderer::StartClip()
{
	Flush();
	return;
	Gwen::Rect clip = ClipRegion();
	Translate(clip);
	D3D11_RECT r;
	r.left = clip.x;
	r.top = clip.y;
	r.right = clip.x + clip.w;
	r.bottom = clip.y + clip.h;

	gpDevice->GetDeviceContext()->RSSetScissorRects(1, &r);	
}

void CrazeGwenRenderer::EndClip()
{
	Flush();
	return;
	gpDevice->GetDeviceContext()->RSSetScissorRects(0, nullptr);
}

void CrazeGwenRenderer::LoadTexture(Gwen::Texture* pTexture)
{
	const TextureResource* res = dynamic_cast<const TextureResource*>(gResMgr.loadResourceBlocking(gFileDataLoader.addFile(pTexture->name.Get())));

	if (res)
	{
		pTexture->data = res->m_texture;
		pTexture->height = res->m_texture->GetHeight();
		pTexture->width = res->m_texture->GetWidth();

		m_TexResources[pTexture] = res;
		return;
	} 
	pTexture->failed = true;
}
void CrazeGwenRenderer::FreeTexture(Gwen::Texture* pTexture)
{
	const Resource* res = m_TexResources[pTexture];
	res->release();
}

void CrazeGwenRenderer::DrawTexturedRect(Gwen::Texture* pTexture, Gwen::Rect rect, float u1, float v1, float u2, float v2)
{
	TextureResource* pData = (TextureResource*)pTexture->data;

	if (m_pCurrentTexture && pData != m_pCurrentTexture)
	{
		Flush();
	}
	m_pCurrentTexture = pData;
	
	Translate(rect);
	AddQuad(UIQuadInstance(Vector4(rect.x, rect.y, rect.w, rect.h), Vector4(u1, v1, u2, v2), m_ActiveColor));
}

void CrazeGwenRenderer::DrawMissingImage(Gwen::Rect rect)
{
	Translate(rect);
	AddQuad(UIQuadInstance(Vector4(rect.x, rect.y, rect.w, rect.h), Vector4(0.f, 0.f, 0.f, 0.f), 0xFF0000FF));
}

void CrazeGwenRenderer::LoadFont(Gwen::Font* pFont)
{
	std::string fontName = WToStr(pFont->facename);
	pFont->realsize = pFont->size * Scale();
	Face *pFace = gFontMgr.createFace(fontName, pFont->bold ? "Bold" : "Regular", pFont->realsize);
	pFont->data = pFace;
}

void CrazeGwenRenderer::FreeFont(Gwen::Font* pFont)
{
	Face *pFace = (Face *)pFont->data;
	delete pFace;
}

void CrazeGwenRenderer::RenderText(Gwen::Font* pFont, Gwen::Point pos, const Gwen::UnicodeString& text)
{
	RenderText(pFont, pos, WToStr(text));
}
Gwen::Point CrazeGwenRenderer::MeasureText(Gwen::Font* pFont, const Gwen::UnicodeString& text)
{
	return MeasureText(pFont, WToStr(text));
}

Gwen::Point CrazeGwenRenderer::MeasureText(Gwen::Font* pFont, const Gwen::String& text)
{
	if (!pFont->data || fabs( pFont->realsize - pFont->size * Scale() ) > 2 )
	{
		FreeFont(pFont);
		LoadFont(pFont);
	}

	if (!pFont->data)
	{
		return Gwen::Point();
	}

	Face *pFace = (Face *)pFont->data;
	unsigned int width = pFace->getStringWidth(text);
	return Gwen::Point(width, pFace->getSize());
}

void CrazeGwenRenderer::RenderText(Gwen::Font* pFont, Gwen::Point pos, const Gwen::String& text)
{
	if (!pFont->data || fabs( pFont->realsize - pFont->size * Scale() ) > 2 )
	{
		FreeFont(pFont);
		LoadFont(pFont);
	}

	if (!pFont->data)
	{
		return;
	}

	Flush();
	gFxFont.set();

	Translate(pos.x, pos.y);
	Face *pFace = (Face *)pFont->data;
	pFace->renderString(text, pos.x, pos.y, Vector3(1.f, 1.f, 1.f));
	
	gFxFont.reset();

	gpDevice->SetShader(m_pVertexShader->m_shader);
	gpDevice->SetShader(m_pPixelShader->m_shader);
}