#pragma once
#include <d3dcompiler.h>
#include "PSO.h"
class PostProccess {
private:
	std::shared_ptr<Gdevice> device_;
	std::shared_ptr<PSO> pso_;
	std::shared_ptr<RootSignature> root_sign_;
	std::vector<Type> type_array_;
	std::vector<int> amount_array_;
	std::vector<D3D12_SHADER_VISIBILITY> visibility_array_;
	ComPtr<ID3DBlob> vertex_shader_;
	ComPtr<ID3DBlob> pixel_shader_;
	int parameter_amount_=0;
	void CompileShader(std::wstring path, ComPtr<ID3DBlob>& shader, std::string& type);
public:
	std::shared_ptr<PSO> GetPSO() { return pso_; }
	std::shared_ptr<RootSignature> GetRootSign() { return root_sign_; }
	PostProccess(std::shared_ptr<Gdevice> device, std::vector<Type>& type_array, std::vector<int>& amount_array, std::vector<D3D12_SHADER_VISIBILITY>& visibility_array, std::string& pixel_shader,
		std::vector<D3D12_INPUT_ELEMENT_DESC>& input_layout, std::vector<DXGI_FORMAT>& formats);
	void ApplyPostProc(const float clearColor[4], D3D12_GPU_DESCRIPTOR_HANDLE base, D3D12_GPU_DESCRIPTOR_HANDLE sampler, std::vector<D3D12_GPU_DESCRIPTOR_HANDLE>& parameters, D3D12_CPU_DESCRIPTOR_HANDLE& rtvHandle);
};