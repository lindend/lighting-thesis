#include "Container.h"

using Craze::Graphics::UI::Container;
using Craze::Graphics::UI::ComponentParams;

Container::Container(std::string name)
{
	m_Params.clickable = false;
	m_Params.visible = true;

	SetSize(1.0f, 1.0f);
	SetPosition(0.0f, 0.0f);

	m_Name = name;


}

std::string Container::VGetName()
{
	return m_Name;
}


ComponentParams* Container::VGetParams()
{
	return &m_Params;
}


void Container::VDraw(Craze::Graphics::Renderer::Renderer* pRenderer)
{

}


void Container::VUpdate(float delta)
{

}