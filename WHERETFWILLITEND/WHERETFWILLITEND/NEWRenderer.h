#pragma once
#include "Graphics/RootSignature.h"
#include "Graphics/RenderTarget.h"
#include "Graphics/GBuffer.h"
#include "Graphics/PSO.h"
#include "Window.h"
#include "Model.h"
struct PassConstants {
	XMFLOAT4X4 model;
	XMFLOAT4X4 inv_model;
	XMFLOAT4X4 view;
	XMFLOAT4X4 inv_view;
	XMFLOAT4X4 projection;
	XMFLOAT4X4 inv_projection;
};
class NewRenderer {
private:
    TGAImage dummy_;
	std::shared_ptr<Gdevice> device_;
	std::shared_ptr<PSO> pso_;
	std::shared_ptr<RootSignature> root_signature_;
	GBuffer g_buffer_;
public:
	void Initialize(UINT width, UINT height, int num_descriptors, Window hwnd);
};