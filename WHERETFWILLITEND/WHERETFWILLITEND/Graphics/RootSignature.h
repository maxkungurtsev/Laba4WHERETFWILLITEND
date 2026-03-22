#pragma once
#include "Gdevice.h"
using Microsoft::WRL::ComPtr;

enum class Type {
	cbv,
	srv,
	sampler
};

class RootSignature {
    ComPtr<ID3D12RootSignature> root_signature_;
	std::vector<D3D12_ROOT_PARAMETER1> root_params_;
	int base_shader_register_cbv_=0;
	int base_shader_register_srv_=0;
	int base_shader_register_sampler_=0;
	void AddParameter(Type type, int descriptor_amount, D3D12_SHADER_VISIBILITY visibility);
	void CreateRootSignature(std::shared_ptr<Gdevice> device);
	ComPtr<ID3D12RootSignature> GetRootSign();
};