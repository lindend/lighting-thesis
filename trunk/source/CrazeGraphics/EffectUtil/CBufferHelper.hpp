#pragma once

#include "../Device.h"

namespace Craze
{
	namespace Graphics2
	{
		class CBufferItemY
		{
			friend class CBufferItem;
		public:
			void operator=(const Vec3& v)
			{
				*(Vec3*)m_pData = v;
			}

			void operator=(const Vector2& v)
			{
				*(Vector2*)m_pData = v;
			}

			void operator=(float v)
			{
				*m_pData = v;
			}

			void operator=(unsigned int v)
			{
				*(unsigned int*)m_pData = v;
			}

			void operator=(int v)
			{
				*(int*)m_pData = v;
			}

		private:
			CBufferItemY(float* pData)
			{
				m_pData = pData;
			}

			float* m_pData;
		};

				
		class CBufferItemZ
		{
			friend class CBufferItem;
		public:

			void operator=(const Vector2& v)
			{
				*(Vector2*)m_pData = v;
			}

			void operator=(float v)
			{
				*m_pData = v;
			}

			void operator=(unsigned int v)
			{
				*(unsigned int*)m_pData = v;
			}

			void operator=(int v)
			{
				*(int*)m_pData = v;
			}

		private:
			CBufferItemZ(float* pData)
			{
				m_pData = pData;
			}

			float* m_pData;
		};
						
		class CBufferItemW
		{
			friend class CBufferItem;
		public:

			void operator=(float v)
			{
				*m_pData = v;
			}

			void operator=(unsigned int v)
			{
				*(unsigned int*)m_pData = v;
			}

			void operator=(int v)
			{
				*(int*)m_pData = v;
			}

		private:
			CBufferItemW(float* pData)
			{
				m_pData = pData;
			}

			float* m_pData;
		};

		class CBufferItem
		{
			friend class CBufferHelper;
		public:
			void operator=(const Matrix4& m)
			{
				*(Matrix4*)m_pData = m;
			}

			void operator=(const Matrix3& m)
			{
				float* pData = (float*)m_pData;
				
				pData[0]  = m.m_Mat[0][0];
				pData[1]  = m.m_Mat[0][1];
				pData[2]  = m.m_Mat[0][2];
				pData[4]  = m.m_Mat[1][0];
				pData[5]  = m.m_Mat[1][1];
				pData[6]  = m.m_Mat[1][2];
				pData[8]  = m.m_Mat[2][0];
				pData[9]  = m.m_Mat[2][1];
				pData[10] = m.m_Mat[2][2];
			}

			void operator=(const Vector4& v)
			{
				*m_pData = v;
			}

			void operator=(const Vec3& v)
			{
				*(Vec3*)m_pData = v;
			}

			void operator=(const Vector2& v)
			{
				*(Vector2*)m_pData = v;
			}

			void operator=(float v)
			{
				*(float*)m_pData = v;
			}

			void operator=(unsigned int v)
			{
				*(unsigned int*)m_pData = v;
			}

			void operator=(int v)
			{
				*(int*)m_pData = v;
			}

			CBufferItemY Y()
			{
				return CBufferItemY((float*)m_pData + 1);
			}

			CBufferItemZ Z()
			{
				return CBufferItemZ((float*)m_pData + 2);
			}

			CBufferItemW W()
			{
				return CBufferItemW((float*)m_pData + 3);
			}
		private:
			CBufferItem(Vector4* pData)
			{
				m_pData = pData;
			}

			Vector4* m_pData;
		};

		class CBufferHelper
		{
		public:
			CBufferHelper()
			{
				m_pData = 0;
				m_pDevice = 0;
				m_pBuffer = 0;
			}

			CBufferHelper(Device* pDevice, ID3D11Buffer* pBuffer)
			{
				Map(pDevice, pBuffer);
			}

			~CBufferHelper()
			{
				Unmap();
			}

			bool Map(Device* pDevice, ID3D11Buffer* pBuffer)
			{
				m_pDevice = pDevice;

				D3D11_MAPPED_SUBRESOURCE res;
				if (FAILED(pDevice->GetDeviceContext()->Map(pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &res)))
				{
					LOG_ERROR("Unable to map constant buffer");
					return false;
				}

				m_pData = (Vector4*)res.pData;

				m_pBuffer = pBuffer;

				return true;

			}

			void Unmap()
			{
				if (m_pBuffer)
				{
					m_pDevice->GetDeviceContext()->Unmap(m_pBuffer, 0);
					m_pBuffer = 0;
					m_pData = 0;
				}
			}

			void Set(const Matrix4& m, unsigned int index)
			{
				*(Matrix4*)&m_pData[index] = m;
			}

			void Set(const Vector4& v, unsigned int index)
			{
				m_pData[index] = v;
			}

			void Set(const Vec3& v, unsigned int index, unsigned int offset = 0)
			{
				*(Vec3*)((float*)&m_pData[index] + offset) = v;
			}

			void Set(const Vector2& v, unsigned int index, unsigned int offset = 0)
			{
				*(Vector2*)((float*)&m_pData[index] + offset) = v;
			}

			void Set(float v, unsigned int index, unsigned int offset = 0)
			{
				*((float*)&m_pData[index] + offset) = v;
			}
			
			CBufferItem operator[](unsigned int index)
			{
				return CBufferItem(&m_pData[index]);
			}

			template <class T> T* GetPtr()
			{
				return (T*)m_pData;
			}

		private:
			Vector4* m_pData;
			Device* m_pDevice;
			ID3D11Buffer* m_pBuffer;

			CBufferHelper(const CBufferHelper&);
			CBufferHelper& operator=(const CBufferHelper&);
		};
	}
}