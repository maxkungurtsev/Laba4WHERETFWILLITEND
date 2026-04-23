#pragma once
#include "Gdevice.h"
using Microsoft::WRL::ComPtr;

enum class Type {
	cbv,
	srv,
	uav,
	sampler
};

class RootSignature {
private:
    ComPtr<ID3D12RootSignature> root_signature_;
	std::vector<D3D12_ROOT_PARAMETER1> root_params_;
	int base_shader_register_cbv_=0;
	int base_shader_register_srv_ = 0;
	int base_shader_register_uav_=0;
	int base_shader_register_sampler_=0;
	std::vector<D3D12_DESCRIPTOR_RANGE1> ranges_;
public:
	void AddParameter(Type type, int descriptor_amount, D3D12_SHADER_VISIBILITY visibility, int base_register=-1);
	void CreateRootSignature(std::shared_ptr<Gdevice> device);
	ComPtr<ID3D12RootSignature> GetRootSign();
};