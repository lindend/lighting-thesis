#include "CrazeGraphicsPCH.h"	

#include "Device.h"
#include "Graphics.h"
#include "Buffer/Buffer.h"

#include "DrawRays.h"

#include "EffectUtil/EffectHelper.h"
#include "EffectUtil/CBufferHelper.hpp"

using namespace Craze;
using namespace Craze::Graphics2;

bool DrawRays::initialize()
{
	unsigned int args[] = { 0, 1, 0, 0};
	m_argBuffer = Buffer::CreateArg(gpDevice, sizeof(unsigned int) * 4, args, "DrawRays arg buffer");

	m_effect.initialize();
	return true;
}

void DrawRays::render(std::shared_ptr<UAVBuffer> rays, const Matrix4& viewProj)
{
	m_effect.set();
	//draw instanced indirect in immediate context (gpdevice->getDeviceContext
	CBPerObject cbObj;
	cbObj.world = Matrix4::IDENTITY * viewProj;
	gpDevice->GetCbuffers()->SetObject(cbObj);

	auto dc = gpDevice->GetDeviceContext();

	//Prepare the argument buffer for the indirect call
	dc->CopyStructureCount(m_argBuffer->GetBuffer(), 0, rays->GetUAV());

	ID3D11Buffer* vs = nullptr;
	unsigned int stride = 0;
	unsigned int offset = 0;
	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	dc->IASetVertexBuffers(0, 1, &vs, &stride, &offset);
	dc->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, 0);
	dc->IASetInputLayout(nullptr);
	
	ID3D11ShaderResourceView* srv = rays->GetSRV();
	gpDevice->GetDeviceContext()->VSSetShaderResources(0, 1, &srv);

	//Draw the rays
	//dc->Draw(16384, 0);
	dc->DrawInstancedIndirect(m_argBuffer->GetBuffer(), 0);

	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//dc->RSSetState(nullptr);
	dc->OMSetDepthStencilState(nullptr, 0);

	srv = nullptr;
	gpDevice->GetDeviceContext()->VSSetShaderResources(0, 1, &srv);

	m_effect.reset();
}


