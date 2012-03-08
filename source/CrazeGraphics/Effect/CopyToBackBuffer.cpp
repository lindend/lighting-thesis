#include "CrazeGraphicsPCH.h"
#include "CopyToBackBuffer.h"

#include "Device.h"

using namespace Craze;
using namespace Craze::Graphics2;

void CopyToBackBuffer::doCopy(std::shared_ptr<Texture> source)
{
	gpDevice->SetRenderTarget(nullptr, nullptr);

	ID3D11ShaderResourceView* srv[1];
	srv[0] = source->GetResourceView();

	gpDevice->GetDeviceContext()->PSSetShaderResources(0, 1, srv);

	IEffect::set();

	gpDevice->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gpDevice->GetDeviceContext()->Draw(3, 0);

	srv[0] = nullptr;
	gpDevice->GetDeviceContext()->PSSetShaderResources(0, 1, srv);
}