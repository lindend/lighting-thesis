#pragma once

#include "IEffect.h"

namespace Craze
{
	namespace Graphics2
	{
		class ShadowEffect : public IEffect
		{
		public:
			bool initialize() { return IEffect::initialize("ESM_Mesh.vsh", "ESM.psh"); }
			
			void set() { IEffect::set(); }

			void setObjectProperties(const Matrix4& world, const Material& material);

			void destroy() { IEffect::destroy(); }
		};
	}
}