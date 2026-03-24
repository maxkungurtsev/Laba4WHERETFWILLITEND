#include "GBuffer.h"

GBuffer::GBuffer(UINT width, UINT height, std::shared_ptr<Gdevice> device) {
	albedo_ = std::make_shared<RenderTarget>(width, height, "albedo", device, TextureUsage::Albedo);
	normal_ = std::make_shared<RenderTarget>(width, height, "normalmap", device, TextureUsage::Normalmap);
	depth_ = std::make_shared<Zbuffer>(width, height, "z buffer", device, TextureUsage::Depth);
};
