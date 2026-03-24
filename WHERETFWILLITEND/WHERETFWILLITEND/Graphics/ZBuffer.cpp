#include "ZBuffer.h"

#include "RenderTarget.h"
Zbuffer::Zbuffer(TGAImage image, std::string& name, std::shared_ptr<Gdevice> device, TextureUsage usage) {
	z_buffer_ = std::make_shared<GTexture>(image, name, device, usage);
	ComPtr<ID3D12Resource> res = z_buffer_->GetResourse()->GetResourse();
	handle_ = device->heaps_->CreateDSV_CPU(res);
}
Zbuffer::Zbuffer(UINT width, UINT height, std::string& name, std::shared_ptr<Gdevice> device, TextureUsage usage) {
	z_buffer_ = std::make_shared<GTexture>(width, height, name, device, usage, D3D12_RESOURCE_FLAG_NONE);
	ComPtr<ID3D12Resource> res = z_buffer_->GetResourse()->GetResourse();
	handle_ = device->heaps_->CreateDSV_CPU(res);
}
Zbuffer::Zbuffer(std::shared_ptr<GResourse> Gresourse, TextureUsage usage) {
	z_buffer_ = std::make_shared<GTexture>(Gresourse, usage);
	ComPtr<ID3D12Resource> res = z_buffer_->GetResourse()->GetResourse();
	handle_ = Gresourse->GetDevice()->heaps_->CreateDSV_CPU(res);
}
Zbuffer::Zbuffer(std::shared_ptr<GTexture> texture) {
	z_buffer_ = texture;
	ComPtr<ID3D12Resource> res = z_buffer_->GetResourse()->GetResourse();
	handle_ = z_buffer_->GetResourse()->GetDevice()->heaps_->CreateDSV_CPU(res);
}