#pragma once
#include "RootSignature.h"
class PSO {
private:
	std::shared_ptr<Gdevice> device_;
	ComPtr<ID3DBlob> compute_shader_;
	ComPtr<ID3DBlob> vertex_shader_;
	ComPtr<ID3DBlob> geom_shader_;
	ComPtr<ID3DBlob> pixel_shader_;
	ComPtr<ID3DBlob> hull_shader_;
	ComPtr<ID3DBlob> domain_shader_;
	ComPtr<ID3D12PipelineState> pipeline_state_;
	std::vector<D3D12_INPUT_ELEMENT_DESC> input_layout_;
public:
	PSO(std::vector<D3D12_INPUT_ELEMENT_DESC> input_layout, ComPtr<ID3DBlob> vertex_shader, ComPtr<ID3DBlob> pixel_shader, std::shared_ptr<Gdevice> device, std::shared_ptr<RootSignature> root_sign, int rtv_amount, std::vector<DXGI_FORMAT> formats);
	ComPtr<ID3D12PipelineState> GetPSO();
	PSO(std::vector<D3D12_INPUT_ELEMENT_DESC> input_layout, ComPtr<ID3DBlob> vertex_shader, ComPtr<ID3DBlob> pixel_shader, ComPtr<ID3DBlob> hull_shader, ComPtr<ID3DBlob> domain_shader, std::shared_ptr<Gdevice> device, std::shared_ptr<RootSignature> root_sign, int rtv_amount, std::vector<DXGI_FORMAT> formats);
	PSO(std::vector<D3D12_INPUT_ELEMENT_DESC> input_layout, ComPtr<ID3DBlob> vertex_shader, ComPtr<ID3DBlob> geom_shader, ComPtr<ID3DBlob> pixel_shader, std::shared_ptr<Gdevice> device, std::shared_ptr<RootSignature> root_sign, int rtv_amount, std::vector<DXGI_FORMAT> formats);
};