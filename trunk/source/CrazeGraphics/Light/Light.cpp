#include "CrazeGraphicsPCH.h"
#include "Light.h"

using namespace Craze;
using namespace Craze::Graphics2;


Light Craze::Graphics2::CreatePointLight(const Vector3& pos, float range, const Vector3& color)
{
	Light l;
	l.type = POINTLIGHT;
	l.pos = pos;
	l.pos->w = range;
	l.color = color;
	return l;
}

Light Craze::Graphics2::createDirectionalLight(const Vector3& dir, const Vector3& color)
{
	Light l;
	l.type = DIRECTIONALLIGHT;
	l.dir = dir;
	l.color = color;
	return l;
}