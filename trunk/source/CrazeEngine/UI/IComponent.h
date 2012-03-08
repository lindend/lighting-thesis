#pragma once
#ifndef CRAZE__USER__INTERFACE__COMPONENT__INTERFACE__H__
#define CRAZE__USER__INTERFACE__COMPONENT__INTERFACE__H__

#include <string>
#include <set>
#include "Graphics/Renderer/Renderer.h"
#include "Math/Matrix4.h"
#include "Math/Vector2.h"

namespace Craze
{
	namespace Graphics
	{
		namespace UI
		{
			enum COMPONENT_LEVEL
			{
				CL_BACK,
				CL_MID,
				CL_FORE,
			};

			struct ComponentParams
			{
				bool clickable;
				bool dragable;
				bool visible;
				COMPONENT_LEVEL level;
				unsigned int depth;
			};

			typedef std::set<class IComponent*> ComponentSet;

			/**
			This class represents a component on the screen, for example a button.
			*/
			class IComponent
			{
			public:
				IComponent();
				/**
				@return The name of the component.
				*/
				virtual std::string VGetName() = 0;
				/**
				@return The component parameters. The paramters should be editable.
				*/
				virtual ComponentParams* VGetParams() = 0;

				/**
				Changes the size of the object, specified in units. 1 unit = 1/2 the screen.
				@param width The width of the object.
				@param height The height of the object.
				*/
				void SetSize(float width, float height);
				/**
				Sets the position of the component in units. 1 unit = 1/2 the screen.
				@param xpos The x position of the object.
				@param ypos The y position of the object.
				*/
				void SetPosition(float xpos, float ypos);

				/**
				Called by the UI manager when the mouse is clicked somwehere on the screen.
				This function will call VIsMouseOver for the current node, and if the mouse
				is over it will also call it for all the children to this node.
				@param xpos The x-position of the mouse.
				@param ypos The y-position of the mouse.
				@return Pointer to the component which was clicked.
				*/
				IComponent* OnClick(float xpos, float ypos);
				
				/**
				Function which returns true if the game should be allowed to continue processing
				of the key.
				*/
				virtual bool VOnKey(char key) { return true; }

				/**
				Adds a child to the component. The child will be updated by the component and
				will also be rendered.
				@param pChild The child to add.
				*/
				void AddChild(IComponent* pChild);

				/**
				Removes a child from the list of children. Also frees the memory of the child.
				@param pChild The child to remove.
				*/
				void RemoveChild(IComponent* pChild);

				/**
				Removes all children and frees up the memory.
				*/
				void RemoveAllChildren();

				/**
				Sets the parent of the component.
				@param pParent The new parent.
				*/
				void SetParent(IComponent* pParent);

				/**
				Renders all children of the component. This should be called in between the matrix push/pop
				calls to the renderer.
				@param pRenderer The renderer to draw to.
				*/
				void DrawChildren(Renderer::Renderer* pRenderer);

				/**
				Updates the current object and all children.
				@param delta The time delta to send to all children.
				*/
				void UpdateChildren(float delta);

				/**
				Removes the component.
				*/
				void Remove();

				virtual bool VHandleClick() = 0;
			protected:
				/**
				Renders the component, called by DrawChildren
				@param pRenderer The renderer that should receive the input data.
				*/
				virtual void VDraw(Renderer::Renderer* pRenderer) = 0;
				/**
				Updates the component, called once each frame by UpdateChildren.
				@param delta The time in milliseconds since the last frame.
				*/
				virtual void VUpdate(float delta) = 0;

				/**
				Checks if the position requested is over the current component.
				@param xpos The x-position to check.
				@param ypos The y-position to check.
				@return True if the position is over the current component.
				*/
				virtual bool VIsOver(float xpos, float ypos) = 0;

				Math::Matrix4 m_Transform;
				Math::Vector2 m_Size;
				Math::Vector2 m_Position;

				ComponentSet m_Children;
				IComponent* m_pParent;
				
			};
		}
	}
}



#endif /*CRAZE__USER__INTERFACE__COMPONENT__INTERFACE__H__*/