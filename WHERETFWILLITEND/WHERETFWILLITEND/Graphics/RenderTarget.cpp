#include "RenderTarget.h"
RenderTarget::RenderTarget(TGAImage image, std::string& name, std::shared_ptr<Gdevice> device, TextureUsage usage) {
	texture_= std::make_shared<GTexture>(image, name, device, usage);
	ComPtr<ID3D12Resource> res = texture_->GetResourse()->GetResourse();
	handle_ = device->heaps_->CreateRTV_CPU(res);
}
RenderTarget::RenderTarget(UINT width, UINT height, std::string& name, std::shared_ptr<Gdevice> device, TextureUsage usage) {
	texture_= std::make_shared<GTexture>(width, height, name, device, usage, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
	ComPtr<ID3D12Resource> res = texture_->GetResourse()->GetResourse();
	handle_ = device->heaps_->CreateRTV_CPU(res);
}
RenderTarget::RenderTarget(std::shared_ptr<GResourse> Gresourse, TextureUsage usage) {
	texture_= std::make_shared<GTexture>(Gresourse, usage);
	ComPtr<ID3D12Resource> res = texture_->GetResourse()->GetResourse();
	handle_ = Gresourse->GetDevice()->heaps_->CreateRTV_CPU(res);
}
RenderTarget::RenderTarget(std::shared_ptr<GTexture> texture) {
	texture_ = texture;
	ComPtr<ID3D12Resource> res = texture_->GetResourse()->GetResourse();
	handle_ = texture_->GetResourse()->GetDevice()->heaps_->CreateRTV_CPU(res);
}