#pragma once
#include "Graphics/RootSignature.h"
#include "Graphics/RenderTarget.h"
#include "Graphics/BackBuffer.h"
#include "Graphics/Constants.h"
#include "Graphics/GBuffer.h"
#include "Graphics/CBuffer.h"
#include "Graphics/PSO.h"
#include "Window.h"
#include "Lights.h"
#include "Model.h"
#include <d3dcompiler.h>

class RenderingSystem {
	std::shared_ptr<Gdevice> device_;
	std::shared_ptr<PSO> geom_pso_;
	std::shared_ptr<PSO> geom_pso_tes_;
	std::shared_ptr<PSO> geom_pso_anim_;
	std::shared_ptr<PSO> geom_pso_water_tes_;
	std::shared_ptr<PSO> light_pso_;
	std::shared_ptr<RootSignature> geom_root_signature_;
	std::shared_ptr<RootSignature> light_root_signature_;
	std::vector<D3D12_INPUT_ELEMENT_DESC> input_layout_;
	std::shared_ptr<GBuffer> g_buffer_;
	std::shared_ptr <Cbuffer<PassConstants>> cbuffer_;
	std::shared_ptr <Lights> light_buffer_;
	ComPtr<ID3DBlob> geom_vertex_shader_;
	ComPtr<ID3DBlob> hull_shader_;
	ComPtr<ID3DBlob> domain_shader_;
	ComPtr<ID3DBlob> water_domain_shader_;
	ComPtr<ID3DBlob> geom_vertex_shader_anim_;
	ComPtr<ID3DBlob> geom_pixel_shader_;
	ComPtr<ID3DBlob> light_vertex_shader_;
	ComPtr<ID3DBlob> light_pixel_shader_;
	ComPtr<ID3DBlob> light_pixel_shader_wire_;
	BoundingFrustum frustum_;
	std::vector<std::shared_ptr<Model>> meshes_;
	bool culling_enabled_=true;
	Handle Sampler_handle_;
	bool first_frame_ = true;
public:
	void CreateGeomRootSign();
	void CreateLightRootSign();
	void CreateInputLayout();
	void SetupGeomPass(const float clearColor[4]);
	void FillCbuffer(XMVECTOR cam_pos, XMVECTOR look_at, XMVECTOR up, float time, XMMATRIX world = XMMatrixIdentity());
	RenderingSystem(std::shared_ptr<Gdevice> device, std::vector<std::string> mesh_pathes, XMVECTOR cam_pos, XMVECTOR look_at, XMVECTOR up, float time);
	void CompileShader(std::wstring path, ComPtr<ID3DBlob>& shader, std::string& type);
	void GeomPass(std::shared_ptr<Model> mesh);
	void ParseModelToCBuffer(std::shared_ptr<Model> mesh);
	void LightPass(const float clearColor[4], D3D12_CPU_DESCRIPTOR_HANDLE& rtvHandle);
	void RenderFrame(float time, XMVECTOR look_at, XMVECTOR cam_pos, XMVECTOR up, D3D12_CPU_DESCRIPTOR_HANDLE& rtvHandle, bool shootlight, bool culling_enabled);
};