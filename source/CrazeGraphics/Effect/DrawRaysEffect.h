#pragma once
#include "IEffect.h"

namespace Craze
{
	namespace Graphics2
	{
		class DrawRaysEffect : public IEffect
		{
		public:

			bool initialize() { return IEffect::initialize("RayTracing/DrawRay.vsh", "RayTracing/DrawRay.psh"); }
			void set() { IEffect::set(); }
			void destroy() { IEffect::destroy(); }

			void setObjectProperties(const Matrix4& tfm, const Material& mat) {}

		protected:

			virtual const D3D11_INPUT_ELEMENT_DESC* getLayout(int& count);
		};
	}
}