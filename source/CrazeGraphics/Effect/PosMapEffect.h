#pragma once

#include "IEffect.h"

namespace Craze
{
	namespace Graphics2
	{
		class PosMapEffect : public IEffect
		{
		public:
			bool initialize() { return IEffect::initialize("PosMap.vsh", "PosMap.psh"); }
			void set() { IEffect::set(); }
			void destroy() { IEffect::destroy(); }

			void setObjectProperties(const Matrix4& tfm, const Material& mat) {}
		};
	}
}