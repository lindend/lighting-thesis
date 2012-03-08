#pragma once
#include "Texture.h"

namespace Craze
{
	namespace Graphics2
	{
		class TextureLock
		{
			friend class DynamicTexture;

		public:
			
			bool IsLocked() const
			{
				return m_MappedResource.pData != 0;
			}

			template<typename T> T* GetPixel(unsigned int x, unsigned int y);

			void Unlock();

		private:
			D3D11_MAPPED_SUBRESOURCE m_MappedResource;
			class DynamicTexture* m_pTexture;
			ID3D11Resource* m_pLockedResource;

			TextureLock(DynamicTexture* pTexture);
		};

		class DynamicTexture : public Texture
		{
		public:
			static std::shared_ptr<DynamicTexture> Create(Device* pDevice, unsigned int width, unsigned int height, TEXTURE_FORMAT format, unsigned int ringBufferSize = 3);

			TextureLock* Lock();
			void Unlock();

			~DynamicTexture();

		private:
			DynamicTexture(Device* pDevice);

			unsigned int m_NumBuffers;

			ID3D11Texture2D** m_ppBuffers;
			TextureLock m_Lock;
		};

		template <typename T> T* TextureLock::GetPixel(unsigned int x, unsigned int y)
		{
			if (!IsLocked())
			{
				return 0;
			}

			if (x >= m_pTexture->GetWidth() || y >= m_pTexture->GetHeight())
			{
				return 0;
			}
				
		#ifdef _WIN64
			Warning! Not 64-bit compatible...
		#endif
			return (T*)((unsigned int)m_MappedResource.pData + y * m_MappedResource.RowPitch + x * Texture::GetPixelSize(m_pTexture->GetFormat()));
		}
	}
}