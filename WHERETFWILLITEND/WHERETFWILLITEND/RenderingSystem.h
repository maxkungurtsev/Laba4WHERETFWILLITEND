#pragma once
#include "Graphics/RootSignature.h"
#include "Graphics/PostProccess.h"
#include "Graphics/RenderTarget.h"
#include "Graphics/BackBuffer.h"
#include "Graphics/Constants.h"
#include "Graphics/GBuffer.h"
#include "Graphics/CBuffer.h"
#include "ParticleEmiter.h"
#include "Window.h"
#include "Lights.h"
#include "Model.h"
#include <d3dcompiler.h>

class RenderingSystem {
	std::vector<std::shared_ptr<ParticleEmiter>> emiters_;
	std::vector<DXGI_FORMAT> PSO_formats_ = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_B8G8R8A8_UNORM ,DXGI_FORMAT_R32_SINT };
	std::shared_ptr<Gdevice> device_;
	std::vector<D3D12_INPUT_ELEMENT_DESC> input_layout_;
	// root signatures
	std::shared_ptr<RootSignature> geom_root_signature_;
	std::shared_ptr<RootSignature> shadow_root_signature_;
	std::shared_ptr<RootSignature> compute_root_signature_;
	std::shared_ptr<RootSignature> particle_root_signature_;
	std::shared_ptr<RootSignature> light_root_signature_;
	std::shared_ptr<RootSignature> present_root_signature_;
	std::vector<std::shared_ptr<RootSignature>> post_proc_root_signs;
	//buffers
	std::shared_ptr<GBuffer> g_buffer_;
	std::shared_ptr <Cbuffer<PassConstants>> pass_buffer_;
	std::shared_ptr <Cbuffer<POVConstants>> pov_buffer_;
	std::shared_ptr <Lights> light_buffer_;
	//particles
	ComPtr<ID3DBlob> p_compute_shader_;
	ComPtr<ID3DBlob> p_vertex_shader_;
	ComPtr<ID3DBlob> p_pixel_shader_;
	std::shared_ptr<PSO> particle_pso_;
	std::shared_ptr<PSO> compute_pso_;
	//tesselation
	ComPtr<ID3DBlob> hull_shader_;
	ComPtr<ID3DBlob> domain_shader_;
	ComPtr<ID3DBlob> water_domain_shader_;
	std::shared_ptr<PSO> geom_pso_tes_;
	std::shared_ptr<PSO> geom_pso_water_tes_;
	//animated
	ComPtr<ID3DBlob> geom_vertex_shader_anim_;
	std::shared_ptr<PSO> geom_pso_anim_;
	// normal ones
	ComPtr<ID3DBlob> geom_vertex_shader_;
	ComPtr<ID3DBlob> geom_pixel_shader_;
	std::shared_ptr<PSO> geom_pso_;
	//shadows
	ComPtr<ID3DBlob> shadow_vertex_shader_;
	ComPtr<ID3DBlob> shadow_pixel_shader_;
	std::shared_ptr<PSO> shadow_pso_;
	// light pass
	ComPtr<ID3DBlob> light_vertex_shader_;
	ComPtr<ID3DBlob> light_pixel_shader_;
	std::shared_ptr<PSO> light_pso_;
	//present pass
	ComPtr<ID3DBlob> empty_vertex_shader_;
	ComPtr<ID3DBlob> empty_pixel_shader_;
	std::shared_ptr<PSO> present_pso_;
	BoundingFrustum frustum_;
	std::vector<std::shared_ptr<Model>> meshes_;
	std::shared_ptr<RenderTarget> scene_color;
	std::shared_ptr<RenderTarget> post_proccess_color0;
	std::shared_ptr<RenderTarget> post_proccess_color1;
	std::vector<std::shared_ptr<PostProccess>> post_procs;
	std::vector<std::vector<D3D12_GPU_DESCRIPTOR_HANDLE>> post_proc_params;

	bool culling_enabled_=true;
	Handle Sampler_handle_;
	bool first_frame_ = true;
	void CreateGeomRootSign();
	void CreateParticleRootSign();
	void CreateLightRootSign();
	void CreateComputeRootSign();
	void CreatePresentRootSign();
	void CreateParticlePSO();
	void CreateComputePSO();
	void CreatePresentPSO();
	void CreateInputLayout();
	void InitGeomPass();
	void InitLightPass();
	void InitPresentPass();
	void SetupGeomPass(const float clearColor[4], bool with_pbr);
	void FillCbuffers(XMVECTOR cam_pos, XMVECTOR look_at, XMVECTOR up, float time, XMMATRIX world = XMMatrixIdentity());
	std::shared_ptr<Model> BilBoardMesh(std::shared_ptr<Model> mesh, float time, XMVECTOR look_at, XMVECTOR cam_pos, XMVECTOR up);
	void CompileShader(std::wstring path, ComPtr<ID3DBlob>& shader, std::string& type);
	void GeomPass(std::shared_ptr<Model> mesh);
	void ComputePass(std::shared_ptr<ParticleEmiter> emiter);
	void ParticlePass(std::shared_ptr<ParticleEmiter> emiter);
	void ShadowPass(XMVECTOR camera_pos, XMVECTOR camera_target, XMVECTOR camera_up_, float fov_y, const float clearColor[4], float time);
	void LightPass(const float clearColor[4], D3D12_CPU_DESCRIPTOR_HANDLE& rtvHandle);
	void PresentPass(const float clearColor[4], D3D12_GPU_DESCRIPTOR_HANDLE& ToRender, D3D12_CPU_DESCRIPTOR_HANDLE& rtvHandle);
	void CompileEmptyShaders();
public:
	RenderingSystem(std::shared_ptr<Gdevice> device, std::vector<std::string> mesh_pathes, XMVECTOR cam_pos, XMVECTOR look_at, XMVECTOR up, float time);
	void RenderFrame(float time, XMVECTOR look_at, XMVECTOR cam_pos, XMVECTOR up, D3D12_CPU_DESCRIPTOR_HANDLE& rtvHandle, bool shootlight, bool culling_enabled);
};