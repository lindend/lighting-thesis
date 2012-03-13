#include "CrazeGraphicsPCH.h"
#include "PIXHelper.h"

#include "d3d9.h"

using namespace Craze;
using namespace Craze::Graphics2;

PIXHelper::PIXHelper(const wchar_t* fName)
{
	D3DPERF_BeginEvent(0x00000000FF, fName);
}
PIXHelper::~PIXHelper()
{
	D3DPERF_EndEvent();
}