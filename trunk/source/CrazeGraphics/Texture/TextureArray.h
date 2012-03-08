#pragma once
#include "Texture.h"

namespace Craze
{
	namespace Graphics2
	{
		class TextureArray : public Texture
		{
		public:

			static std::shared_ptr<TextureArray> Create(Device* pDevice, unsigned int width, unsigned int height, unsigned int arraySize, unsigned int mipLevels, TEXTURE_FORMAT format = TEXTURE_FORMAT_COLOR);

			bool LoadTextureIntoSlice(unsigned int slice, std::shared_ptr<Texture> pTexture);

		private:
			TextureArray() {}
			ID3D11Texture2D* CreateMatchingTexture();

			unsigned int m_ArraySize;
		};
	}
}