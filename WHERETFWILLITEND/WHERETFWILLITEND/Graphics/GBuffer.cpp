#include "GBuffer.h"

GBuffer::GBuffer(UINT width, UINT height, std::shared_ptr<Gdevice> device) {
	std::string name = "albedo";
	albedo_ = std::make_shared<RenderTarget>(width, height, name, device, TextureUsage::Albedo, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
	//OutputDebugStringA("Albedo" + '\n');
	name = "normalmap";
	normal_ = std::make_shared<RenderTarget>(width, height, name, device, TextureUsage::Normalmap, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
	//OutputDebugStringA("Normalmap" + '\n');
	name = "z buffer";
	depth_ = std::make_shared<Zbuffer>(width, height, name, device, TextureUsage::Depth);
	//OutputDebugStringA("Depth" + '\n');
};
