#pragma once

#ifdef CRAZEMATH_BUILD_DLL
#ifdef CRAZEMATH_DLL
#define CRAZEMATH_EXP __declspec(dllexport)
#else
#define CRAZEMATH_EXP __declspec(dllimport)
#endif
#else
#define CRAZEMATH_EXP 
#endif
