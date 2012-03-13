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
		};

		#define PIXMARKER(fName) Craze::Graphics2::PIXHelper local_pix_helper_ = Craze::Graphics2::PIXHelper(fName);
	}
}