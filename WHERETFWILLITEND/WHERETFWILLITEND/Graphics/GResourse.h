#pragma once
#include "Gdevice.h"
class GResourse{
private:
	std::shared_ptr<Gdevice> device_;
	std::string name_;
	ComPtr<ID3D12Resource> resourse_;
	D3D12_RESOURCE_DESC res_desc_{};
	D3D12_CPU_DESCRIPTOR_HANDLE srv_handle;
public:
	GResourse(D3D12_RESOURCE_DESC res_desc, D3D12_HEAP_PROPERTIES heapProps, std::string& name, std::shared_ptr<Gdevice> device, D3D12_CLEAR_VALUE* clear_value = nullptr);
	ComPtr<ID3D12Resource> GetResourse();
	std::shared_ptr<Gdevice> GetDevice();
	D3D12_RESOURCE_DESC& GetResDesc();
	void CreateSwapChain(std::shared_ptr<Gdevice> device);
};