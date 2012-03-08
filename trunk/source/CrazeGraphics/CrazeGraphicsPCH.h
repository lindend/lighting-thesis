#pragma once

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x) if(x) { (x)->Release(); (x) = NULL; }
#endif

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>
#include <algorithm>

#include <stdio.h>
#include <assert.h>

#define VC_EXTRALEAN

#include "windows.h"

#include "d3d11.h"
#include "d3dx11.h"

//#include "EventLogger.h"

#include "CrazeMath.h"
#include "Memory/MemoryManager.h"

#include "Profiling/Profiler.h"
#include "EventLogger.h"

#include "StrUtil.hpp"

void SetDebugName(ID3D11DeviceChild* pItem, const char* pName);