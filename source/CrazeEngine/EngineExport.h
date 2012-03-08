#pragma once

#ifdef CRAZEENGINE_BUILD_DLL
	#ifdef CRAZEENGINE_DLL
		#define CRAZEENGINE_EXP __declspec(dllexport)
	#else
		#define CRAZEENGINE_EXP __declspec(dllimport)
	#endif
#else
	#define CRAZEENGINE_EXP 
#endif