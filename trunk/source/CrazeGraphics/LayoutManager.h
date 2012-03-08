#pragma once
#include "d3d11.h"

namespace Craze
{
	namespace Graphics2
	{
		class IEffect;
		extern class Device* gpDevice;

		class LayoutManager
		{
		public:
			void Initialize();
			void Destroy();

			LayoutManager() : m_pInputElemDesc(nullptr)
			{
				
			}

			ID3D11InputLayout* GetLayout(IEffect* pEffect);

			~LayoutManager();

		private:
			typedef std::map<IEffect*, ID3D11InputLayout*> LayoutMap;
			LayoutMap m_Layouts;
			int m_InputElemCount;

			const D3D11_INPUT_ELEMENT_DESC* m_pInputElemDesc;

			LayoutManager(const LayoutManager&);
			LayoutManager& operator=(const LayoutManager&);
		};
	}
}
/*

#include "Device.h"

template<class T> ID3D11InputLayout* Craze::Graphics2::LayoutManager::GetLayout()
{
	size_t type = T::GetType();

	LayoutMap::iterator it = m_Layouts.find(type);

	if (it != m_Layouts.end())
	{
		return it->second;
	}

	int count;
	const D3D11_INPUT_ELEMENT_DESC* layoutDesc = T::GetLayoutDesc(count);

	ID3D11InputLayout* pLayout = 0;
	if (FAILED(gpDevice->GetDevice()->CreateInputLayout(layoutDesc, count, m_pVertexShaderCode->GetBufferPointer(), 
		m_pVertexShaderCode->GetBufferSize(), &pLayout)))
	{
		return 0;
	}

	m_Layouts[type] = pLayout;

	return pLayout;
}
*/