#pragma once

namespace Craze
{
	namespace Graphics2
	{
		class PIXHelper
		{
		public:
			PIXHelper(const wchar_t* fName);
			~PIXHelper();
			static void begin(const wchar_t* fName);
			static void end();
		};

#ifdef _DEBUG
		#define PIXMARKER(fName) Craze::Graphics2::PIXHelper local_pix_helper_ = Craze::Graphics2::PIXHelper(fName);
		#define PIXBEGIN(fName) Craze::Graphics2::PIXHelper::begin(fName)
		#define PIXEND() Craze::Graphics2::PIXHelper::end()
#else
		#define PIXMARKER(fName) 
		#define PIXBEGIN(fName) 
		#define PIXEND() 
#endif
	}
}