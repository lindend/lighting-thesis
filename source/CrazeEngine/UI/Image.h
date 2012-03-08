#pragma once
#ifndef CRAZE__UI__IMAGE__H__
#define CRAZE__UI__IMAGE__H__

#include "IComponent.h"

namespace Craze
{
	namespace Graphics
	{
		namespace UI
		{
			class Image : public IComponent
			{
			public:
				Image(IDevice* pDevice, std::string name = "");
				Image(IDevice* pDevice, float xpos, float ypos, std::string name = "");
				Image(IDevice* pDevice, float xpos, float ypos, float width, float height, std::string name = "");

				/**
				@return The name of the component.
				*/
				virtual std::string VGetName();

				/**
				@return The component parameters. The paramters should be editable.
				*/
				virtual ComponentParams* VGetParams();
	
				/**
				Sets the texture for the button.
				@param texture The texture to use.
				*/
				bool SetTexture(TextureId texture);

				virtual bool VHandleClick() { return false; }
			protected:

				/**
				Renders the component
				@param pRenderer The renderer that should receive the input data.
				*/
				virtual void VDraw(Renderer::Renderer* pRenderer);

				/**
				Updates the component, called once each frame.
				@param delta The time in milliseconds since the last frame.
				*/
				virtual void VUpdate(float delta);

				/**
				Checks if the position requested is over the current component.
				@param xpos The x-position to check.
				@param ypos The y-position to check.
				@return True if the position is over the current component.
				*/
				virtual bool VIsOver(float xpos, float ypos);

				void CreateMesh(IDevice* pDevice);

				ComponentParams m_Params;

				float m_Depth;

				std::string m_Name;

				IMesh<Vertex>* m_pMesh;
				TextureId m_Texture;
				Material m_Material;
			};
		}
	}
}



#endif /*CRAZE__UI__IMAGE__H__*/