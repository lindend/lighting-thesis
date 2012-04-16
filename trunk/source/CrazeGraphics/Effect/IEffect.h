#pragma once
#include "../EffectUtil/EffectHelper.h"
#include "../EffectUtil/ShaderResource.h"

#include "CrazeMath.h"

#include "../Material.h"

namespace Craze
{
	namespace Graphics2
	{
		class LayoutManager;

		class IEffect
		{
		public:
			IEffect() : m_vs(0), m_gs(0), m_ps(0), m_byteCode(0), m_inputLayout(0) {}
			virtual ~IEffect() { destroy(); }

			virtual void setObjectProperties(const Matrix4& world, const Material& material) { }

			ID3DBlob* getShaderByteCode() { return m_byteCode; }
			ID3D11InputLayout* getInputLayout() { return m_inputLayout; }

			void set();
			void destroy();
			void reset();

			bool initialize(const char* vsFile, const char* psFile, const char* gsFile = 0);

		protected:

			virtual const D3D11_INPUT_ELEMENT_DESC* getLayout(int& count);

			ID3DBlob* m_byteCode;
			ID3D11InputLayout* m_inputLayout;

			std::shared_ptr<const VertexShaderResource> m_vs;
			std::shared_ptr<const GeometryShaderResource> m_gs;
			std::shared_ptr<const PixelShaderResource> m_ps;

		private:
			IEffect(const IEffect&);
			IEffect& operator=(const IEffect&);
		};
	}
}