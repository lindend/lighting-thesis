#include <windows.h>
#include <crtdbg.h>

#include "Battlecraze/BattlecrazeApplication.h"

#include <memory>

#ifndef _DEBUG
int WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
int main()
#endif
{

	int flag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG | _CRTDBG_DELAY_FREE_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF);

	_CrtCheckMemory();
	
	{
		//Create a BATTLECRAZE application instance
		BattlecrazeApplication app;
		if (app.VInitialize())
			app.VMainLoop();
		app.VDestroy();

	}

	_CrtCheckMemory();
	_CrtDumpMemoryLeaks();
	

	return 0;

	/*Craze::EventLogger evtLoggr;
	evtLoggr.Initialize(L"craze.log", Craze::LOGTYPE_INFORMATION);

	Craze::Win32Timer tmr;
	tmr.Start();

	strstream s;

	Craze::Resource::ResourceLoader ldr(130 * Craze::Resource::MEGABYTE);

	ldr.AddResourceLocationFolder("");

	Craze::Resource::Resource* pRes = ldr.LoadFile("bigFile1.txt");

	ldr.Process(7 * Craze::Resource::MEGABYTE);

	s << tmr.Snap() << "\n";

	ldr.LoadFile("bigFile2.txt");

	ldr.Process(14 * Craze::Resource::MEGABYTE);

	cout << tmr.Snap() << "\n";

	ldr.LoadFile("bigFile3.bmp");

	ldr.Process(170 * Craze::Resource::MEGABYTE);

	s << tmr.Snap() << "\n";

	ldr.LoadFile("bigFile4.txt");

	pRes->GetData();

	ldr.Process(7 * Craze::Resource::MEGABYTE);

	s << tmr.Snap() << "\n";

	ldr.LoadFile("bigFile3.bmp");

	ldr.Process(70 * Craze::Resource::MEGABYTE);

	ldr.Process(Craze::Resource::MEGABYTE);

	ldr.Flush();

	LOG_INFO(s.str());

	ldr.LoadFile("bigFile2.txt");

	ldr.Clear();

	return 0;*/
}