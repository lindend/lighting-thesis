#include "SystemInfo.h"

#define VC_EXTRALEAN
#include "windows.h"

using namespace Craze;

unsigned int gNumProcessors;
std::string gCurrentDir;

bool Craze::SystemInfo::Initialize()
{
	SYSTEM_INFO inf;
	GetSystemInfo(&inf);
	gNumProcessors = inf.dwNumberOfProcessors;

    {
        const int bufSize = 1024;
        char buf[bufSize];
        GetCurrentDirectoryA(bufSize, buf);
        gCurrentDir = buf;
    }

	return true;
}

const std::string& Craze::SystemInfo::GetCurrentDir()
{
    return gCurrentDir;
}

unsigned int Craze::SystemInfo::GetNumProcessors()
{
	return gNumProcessors;
}
