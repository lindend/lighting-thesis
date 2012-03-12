#include "CrazeGraphicsPCH.h"	

using namespace Craze;
using namespace Graphics2;

#include "Device.h";

void render(std::shared_ptr<UAVBuffer> rays)
{
	for (auto i = rays.begin(); i != rays.end(); ++i)
	{
		gFxGBuffer.setObjectProperties(*i->second.m_transform, *i->second.m_material);
		i->second.m_mesh->draw();
	}
}


