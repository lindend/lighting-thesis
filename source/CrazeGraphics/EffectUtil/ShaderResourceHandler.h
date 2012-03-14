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

            virtual bool preRead(std::shared_ptr<Resource> res)
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
			virtual std::shared_ptr<Resource> createResource(u32, u64) { return std::shared_ptr<Resource>(CrNew VertexShaderResource()); }
		};

		class PixelShaderResourceHandler : public ShaderResourceHandler
		{
        public:
            virtual bool readComplete(ResourceLoadData* data);
			virtual std::shared_ptr<Resource> createResource(u32, u64) { return std::shared_ptr<Resource>(CrNew PixelShaderResource()); }
		};

		class GeometryShaderResourceHandler : public ShaderResourceHandler
		{
        public:
            virtual bool readComplete(ResourceLoadData* data);
			virtual std::shared_ptr<Resource> createResource(u32, u64) { return std::shared_ptr<Resource>(CrNew GeometryShaderResource()); }
		};

		class ComputeShaderResourceHandler : public ShaderResourceHandler
		{
        public:
            virtual bool readComplete(ResourceLoadData* pData);
			virtual std::shared_ptr<Resource> createResource(u32, u64) { return std::shared_ptr<Resource>(CrNew ComputeShaderResource()); }
		};

		class TessShaderResourceHandler : public ShaderResourceHandler
		{
			virtual bool readComplete(ResourceLoadData* data);
			virtual std::shared_ptr<Resource> createResource(u32, u64) { return std::shared_ptr<Resource>(CrNew TessShaderResource()); }
		};

		class HullShaderResourceHandler : public ShaderResourceHandler
		{
		public:
			virtual bool readComplete(ResourceLoadData* data);
			virtual std::shared_ptr<Resource> createResource(u32, u64) { return std::shared_ptr<Resource>(CrNew HullShaderResource()); }
		};

		class DomainShaderResourceHandler : public ShaderResourceHandler
		{
		public:
			virtual bool readComplete(ResourceLoadData* data);
			virtual std::shared_ptr<Resource> createResource(u32, u64) { return std::shared_ptr<Resource>(CrNew DomainShaderResource()); }
		};
    }
}
