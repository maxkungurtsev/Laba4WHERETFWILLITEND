#include "GBuffer.h"

GBuffer::GBuffer(UINT width, UINT height, std::shared_ptr<Gdevice> device) {
	std::string name = "albedo";
	albedo_ = std::make_shared<RenderTarget>(width, height, name, device, TextureUsage::Albedo);
	name = "normalmap";
	normal_ = std::make_shared<RenderTarget>(width, height, name, device, TextureUsage::Normalmap);
	name = "z buffer";
	depth_ = std::make_shared<Zbuffer>(width, height, name, device, TextureUsage::Depth);
	name = "material_index";
	material_index_ = std::make_shared<RenderTarget>(width, height, name, device, TextureUsage::Albedo);
};
