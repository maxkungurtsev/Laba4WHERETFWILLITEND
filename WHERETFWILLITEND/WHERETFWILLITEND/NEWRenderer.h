#pragma once
#include "Graphics/RootSignature.h"
#include "Graphics/RenderTarget.h"
#include "Graphics/BackBuffer.h"
#include "Graphics/Constants.h"
#include "Graphics/GBuffer.h"
#include "Graphics/CBuffer.h"
#include "Graphics/PSO.h"
#include "Window.h"
#include "Model.h"
#include <d3dcompiler.h>
#include "RenderingSystem.h"
class NewRenderer {
private:
	std::shared_ptr<Gdevice> device_;
	int frame_count_;
	ComPtr<IDXGISwapChain3> swap_chain_;
	std::shared_ptr <BackBuffer> back_buffer_;
	std::shared_ptr<RenderingSystem> render_system_;
public:
	NewRenderer(UINT width, UINT height, int frame_count, Window* hwnd, std::string mesh_path, XMVECTOR cam_pos, XMVECTOR look_at, XMVECTOR up, float time);
	void RenderFrame(float time, XMVECTOR look_at, XMVECTOR cam_pos, XMVECTOR up, bool shootlight);
};