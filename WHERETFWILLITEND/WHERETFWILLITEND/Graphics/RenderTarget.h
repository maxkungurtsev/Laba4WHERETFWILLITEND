#pragma once
#include "GTexture.h"
class RenderTarget {
public:
	std::shared_ptr<GTexture> texture_;
	D3D12_CPU_DESCRIPTOR_HANDLE handle_;
	RenderTarget(TGAImage image, std::string& name, std::shared_ptr<Gdevice> device, TextureUsage usage);
	RenderTarget(UINT width, UINT height, std::string& name, std::shared_ptr<Gdevice> device, TextureUsage usage);
	RenderTarget(std::shared_ptr<GResourse> Gresourse, TextureUsage usage);
	RenderTarget(std::shared_ptr<GTexture> texture);
};