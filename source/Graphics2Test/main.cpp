#include "windows.h"
#include "crtdbg.h"

#include "TestApp.hpp"

#include <memory>

#include "CrazeMath.h"

using namespace Craze;

#include "../CrazeGraphics/kdTree/kdTree.h"

using namespace Craze::Graphics2;

#ifndef _DEBUG
int WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
int main(int argc, const char* argv[])
#endif
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	TestApp app;
	if (app.Initialize(argc, argv))
	{
		app.VMainLoop();

	}

	app.VDestroy();

	return 0;
}