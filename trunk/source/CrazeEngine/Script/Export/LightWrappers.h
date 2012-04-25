#pragma once

#include "CrazeGraphics/Light/Light.h"
#include "Scene/Scene.h"

namespace Craze
{
    class ScriptDirectionalLight
    {
    public:
        ScriptDirectionalLight(Graphics2::HLIGHT light, Graphics2::Scene* scene);
        ~ScriptDirectionalLight();

        void setDirection(const Vec3& dir);
        void setColor(const Vec3& color);

        const Vec3 getDirection() const;
        const Vec3 getColor() const;

        void remove();

    private:
        Graphics2::DirectionalLight* getLight() const;

        Graphics2::HLIGHT m_light;
        Graphics2::Scene* m_scene;
    };

    class ScriptSpotLight
    {
    public:
        ScriptSpotLight(Graphics2::HLIGHT light, Graphics2::Scene* scene);
        ~ScriptSpotLight();

        void setPosition(const Vec3& pos);
        void setDirection(const Vec3& dir);
        void setColor(const Vec3& color);
        void setAngle(float angle);
        void setRange(float range);

        const Vec3 getPosition() const;
        const Vec3 getDirection() const;
        const Vec3 getColor() const;
        const float getAngle() const;
        const float getRange() const;

        void remove();

    private:
        Graphics2::SpotLight* getLight() const;

        Graphics2::HLIGHT m_light;
        Graphics2::Scene* m_scene;
    };
}