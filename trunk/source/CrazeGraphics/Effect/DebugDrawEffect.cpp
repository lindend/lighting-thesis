#include "CrazeGraphicsPCH.h"
#include "DebugDrawEffect.h"
#include "../Graphics.h"
#include "../EffectUtil/CBufferHelper.hpp"
#include "../Scene/Camera.h"

using namespace Craze::Graphics2;
using namespace Craze;


void DebugDrawEffect::setObjectProperties(const Matrix4& world, const Material& material)
{
	Matrix4 worldView = world * m_camera->GetView();
	CBPerObject data;
	data.specularFactor = material.m_specular;
	data.world = worldView * m_camera->GetProjection();
	data.normalTfm = Transpose(Inverse(Matrix3(worldView)));

	gpDevice->GetCbuffers()->SetObject(data);
}