#pragma once

#include "Resource/Resource.h"

#include "d3d11.h"

namespace Craze
{
	class Resource;
    namespace Graphics2
    {
        template <class T>
		class ShaderResource : public Resource
		{
        public:
			ShaderResource() {}

            typedef T ShaderType;

            T m_shader;

		protected:
			virtual void onDestroy()
			{
				if (m_shader)
				{
					m_shader->Release();
					m_shader = nullptr;
				}
			}

		private:
			ShaderResource(const ShaderResource&);
			ShaderResource& operator=(const ShaderResource&);
		};

		class VertexShaderResource : public ShaderResource<ID3D11VertexShader*>
		{
			CRAZE_ALLOC();
        public:
            ID3D10Blob* m_shaderByteCode;
		};

		class GeometryShaderResource : public ShaderResource<ID3D11GeometryShader*>
		{
			CRAZE_ALLOC();
		};

		class PixelShaderResource : public ShaderResource<ID3D11PixelShader*>
		{
			CRAZE_ALLOC();
		};

		class ComputeShaderResource : public ShaderResource<ID3D11ComputeShader*>
		{
			CRAZE_ALLOC();
		};
    }
}
