#include "Image.h"

#include "Graphics/Geometry/GeometryData.h"
#include "EventLogger.h"
#include "Graphics/Geometry/IMesh.h"
#include "Graphics/Assets/Material.h"
#include "Graphics/Texture/TextureId.h"
#include "Graphics/Device/IDevice.h"
#include "Math/Matrix4.h"
#include "Math/Vector2.h"

using Craze::Graphics::UI::Image;
using std::string;
using Craze::Math::Vector2;
using Craze::Math::Vector3;
using namespace Craze;

Image::Image(Craze::Graphics::IDevice *pDevice, string name) : m_Material("")
{
	m_Name = name;
	m_pMesh = NULL;

	m_Params.clickable = false;
	m_Params.depth = 1;
	m_Params.dragable = false;
	m_Params.level = Craze::Graphics::UI::CL_MID;
	m_Params.visible = true;

	m_Transform.SetToIdentity();

	SetPosition(0.0f, 0.0f);
	SetSize(0.1f, 0.1f);

	m_Material.SetMaterialId(Craze::Graphics::MATERIAL_UI);

	CreateMesh(pDevice);

}

Image::Image(Craze::Graphics::IDevice *pDevice, float xpos, float ypos, string name) : m_Material("")
{
	m_Name = name;
	m_pMesh = NULL;
	m_Position = Vector2(xpos, ypos);
	m_Params.clickable = false;
	m_Params.depth = 1;
	m_Params.dragable = false;
	m_Params.level = Craze::Graphics::UI::CL_MID;
	m_Params.visible = true;

	m_Transform.SetToIdentity();

	SetPosition(xpos, ypos);
	SetSize(0.1f, 0.1f);

	m_Material.SetMaterialId(Craze::Graphics::MATERIAL_UI);

	CreateMesh(pDevice);
}

Image::Image(Craze::Graphics::IDevice *pDevice, float xpos, float ypos, float width, float height, std::string name) : m_Material("")
{
	m_Name = name;
	m_pMesh = NULL;

	m_Params.clickable = false;
	m_Params.depth = 1;
	m_Params.dragable = false;
	m_Params.level = Craze::Graphics::UI::CL_MID;
	m_Params.visible = true;

	m_Transform.SetToIdentity();

	SetPosition(xpos, ypos);
	SetSize(width, height);

	m_Material.SetMaterialId(Craze::Graphics::MATERIAL_UI);

	CreateMesh(pDevice);
}


void Image::CreateMesh(Craze::Graphics::IDevice *pDevice)
{
	//Create a quad that covers the entire screen.
	Craze::Graphics::GeometryData data;
	data.AddVertex(Craze::Graphics::Vertex(-1.0f, 1.0f, 0.0f, 0.0f, 0.0f));
	data.AddVertex(Craze::Graphics::Vertex(1.0f, 1.0f, 0.0f, 1.0f, 0.0f));
	data.AddVertex(Craze::Graphics::Vertex(1.0f, -1.0f, 0.0f, 1.0f, 1.0f));
	data.AddVertex(Craze::Graphics::Vertex(-1.0f, -1.0f, 0.0f, 0.0f, 1.0f));
	data.AddTriangleByIndicies(0, 1, 3);
	data.AddTriangleByIndicies(1, 2, 3);

	m_pMesh = pDevice->VCreateMesh(data);

}

std::string Image::VGetName()
{
	return m_Name;
}

Craze::Graphics::UI::ComponentParams* Image::VGetParams()
{
	return &m_Params;
}

void Image::VDraw(Craze::Graphics::Renderer::Renderer* pRenderer)
{	
	pRenderer->DrawObject(m_pMesh, &m_Material, Craze::Graphics::Renderer::RENDERQUE_SCREENSPACE, (float)(m_Params.depth * (3 - m_Params.level)));
}

bool Image::SetTexture(Craze::Graphics::TextureId texture)
{
	m_Texture = texture;
	m_Material.SetDecalTexture(m_Texture);
	return true;
}


void Image::VUpdate(float delta)
{
	
}

bool Image::VIsOver(float xpos, float ypos)
{
	if (xpos < m_Position.x - m_Size.x || ypos < m_Position.y - m_Size.y)
	{
		return false;
	}

	if (xpos > m_Position.x + m_Size.x || ypos > m_Position.y + m_Size.y)
	{
		return false;
	}

	return true;

}