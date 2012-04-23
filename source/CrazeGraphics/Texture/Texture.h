#pragma once

#include <string>

#include "d3d11.h"
#include "d3dx11.h"

#include "Resource/ResourceEventHandler.h"
#include "Resource/Resource.h"

namespace Craze
{
	namespace Graphics2
	{
		enum TEXTURE_FORMAT
		{
			TEXTURE_FORMAT_COLOR = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
			TEXTURE_FORMAT_COLOR_LINEAR = DXGI_FORMAT_R8G8B8A8_UNORM,
			TEXTURE_FORMAT_FLOAT = DXGI_FORMAT_R32_FLOAT,
			TEXTURE_FORMAT_VECTOR2 = DXGI_FORMAT_R32G32_FLOAT,
			TEXTURE_FORMAT_VECTOR3 = DXGI_FORMAT_R32G32B32_FLOAT,
			TEXTURE_FORMAT_VECTOR4 = DXGI_FORMAT_R32G32B32A32_FLOAT,
			TEXTURE_FORMAT_HALF = DXGI_FORMAT_R16_FLOAT,
			TEXTURE_FORMAT_HALFVECTOR2 = DXGI_FORMAT_R16G16_FLOAT,
			TEXTURE_FORMAT_HALFVECTOR4 = DXGI_FORMAT_R16G16B16A16_FLOAT,
			TEXTURE_FORMAT_8BIT_UNORM = DXGI_FORMAT_R8_UNORM,
			TEXTURE_FORMAT_32BIT_UINT = DXGI_FORMAT_R32_UINT,
			TEXTURE_FORMAT_COMPRESSED = DXGI_FORMAT_BC7_UNORM_SRGB,
			TEXTURE_FORMAT_COMPRESSED_LINEAR = DXGI_FORMAT_BC7_UNORM,
		};

		class Device;

		class Texture
		{
			friend Texture* CreateTextureFromDevIL(Device* pDevice, bool gammaCorrected, const char* pDebugName);
		public:
			Texture();
			~Texture();
			static Texture* CreateFromFile(Device* pDevice, std::string fileName, bool gammaCorrected = true);
			static Texture* CreateFromMemory(Device* pDevice, void* pData, unsigned long dataLength, bool gammaCorrected = true, const char* pDebugName = nullptr);
			static Texture* CreateFromData(Device* pDevice, unsigned int width, unsigned int height, TEXTURE_FORMAT format, void* pData, const char* pDebugName);
			static Texture* CreateDDSFromMemory(Device* device, void* data, unsigned long len, const char* debugName);

			ID3D11ShaderResourceView* GetResourceView();
			ID3D11Resource* GetResource();

			TEXTURE_FORMAT GetFormat() const { return (TEXTURE_FORMAT)m_Format; }

			unsigned int GetHeight() const { return m_Height; }
			unsigned int GetWidth() const { return m_Width; }
			unsigned int GetNumMips() const { return m_NumMips; }

			void SetID(unsigned short id) { m_ID = id; }
			unsigned short GetID() const { return m_ID; }

			static unsigned int GetPixelSize(TEXTURE_FORMAT fmt);

		protected:
			D3D11_SRV_DIMENSION m_Dimension;

			Device* m_pDevice;

			ID3D11Resource* m_pTexture;
			ID3D11ShaderResourceView* m_pResourceView;

			unsigned int m_Width;
			unsigned int m_Height;
			unsigned int m_NumMips;

			DXGI_FORMAT m_Format;
			

			unsigned short m_ID;
		};

		class TextureResource : public Resource
		{
			CRAZE_ALLOC();
		public:
			TextureResource() : m_texture(nullptr) {}
            Texture* m_texture;
        protected:
  
            virtual void onDestroy() 
			{
				delete m_texture;
				m_texture = nullptr;
			}
		};

		class TexturePtr
		{
		public:
			TexturePtr() : m_tex(nullptr), m_texRes(nullptr) {}
			TexturePtr(Texture* tex) : m_tex(tex), m_texRes(nullptr) {}
			TexturePtr(std::shared_ptr<const TextureResource> texRes) : m_tex(nullptr), m_texRes(texRes) {}

			TexturePtr& operator=(Texture* tex)
			{
				m_tex = tex;
				m_texRes = nullptr;
				return *this;
			}

			TexturePtr& operator=(std::shared_ptr<TextureResource> texRes)
			{
				m_tex = nullptr;
				m_texRes = texRes;
				return *this;
			}

			Texture* operator->() const
			{
				return get();
			}

			operator bool() const
			{
				return (m_tex || m_texRes);
			}

			Texture* get() const
			{
				if (m_tex)
				{
					return m_tex;
				} else if(m_texRes)
				{
					return m_texRes->m_texture;
				}
				return nullptr;
			}

		private:
			Texture* m_tex;
			std::shared_ptr<const TextureResource> m_texRes;
		};

		class TextureResourceHandler : public ResourceEventHandler
		{
		public:
			TextureResourceHandler() { m_readCompleteMT = true; m_allCompleteMT = true;}

			virtual bool preRead(std::shared_ptr<Resource> res) { return true; }
			virtual bool readComplete(ResourceLoadData* loadData);
			virtual bool allComplete(ResourceLoadData* loadData) { return true; }

			virtual bool fileReadError(ResourceLoadData* loadData);

			virtual std::shared_ptr<Resource> createResource(u32, u64) { return std::shared_ptr<Resource>(CrNew TextureResource()); }
		};

        class DDSTextureResourceHandler : public ResourceEventHandler
        {
        public:
			DDSTextureResourceHandler() { m_readCompleteMT = true; m_allCompleteMT = true;}

			virtual bool preRead(std::shared_ptr<Resource> res) { return true; }
			virtual bool readComplete(ResourceLoadData* loadData);
			virtual bool allComplete(ResourceLoadData* loadData) { return true; }

            virtual bool fileReadError(ResourceLoadData* loadData) { return false; }

			virtual std::shared_ptr<Resource> createResource(u32, u64) { return std::shared_ptr<Resource>(CrNew TextureResource()); }
        };
	}
}