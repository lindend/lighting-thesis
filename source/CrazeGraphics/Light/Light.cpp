#include "CrazeGraphicsPCH.h"
#include "Light.h"

using namespace Craze;
using namespace Craze::Graphics2;


PointLight Craze::Graphics2::CreatePointLight(const Vector3& pos, float range, const Vector3& color)
{
	PointLight l;
	l.pos = pos;
	l.range = range;
	l.color = color;
	l.specular = 1.f;
	return l;
}

DirectionalLight Craze::Graphics2::createDirectionalLight(const Vector3& dir, const Vector3& color)
{
	DirectionalLight l;
	l.dir = dir;
	l.color = color;
	l.specular = 1.f;
	return l;
}

SpotLight Craze::Graphics2::createSpotLight(const Vector3& pos, const Vector3& dir, float angle, float range, const Vector3& color)
{
	SpotLight l;
	l.pos = pos;
	l.direction = dir;
	l.angle = angle;
	l.color = color;
	l.range = range;
	l.specular = 1.f;
	return l;
}