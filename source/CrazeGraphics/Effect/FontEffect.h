#pragma once
#include "IEffect.h"

namespace Craze
{
	namespace Graphics2
	{
		class Device;

		class FontEffect : public IEffect
		{
		public:

			bool initialize() { return IEffect::initialize("Font.vsh", "Font.psh", "Font.gsh"); }

			void set() { IEffect::set(); }
			void setObjectProperties(const Matrix4& world, const Material& material);

			void reset() { IEffect::reset(); }

			void destroy() { IEffect::destroy(); }

		protected:
			const D3D11_INPUT_ELEMENT_DESC* getLayout(int& count);
		};
	}
}