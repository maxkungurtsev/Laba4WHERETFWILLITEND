#pragma once
#include "GResourse.h"
#include "tgaimage.h"
enum class TextureUsage
{
    Albedo,
    Diffuse = Albedo,
    // Treat Diffuse and Albedo textures the same.,
    // Treat height and depth textures the same.
    Normalmap,
    Heightmap,
    Depth = Heightmap
};

class GTexture{
	UINT width_=0;
	UINT height_=0;
    TextureUsage usage_;
	std::shared_ptr<GResourse> Gresourse_;
    DXGI_FORMAT formats[3] = { DXGI_FORMAT_R8G8B8A8_UNORM , DXGI_FORMAT_B8G8R8A8_UNORM ,DXGI_FORMAT_R32_TYPELESS };
    D3D12_RESOURCE_STATES initial_states_[3] = { D3D12_RESOURCE_STATE_COPY_DEST , D3D12_RESOURCE_STATE_COPY_DEST ,D3D12_RESOURCE_STATE_DEPTH_WRITE };
	void FillData(UINT width, UINT height, std::string& name, std::shared_ptr<Gdevice> device, TextureUsage usage, D3D12_RESOURCE_FLAGS flag = D3D12_RESOURCE_FLAG_NONE);
public:
    std::shared_ptr<GResourse> GetResourse();
    GTexture(TGAImage image, std::string& name, std::shared_ptr<Gdevice> device, TextureUsage usage);
	GTexture(UINT width, UINT height, std::string& name, std::shared_ptr<Gdevice> device, TextureUsage usage, D3D12_RESOURCE_FLAGS flag);
    GTexture(std::shared_ptr<GResourse> Gresourse, TextureUsage usage);
};