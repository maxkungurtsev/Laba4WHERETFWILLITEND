#pragma once
#include "GResourse.h"
#include "tgaimage.h"

enum class TextureUsage
{
    Albedo,
    Diffuse = Albedo,
    // Treat Diffuse and Albedo textures the same.
    Heightmap,
    Depth = Heightmap,
    // Treat height and depth textures the same.
    Normalmap,
};

class GTexture{
	UINT width_=0;
	UINT height_=0;
    TextureUsage usage_;
	std::shared_ptr<GResourse> Gresourse_;
	void FillData(UINT width, UINT height, std::string& name, std::shared_ptr<Gdevice> device, TextureUsage usage = TextureUsage::Albedo);
public:
    std::shared_ptr<GResourse> GetResourse();
    GTexture(TGAImage image, std::string& name, std::shared_ptr<Gdevice> device, TextureUsage usage= TextureUsage::Albedo);
	GTexture(UINT width, UINT height, std::string& name, std::shared_ptr<Gdevice> device, TextureUsage usage = TextureUsage::Albedo);
    GTexture(std::shared_ptr<GResourse> Gresourse, TextureUsage usage = TextureUsage::Albedo);
};