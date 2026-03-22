#include "RootSignature.h"
void RootSignature::AddParameter(Type type, int descriptor_amount, D3D12_SHADER_VISIBILITY visibility){
		D3D12_DESCRIPTOR_RANGE1 range{};
	switch (type) {
	case Type::cbv:
		range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		range.NumDescriptors = descriptor_amount;
		range.BaseShaderRegister = base_shader_register_cbv_;
		range.RegisterSpace = 0;
		range.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC;
		range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		base_shader_register_cbv_ += descriptor_amount;
		break;

	case Type::srv:
		range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		range.NumDescriptors = descriptor_amount;
		range.BaseShaderRegister = base_shader_register_srv_;
		range.RegisterSpace = 0;
		range.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE;
		range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		base_shader_register_srv_ += descriptor_amount;
		break;

	case Type::sampler:
		range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
		range.NumDescriptors = descriptor_amount;
		range.BaseShaderRegister = base_shader_register_sampler_;
		range.RegisterSpace = 0;
		range.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;
		range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		base_shader_register_sampler_ += descriptor_amount;
		break;
	}
	D3D12_ROOT_PARAMETER1 new_root_param;
	new_root_param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	new_root_param.DescriptorTable.NumDescriptorRanges = 1;
	new_root_param.DescriptorTable.pDescriptorRanges = &range;
	new_root_param.ShaderVisibility = visibility;
	root_params_.push_back(new_root_param);
}
void RootSignature::CreateRootSignature(std::shared_ptr<Gdevice> device) {
	D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootSigDesc{};
	rootSigDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
	rootSigDesc.Desc_1_1.NumParameters = root_params_.size();
	rootSigDesc.Desc_1_1.pParameters = root_params_.data();
	rootSigDesc.Desc_1_1.NumStaticSamplers = 0;
	rootSigDesc.Desc_1_1.pStaticSamplers = nullptr;
	rootSigDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	ComPtr<ID3DBlob> serialized;
	ComPtr<ID3DBlob> error;
	HRESULT hr = D3D12SerializeVersionedRootSignature(&rootSigDesc, &serialized, &error);
	if (FAILED(hr)) {
		if (error) OutputDebugStringA((char*)error->GetBufferPointer());
		throw std::runtime_error("Failed to serialize root signature");
	}
	hr = device->GetDXDevice()->CreateRootSignature(0, serialized->GetBufferPointer(), serialized->GetBufferSize(), IID_PPV_ARGS(&root_signature_));
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to create root signature");
	}
}

ComPtr<ID3D12RootSignature> RootSignature::GetRootSign(){
	return root_signature_;
}