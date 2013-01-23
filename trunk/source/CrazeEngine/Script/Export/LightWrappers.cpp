#include "CrazeEngine.h"
#include "LightWrappers.h"

using namespace Craze;
using namespace Craze::Graphics2;

ScriptDirectionalLight::ScriptDirectionalLight(HLIGHT light, Scene* scene) : m_light(light), m_scene(scene)
{
}

ScriptDirectionalLight::~ScriptDirectionalLight()
{
    remove();
}

void ScriptDirectionalLight::remove()
{
    if (m_scene)
    {
        m_scene->removeDirectionalLight(m_light);
        m_light = nullptr;
        m_scene = nullptr;
    }
}

DirectionalLight* ScriptDirectionalLight::getLight() const
{
    if (m_scene)
    {
        return m_scene->getDirectionalLight(m_light);
    } else
    {
        return nullptr;
    }
}

void ScriptDirectionalLight::setDirection(const Vec3& dir)
{
    auto light = getLight();
    if (light)
    {
        light->prevDir = light->dir;
        light->dir = dir;
    }
}
void ScriptDirectionalLight::setColor(const Vec3& color)
{
    auto light = getLight();
    if (light)
    {
        light->color = color;
    }
}

const Vec3 ScriptDirectionalLight::getDirection() const
{
    auto light = getLight();
    if (light)
    {
        return light->dir;
    }
    return Vec3();
}

const Vec3 ScriptDirectionalLight::getColor() const
{
    auto light = getLight();
    if (light)
    {
        return light->color;
    }
    return Vec3();
}


ScriptSpotLight::ScriptSpotLight(HLIGHT light, Scene* scene) : m_light(light), m_scene(scene)
{
}

ScriptSpotLight::~ScriptSpotLight()
{
    remove();
}

void ScriptSpotLight::remove()
{
    if (m_scene)
    {
        m_scene->removeSpotLight(m_light);
        m_light = nullptr;
        m_scene = nullptr;
    }
}

SpotLight* ScriptSpotLight::getLight() const
{
    if (m_scene)
    {
        return m_scene->getSpotLight(m_light);
    }
    return nullptr;
}

void ScriptSpotLight::setPosition(const Vec3& pos)
{
    auto light = getLight();
    if (light)
    {
        light->prevPos = light->pos;
        light->pos = pos;
    }
}
void ScriptSpotLight::setDirection(const Vec3& dir)
{
    auto light = getLight();
    if (light)
    {
        light->prevDir = light->direction;
        light->direction = dir;
    }
}
void ScriptSpotLight::setColor(const Vec3& color)
{
    auto light = getLight();
    if (light)
    {
        light->color = color;
    }
}
void ScriptSpotLight::setAngle(float angle)
{
    auto light = getLight();
    if (light)
    {
        light->angle = angle;
    }
}
void ScriptSpotLight::setRange(float range)
{
    auto light = getLight();
    if (light)
    {
        light->range = range;
    }
}

const Vec3 ScriptSpotLight::getPosition() const
{
    auto light = getLight();
    if (light)
    {
        return light->pos;
    }
    return Vec3();
}
const Vec3 ScriptSpotLight::getDirection() const
{
    auto light = getLight();
    if (light)
    {
        return light->direction;
    }
    return Vec3();
}
const Vec3 ScriptSpotLight::getColor() const
{
    auto light = getLight();
    if (light)
    {
        return light->color;
    }
    return Vec3();
}
const float ScriptSpotLight::getAngle() const
{
    auto light = getLight();
    if (light)
    {
        return light->angle;
    }
    return 0.f;
}
const float ScriptSpotLight::getRange() const
{
    auto light = getLight();
    if (light)
    {
        return light->range;
    }
    return 0.f;
}