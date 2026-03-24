#pragma once
#include "Graphics/RootSignature.h"
#include "Graphics/RenderTarget.h"
#include "Graphics/GBuffer.h"
#include "Graphics/CBuffer.h"
#include "Graphics/PSO.h"
#include "Window.h"
#include "Model.h"
#include <d3dcompiler.h>
struct LightData{
	XMFLOAT3 strength;
	float falloff_start;
	float falloff_end;
	XMVECTOR direction;
	XMVECTOR position;
	float spot_power;
};
struct PassConstants {
	XMFLOAT4X4 model;
	XMFLOAT4X4 inv_model;
	XMFLOAT4X4 view;
	XMFLOAT4X4 inv_view;
	XMFLOAT4X4 projection;
	XMFLOAT4X4 inv_projection;
	XMVECTOR cam_pos;
	XMVECTOR cam_forward;
	float time;
	XMFLOAT3 amb_light;
	std::vector<LightData> lights;
};
class NewRenderer {
private:
    TGAImage dummy_;
	std::shared_ptr<Gdevice> device_;
	std::shared_ptr<PSO> pso_;
	std::shared_ptr<RootSignature> geom_root_signature_;
	std::shared_ptr<RootSignature> light_root_signature_;
	std::vector<D3D12_INPUT_ELEMENT_DESC> input_layout_;
	std::shared_ptr<GBuffer> g_buffer_;
	std::vector<ComPtr<ID3D12Resource>> back_buffer_;
	UINT current_backbuffer_ = 0;
    UINT vertex_count_;
	ComPtr<ID3D12Resource> vertex_buffer_;
	D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view_;
	int frame_count_;
	Cbuffer<PassConstants> cbuffer_;
	ComPtr<IDXGISwapChain3> swap_chain_;
	ComPtr<ID3DBlob> vertex_shader_;
	ComPtr<ID3DBlob> vertex_shader_anim_;
	ComPtr<ID3DBlob> pixel_shader_;
	std::shared_ptr<Model> mesh_;
public:
	void CreateBackbuffer();
	void CreateGeomRootSign(int textures_amount);
	void CreateLightRootSign();
	void CreateVertexBuffer(std::shared_ptr<Model> mesh);
	void CreateInputLayout();
	void FillCbuffer(XMVECTOR cam_pos, XMVECTOR look_at, XMVECTOR up, int time, XMFLOAT3 amb_light = { 0.0f,0.3f,0.3f });
	void Initialize(UINT width, UINT height, int num_descriptors, Window hwnd, std::string mesh_path, XMVECTOR cam_pos, XMVECTOR look_at, XMVECTOR up, int time);
	void CompileShader(std::wstring path, ComPtr<ID3DBlob> shader);
	void RenderFrame();
};