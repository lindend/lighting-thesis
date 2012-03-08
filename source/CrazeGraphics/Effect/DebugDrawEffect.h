#pragma once
#include "IEffect.h"

namespace Craze
{
	namespace Graphics2
	{
		class Device;
		class Camera;

		class DebugDrawEffect : public IEffect
		{
		public:
			bool initialize() 
			{ 
				return IEffect::initialize("DefaultMesh.vsh", "DebugDraw.psh"); 
			}

			void set(const Camera* cam) { m_camera = cam; return IEffect::set(); }
			void setObjectProperties(const Matrix4& world, const Material& material);

			void reset() { IEffect::reset(); }

			void destroy() { IEffect::destroy(); }
		private:
			const Camera* m_camera;
		};
	}
}