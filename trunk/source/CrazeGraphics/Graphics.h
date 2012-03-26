#pragma once

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x) if(x) { (x)->Release(); (x) = NULL; }
#endif

namespace Craze
{
	namespace Graphics2
	{	
		class Scene;
		extern class Graphics* gpGraphics;
		extern class Resources gGfxResources;
		extern class Renderer* gpRenderer;

		extern class CopyToBackBuffer gFxCopyToBack;
		extern class FontEffect gFxFont;
		extern class GBufferEffect gFxGBuffer;
		extern class ShadingEffect gFxShading;
		extern class ShadowEffect gFxShadow;
		extern class AmbientLighting gFxAmbientLighting;
		extern class CSLighting gFxCSLighting;
		extern class LightingEffect gFxLighting;
		extern class PosMapEffect gFxPosMap;
		extern class NullEffect gFxNull;
		extern class DebugDrawEffect gFxDebugDraw;
		extern class DrawRaysEffect gFxDrawRays;
		extern class LVAmbientLightingEffect gFxLVAmbientLighting;

		struct GfxParams
		{
			GfxParams()
			{
				ShowLAB = false;
				DisableDirect = false;
				DisableAO = false;
				DisableIndirect = false;
				BoostIndirect = false;
				UseESM = true;
			}

			bool ShowLAB;
			bool DisableDirect;
			bool DisableAO;
			bool DisableIndirect;
			bool BoostIndirect;
			bool UseESM;
		};

		bool InitGraphics(HWND hWnd, unsigned int width, unsigned int height);
		void ShutdownGraphics();

		class Graphics
		{
			friend bool InitGraphics(HWND hWnd, unsigned int width, unsigned int height);
			friend void ShutdownGraphics();
		public:
		
			void Render();
			void BindScene(Scene* pScene);

			GfxParams Params;

		private:
			Graphics();
			~Graphics();

			Scene* m_pScene;
		};

		extern bool gUseIndirectLighting;
		extern bool gUseShadows;
		extern bool gUseDirectLighting;
		extern bool gUseConstantAmbient;
		extern bool gDrawRays;
	}
}