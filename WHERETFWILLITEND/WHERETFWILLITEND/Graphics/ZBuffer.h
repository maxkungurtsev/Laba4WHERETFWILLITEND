#pragma once
#include "GTexture.h"
class Zbuffer {
public:
	std::shared_ptr<GTexture> z_buffer_;
	Handle handle_;
	Zbuffer(TGAImage image, std::string& name, std::shared_ptr<Gdevice> device, TextureUsage usage);
	Zbuffer(UINT width, UINT height, std::string& name, std::shared_ptr<Gdevice> device, TextureUsage usage);
	Zbuffer(std::shared_ptr<GResourse> Gresourse, TextureUsage usage);
	Zbuffer(std::shared_ptr<GTexture> texture);
};