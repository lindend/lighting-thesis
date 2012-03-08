#pragma once

#include "CrazeEngine/Application/Application.h"
//#include "TestGame.hpp"

class TestApp : public Craze::Application
{
public:
	virtual std::string VGetGameName() { return "CrazeGraphics 2 Test Application"; }
	bool Initialize(int argc, const char* argv[])
	{
		bool result = Application::Initialize(argc, argv);
		ShowCursor(true);
		return result;
	}
};