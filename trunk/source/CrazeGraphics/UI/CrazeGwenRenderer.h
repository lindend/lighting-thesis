#pragma once
#include "Memory/MemoryManager.h"
#include "gwen/BaseRender.h"

#include "CrazeMath.h"

#include "../Device.h"
#include <map>
#include <memory>

/*
@todo
Implement ICacheToTexture
*/
namespace Craze
{
	class Resource;
	namespace Graphics2
	{
		class TextureResource;
		class GeometryBuffer;
		class VertexShaderResource;
		class PixelShaderResource;

		struct UIQuadInstance
		{
			UIQuadInstance(const Vector4 &pos, const Vector4 &uv, unsigned int color) : pos(pos), uv(uv), color(color)
			{
			}

			Vec4 pos;
			Vec4 uv;
			unsigned int color;
			Vec3 _pad;
		};

		class CrazeGwenRenderer : public Gwen::Renderer::Base
		{
			CRAZE_ALLOC_ALIGN(16);
		public:
			bool Initialize();
			void Shutdown();

			virtual void Begin();
			virtual void End();

			virtual void SetDrawColor(Gwen::Color color);

			//According to my research, this function is not actually used by Gwen!
			virtual void DrawLine(int x, int y, int a, int b) { assert(false); }

			virtual void DrawFilledRect(Gwen::Rect rect);

			virtual void StartClip();
			virtual void EndClip();

			virtual void LoadTexture(Gwen::Texture* pTexture);
			virtual void FreeTexture(Gwen::Texture* pTexture);
			virtual void DrawTexturedRect(Gwen::Texture* pTexture, Gwen::Rect pTargetRect, float u1=0.0f, float v1=0.0f, float u2=1.0f, float v2=1.0f);
			virtual void DrawMissingImage(Gwen::Rect pTargetRect);

			//virtual Gwen::Renderer::ICacheToTexture* GetCTT() { return NULL; }

			virtual void LoadFont(Gwen::Font* pFont);
			virtual void FreeFont(Gwen::Font* pFont);
			virtual void RenderText(Gwen::Font* pFont, Gwen::Point pos, const Gwen::UnicodeString& text);
			virtual Gwen::Point MeasureText(Gwen::Font* pFont, const Gwen::UnicodeString& text);

			//virtual void DrawLinedRect( Rect rect );
			//virtual void DrawPixel( int x, int y );
			//virtual void DrawShavedCornerRect( Gwen::Rect rect, bool bSlight = false );
			virtual Gwen::Point MeasureText(Gwen::Font* pFont, const Gwen::String& text);
			virtual void RenderText(Gwen::Font* pFont, Gwen::Point pos, const Gwen::String& text);

		private:
			void AddQuad(const UIQuadInstance &quad);
			void Flush();

			std::map<void*, std::shared_ptr<const Resource>> m_TexResources;

			std::vector<UIQuadInstance> m_Quads;
			static const int QuadBufferSize = 4096;

			std::shared_ptr<GeometryBuffer> m_pQuad;
			std::shared_ptr<GeometryBuffer> m_pInstanceBuffer;

			unsigned int m_ActiveColor;
			ID3D11BlendState* m_pBlendState;
			ID3D11DepthStencilState* m_pDepthState;
			ID3D11RasterizerState* m_pRastState;
			ID3D11InputLayout* m_pInputLayout;

			std::shared_ptr<const VertexShaderResource> m_pVertexShader;
			std::shared_ptr<const PixelShaderResource> m_pPixelShader;
			ID3DBlob* m_pShaderByteCode;

			TextureResource *m_pCurrentTexture;
		};
	}
}