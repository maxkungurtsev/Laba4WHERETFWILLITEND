#include "RenderTarget.h"
RenderTarget::RenderTarget(TGAImage image, std::string& name, std::shared_ptr<Gdevice> device, TextureUsage usage) {
	texture_= std::make_shared<GTexture>(image, name, device, usage);
	handle_ = device->heaps_->CreateRTV_CPU(texture_->GetResourse()->GetResourse());
}
RenderTarget::RenderTarget(UINT width, UINT height, std::string& name, std::shared_ptr<Gdevice> device, TextureUsage usage) {
	texture_= std::make_shared<GTexture>(width, height, name, device, usage, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
	handle_ = device->heaps_->CreateRTV_CPU(texture_->GetResourse()->GetResourse());
}
RenderTarget::RenderTarget(std::shared_ptr<GResourse> Gresourse, TextureUsage usage) {
	texture_= std::make_shared<GTexture>(Gresourse, usage);
	handle_ = Gresourse->GetDevice()->heaps_->CreateRTV_CPU(texture_->GetResourse()->GetResourse());
}
RenderTarget::RenderTarget(std::shared_ptr<GTexture> texture) {
	texture_ = texture;
	handle_ = texture_->GetResourse()->GetDevice()->heaps_->CreateRTV_CPU(texture_->GetResourse()->GetResourse());
}