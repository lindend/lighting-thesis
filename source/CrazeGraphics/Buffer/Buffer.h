#pragma once

#include "d3d11.h"
#include "Memory/MemoryManager.h"

#include <memory>

namespace Craze
{
	namespace Graphics2
	{
		class Device;

		class Buffer
		{
			CRAZE_ALLOC();
		public:
			~Buffer();

			ID3D11Buffer* GetBuffer() const { return m_pBuffer; }
			operator ID3D11Buffer *() const { return m_pBuffer; }

			static std::shared_ptr<Buffer> CreateArg(Device *pDevice, int size, const void *pData, const char *pDebugName);

		protected:
			Buffer(Device *pDevice, ID3D11Buffer *pBuf) : m_pDevice(pDevice), m_pBuffer(pBuf) {}
			Device *m_pDevice;
		private:
			ID3D11Buffer *m_pBuffer;
		
			Buffer(const Buffer&);
			Buffer& operator=(const Buffer&);
		};

		//A buffer meant for vertices and indices
		class GeometryBuffer : public Buffer
		{
			CRAZE_ALLOC();
		public:
			enum TYPE
			{
				VERTEX,
				INDEX,
			};
			static std::shared_ptr<GeometryBuffer> Create(Device *pDevice, TYPE t, const void *pData, unsigned int stride, unsigned int numElems, bool dynamic, const char *pDebugName);

			unsigned int GetStride() const { return m_Stride; }
			//Returns the number of elements in the buffer.
			//For dynamic buffers, this equals the number of elements written in the last call to WriteData
			unsigned int GetNumElems() const { return m_LastElem - m_FirstElem; }

			//These three functions are only interesting for dynamic buffers (although GetFirstIndex and GetMaxElems can be called, their result is hardly interesting)
			bool WriteData(const void *pData, int numElems); 
			int GetFirstIndex() const { return m_FirstElem; }
			int GetMaxElems() const { return m_NumElems; }

			bool IsDynamic() const { return m_IsDynamic; }
			TYPE GetType() const { return m_Type; }

			~GeometryBuffer() {}
		private:
			GeometryBuffer(Device *pDevice, ID3D11Buffer *pBuf) : Buffer(pDevice, pBuf) {}
			int m_Stride;
			int m_NumElems;

			//Used for dynamic buffers
			int m_FirstElem;
			int m_LastElem;
			bool m_IsDynamic;
			TYPE m_Type;
		};

		class SRVBuffer : public Buffer
		{
			CRAZE_ALLOC();
		public:
			ID3D11ShaderResourceView *GetSRV() const { return m_pSRV; }
			operator ID3D11ShaderResourceView *() const { return m_pSRV; }

			static std::shared_ptr<SRVBuffer> CreateStructured(Device *pDevice, int elemSize, int numElems, const void *pData, bool dynamic, const char *pDebugName);
			static std::shared_ptr<SRVBuffer> CreateRaw(Device *pDevice, DXGI_FORMAT format, int size, const void *pData, const char *pDebugName);

			~SRVBuffer()
			{
				m_pSRV->Release();
				m_pSRV = 0;
			}
		protected:
			SRVBuffer(Device *pDevice, ID3D11Buffer *pBuf, ID3D11ShaderResourceView *pSRV) : Buffer(pDevice, pBuf), m_pSRV(pSRV) {}
		private:
			ID3D11ShaderResourceView *m_pSRV;
			bool m_IsDynamic;
		};

		class UAVBuffer : public SRVBuffer
		{
			CRAZE_ALLOC();
		public:
			ID3D11UnorderedAccessView *GetUAV() const { return m_pUAV; }
			operator ID3D11UnorderedAccessView *() const { return m_pUAV; }

			static std::shared_ptr<UAVBuffer> Create(Device *pDevice, int elemSize, int numElems, bool appendConsume, const char *pDebugName);
			~UAVBuffer()
			{
				m_pUAV->Release();
				m_pUAV = 0;
			}
		private:
			UAVBuffer(Device *pDevice, ID3D11Buffer *pBuf, ID3D11ShaderResourceView *pSRV, ID3D11UnorderedAccessView *pUAV) : SRVBuffer(pDevice, pBuf, pSRV), m_pUAV(pUAV) {}
			ID3D11UnorderedAccessView *m_pUAV;
		};
	}
}