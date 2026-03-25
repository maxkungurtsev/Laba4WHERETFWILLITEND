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
class NewRenderer {
private:
	std::shared_ptr<Gdevice> device_;
	int frame_count_;
	ComPtr<IDXGISwapChain3> swap_chain_;
	//////////////////////////////////////////////////////
	std::shared_ptr<PSO> pso_;
	std::shared_ptr<PSO> pso_anim_;
	std::shared_ptr<RootSignature> geom_root_signature_;
	std::shared_ptr<RootSignature> light_root_signature_;
	std::vector<D3D12_INPUT_ELEMENT_DESC> input_layout_;
	std::shared_ptr<GBuffer> g_buffer_;
	std::shared_ptr<BackBuffer> back_buffer_;
    UINT vertex_count_;
	ComPtr<ID3D12Resource> vertex_buffer_;
	D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view_;
	std::shared_ptr <Cbuffer<PassConstants>> cbuffer_;
	ComPtr<ID3DBlob> vertex_shader_;
	ComPtr<ID3DBlob> vertex_shader_anim_;
	ComPtr<ID3DBlob> pixel_shader_;
	std::shared_ptr<Model> mesh_;
	Handle Sampler_handle_;
	////////////////////////////////////////////////////////
public:
	void CreateGeomRootSign(int textures_amount);
	void CreateLightRootSign();
	void CreateVertexBuffer(std::shared_ptr<Model> mesh);
	void CreateInputLayout();
	void FillCbuffer(XMVECTOR cam_pos, XMVECTOR look_at, XMVECTOR up, int time, XMFLOAT3 amb_light = { 0.0f,0.3f,0.3f });
	NewRenderer(UINT width, UINT height, int num_descriptors, Window* hwnd, std::string mesh_path, XMVECTOR cam_pos, XMVECTOR look_at, XMVECTOR up, int time);
	void CompileShader(std::wstring path, ComPtr<ID3DBlob>& shader, std::string& type);
	void RenderFrame(float time);
};