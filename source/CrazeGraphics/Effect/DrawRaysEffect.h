#pragma once
#include "IEffect.h"

namespace Craze
{
	namespace Graphics2
	{
		class DrawRaysEffect : public IEffect
		{
		public:

			bool initialize() { IEffect::initialize(


		protected:

			virtual const D3D11_INPUT_ELEMENT_DESC* getLayout(int& count);
		};
	}
}