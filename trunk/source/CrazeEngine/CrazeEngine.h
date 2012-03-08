#pragma once

#include <string>
#include <list>
#include <vector>
#include <map>
#include <set>
#include <utility>
#include <functional>
#include <memory>
#include <strstream>
#include <algorithm>

#include <assert.h>

#define WIN32_LEAN_AND_MEAN

#include "windows.h"

#include "boost/filesystem.hpp"
#include "boost/pool/object_pool.hpp"

#include "CrazeMath.h"

#include "EventLogger.h"
#include "Profiling/Profiler.h"

#include "Util/Type/StringHashType.h"
#include "Util/optional.hpp"

#include "Memory/MemoryManager.h"

#include "StrUtil.hpp"
