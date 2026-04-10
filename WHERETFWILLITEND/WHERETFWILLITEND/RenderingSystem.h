#pragma once
#include "Graphics/RootSignature.h"
#include "Graphics/RenderTarget.h"
#include "Graphics/BackBuffer.h"
#include "Graphics/Constants.h"
#include "Graphics/GBuffer.h"
#include "Graphics/CBuffer.h"
#include "Graphics/PSO.h"
#include "OctreeNode.h"
#include "Window.h"
#include "Lights.h"
#include "Model.h"
#include <d3dcompiler.h>

class RenderingSystem {
	std::shared_ptr<Gdevice> device_;
	std::shared_ptr<PSO> geom_pso_;
	std::shared_ptr<PSO> geom_pso_tes_;
	std::shared_ptr<PSO> geom_pso_anim_;
	std::shared_ptr<PSO> geom_pso_anim_tes_;
	std::shared_ptr<PSO> light_pso_;
	std::shared_ptr<RootSignature> geom_root_signature_;
	std::shared_ptr<RootSignature> light_root_signature_;
	std::vector<D3D12_INPUT_ELEMENT_DESC> input_layout_;
	std::shared_ptr<GBuffer> g_buffer_;
	UINT vertex_count_;
	ComPtr<ID3D12Resource> vertex_buffer_;
	D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view_;
	ComPtr<ID3D12Resource> index_buffer_;
	ComPtr<ID3D12Resource> index_buffer_upload_buffer_;
	D3D12_INDEX_BUFFER_VIEW index_buffer_view_;
	std::shared_ptr <Cbuffer<PassConstants>> cbuffer_;
	std::shared_ptr <Lights> light_buffer_;
	ComPtr<ID3DBlob> geom_vertex_shader_;
	ComPtr<ID3DBlob> hull_shader_;
	ComPtr<ID3DBlob> domain_shader_;
	ComPtr<ID3DBlob> geom_vertex_shader_anim_;
	ComPtr<ID3DBlob> geom_pixel_shader_;
	ComPtr<ID3DBlob> light_vertex_shader_;
	ComPtr<ID3DBlob> light_pixel_shader_;
	ComPtr<ID3DBlob> light_pixel_shader_wire_;
	BoundingFrustum frustum_;
	std::shared_ptr<Model> mesh_;
	std::shared_ptr<OctreeNode> octree_;
	Handle Sampler_handle_;
	bool first_frame_ = true;
public:
	void CreateGeomRootSign(int textures_amount);
	void CreateLightRootSign();
	void CreateVertexBuffer(std::shared_ptr<Model> mesh);
	void CreateIndexBuffer(std::shared_ptr<Model> model);
	void CreateInputLayout();
	void FillCbuffer(XMVECTOR cam_pos, XMVECTOR look_at, XMVECTOR up, float time);
	RenderingSystem(std::shared_ptr<Gdevice> device, std::string mesh_path, XMVECTOR cam_pos, XMVECTOR look_at, XMVECTOR up, float time);
	void CompileShader(std::wstring path, ComPtr<ID3DBlob>& shader, std::string& type);
	void GeomPass(const float clearColor[4]);
	void ParseModelToCBuffer();
	void LightPass(const float clearColor[4], D3D12_CPU_DESCRIPTOR_HANDLE& rtvHandle);
	void RenderFrame(float time, XMVECTOR look_at, XMVECTOR cam_pos, XMVECTOR up, D3D12_CPU_DESCRIPTOR_HANDLE& rtvHandle, bool shootlight);
};