#pragma once

#include "Resource/ResourceManager.h"

#include "shaderresource.h"

namespace Craze
{
    namespace Graphics2
    {
		class ShaderResourceHandler : public ResourceEventHandler
		{
        public:
            ShaderResourceHandler() 
			{
				m_readCompleteMT = true;
				m_allCompleteMT = true;
			}

            virtual bool preRead(Resource* res)
            {
                return true;
            }
            virtual bool allComplete(ResourceLoadData* data) { return true; }
            virtual bool fileReadError(ResourceLoadData* data) { return false; }
		};

		class VertexShaderResourceHandler : public ShaderResourceHandler
		{
        public:
            virtual bool readComplete(ResourceLoadData* data);
			virtual Resource* createResource(u32, u64) { return CrNew VertexShaderResource(); }
		};

		class PixelShaderResourceHandler : public ShaderResourceHandler
		{
        public:
            virtual bool readComplete(ResourceLoadData* data);
			virtual Resource* createResource(u32, u64) { return CrNew PixelShaderResource(); }
		};

		class GeometryShaderResourceHandler : public ShaderResourceHandler
		{
        public:
            virtual bool readComplete(ResourceLoadData* data);
			virtual Resource* createResource(u32, u64) { return CrNew GeometryShaderResource(); }
		};

		class ComputeShaderResourceHandler : public ShaderResourceHandler
		{
        public:
            virtual bool readComplete(ResourceLoadData* pData);
			virtual Resource* createResource(u32, u64) { return CrNew ComputeShaderResource(); }
		};
    }
}
