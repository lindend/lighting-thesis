#pragma once

#include "gwen/Controls/ImagePanel.h"
#include "gwen/Controls/Button.h"
#include "gwen/Controls/Label.h"
#include "gwen/Controls/GroupBox.h"
#include "gwen/Controls/WindowControl.h"

#include "luabind/luabind.hpp"

#include "UI/UISystem.h"

namespace Craze
{
	class Base
	{
	public:
		void SetParent(Base* pBase) 
		{ 
			GetBase()->SetParent(pBase->GetBase());
		}

		virtual Gwen::Controls::Base* GetBase() = 0;

		virtual ~Base() {}
	};

#define GWENBASE public: \
					virtual Gwen::Controls::Base* GetBase() { return this; }

	class Image : public Base, Gwen::Controls::ImagePanel
	{
		GWENBASE;
	public:
		Image(const std::string& imgFile, int x, int y, int w, int h) : Gwen::Controls::ImagePanel(Graphics2::ui_getCanvas()) 
		{
			SetImage(imgFile);
			SetBounds(x, y, w, h);
		}

		void SetColor(float r, float g, float b, float a) { SetDrawColor(Gwen::Color(r * 255.f, g * 255.f, b * 255.f, a * 255.f)); }
	};
	
	class Button : public Base, Gwen::Controls::Button
	{
		GWENBASE;
	public:
		Button(const std::string& text, int x, int y, int w, int h, const luabind::object& click) : Gwen::Controls::Button(Graphics2::ui_getCanvas())
		{
			m_CallBack = click;
			SetText(text);
			SetBounds(x, y, w, h);
			onPress.Add(this, &Button::OnClick);
		}

	private:
		void OnClick(Gwen::Controls::Base* pButton)
		{
			luabind::call_function<void>(m_CallBack, this);
		}

		luabind::object m_CallBack;
	};

	class ImageButton : public Base, Gwen::Controls::Button
	{
		GWENBASE;
	public:
		ImageButton(const std::string& imgName, int x, int y, int w, int h, const luabind::object& click) : Gwen::Controls::Button(Graphics2::ui_getCanvas())
		{
			m_CallBack = click;
			SetBounds(x, y, w, h);
			onPress.Add(this, &ImageButton::OnClick);
			SetImage(imgName, true);
		}
	private:
		void OnClick(Gwen::Controls::Base* pButton)
		{
			luabind::call_function<void>(m_CallBack, this);
		}

		luabind::object m_CallBack;
	};

	class Label : public Base, Gwen::Controls::Label
	{
		GWENBASE;
	public:
		Label(const std::string& text, int x, int y, int w, int h) : Gwen::Controls::Label(Graphics2::ui_getCanvas())
		{
			SetBounds(x, y, w, h);
			SetText(text);
			BringToFront();
			//this->SetTextColor(Gwen::Color(0, 0, 0, 255));
		}
		void setText(const std::string& text)
		{
			SetText(text);
		}
	};

	class Window : public Base, Gwen::Controls::WindowControl
	{
		GWENBASE;
	public:
		Window(const std::string& name, int x, int y, int w, int h) : Gwen::Controls::WindowControl(Graphics2::ui_getCanvas())
		{
			SetBounds(x, y, w, h);
			SetTitle(StrToW(name));
			BringToFront();
			SetDeleteOnClose(false);
			SetClosable(false);
		}
	};

#undef GWENBASE
}