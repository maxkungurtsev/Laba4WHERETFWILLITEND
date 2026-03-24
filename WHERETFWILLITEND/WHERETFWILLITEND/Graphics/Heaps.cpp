#include "Heaps.h"
#include "Gdevice.h"
GHeaps::GHeaps(int num_descriptors, ComPtr<ID3D12Device> device) {
		device_ = device;
		rtv_descriptor_size_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		dsv_descriptor_size_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		cbv_srv_uav_descriptor_size_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		sampler_descriptor_size_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
		D3D12_DESCRIPTOR_HEAP_DESC desc{};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		desc.NumDescriptors = num_descriptors;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		desc.NodeMask = 0;
		HRESULT hr = device_->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&rtv_heap_));
		if (FAILED(hr)) {
			throw std::runtime_error("Failed to create RTV heap");
		}
		desc.NumDescriptors = num_descriptors;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		hr = device_->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&dsv_heap_));
		if (FAILED(hr)) {
			throw std::runtime_error("Failed to create DSV heap");
		}
		D3D12_DESCRIPTOR_HEAP_DESC cbvDesc{};
		cbvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbvDesc.NumDescriptors = num_descriptors;
		cbvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		hr = device_->CreateDescriptorHeap(&cbvDesc, IID_PPV_ARGS(&cbv_srv_uav_heap_));
		if (FAILED(hr)) {
			throw std::runtime_error("Failed to create CBV, SRV and UAV heap");
		}
		D3D12_DESCRIPTOR_HEAP_DESC sampDesc{};
		sampDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		sampDesc.NumDescriptors = num_descriptors;
		sampDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		hr = device_->CreateDescriptorHeap(&sampDesc, IID_PPV_ARGS(&sampler_heap_));
		if (FAILED(hr)) {
			throw std::runtime_error("Failed to create Sampler heap");
		}
}
ComPtr<ID3D12DescriptorHeap> GHeaps::GetRTVHeap() {
	return rtv_heap_;
}
ComPtr<ID3D12DescriptorHeap> GHeaps::GetDSVHeap() {
	return dsv_heap_;
}
ComPtr<ID3D12DescriptorHeap> GHeaps::GetCBV_SRV_UAV_Heap() {
	return cbv_srv_uav_heap_;
}
ComPtr<ID3D12DescriptorHeap> GHeaps::GetSamplerHeap() {
	return sampler_heap_;
}

int GHeaps::GetRTVHeapDescriptorSize() {return rtv_descriptor_size_; }
int GHeaps::GetDSVHeapDescriptorSize() { return dsv_descriptor_size_; }
int GHeaps::GetCBV_SRV_UAV_HeapDescriptorSize() { return cbv_srv_uav_descriptor_size_; }
int GHeaps::GetSamplerHeapDescriptorSize() { return sampler_descriptor_size_; }

// In the begining God created SRV, CBV and RTV.
Handle GHeaps::CreateSRV_CPU(DXGI_FORMAT Format, ComPtr<ID3D12Resource> resourse) {
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	Handle handle;
	D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle = cbv_srv_uav_heap_->GetCPUDescriptorHandleForHeapStart();
	cpu_handle.ptr += (cbv_srv_uav_amount_ * cbv_srv_uav_descriptor_size_);
	handle.cpu_ = cpu_handle;
	D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle = cbv_srv_uav_heap_->GetGPUDescriptorHandleForHeapStart();
	gpu_handle.ptr += (cbv_srv_uav_amount_ * cbv_srv_uav_descriptor_size_);
	handle.gpu_ = gpu_handle;
	cbv_srv_uav_amount_++;
	device_->CreateShaderResourceView(resourse.Get(), &srvDesc, cpu_handle);
	return handle;
};
Handle GHeaps::CreateCBV_CPU(ComPtr<ID3D12Resource> resourse, UINT size_in_bytes) {
	//CBV desc
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc{};
	cbv_desc.BufferLocation = resourse->GetGPUVirtualAddress();
	cbv_desc.SizeInBytes = size_in_bytes;
	Handle handle;
	D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle = cbv_srv_uav_heap_->GetCPUDescriptorHandleForHeapStart();
	cpu_handle.ptr += (cbv_srv_uav_amount_ * cbv_srv_uav_descriptor_size_);
	handle.cpu_ = cpu_handle;
	D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle = cbv_srv_uav_heap_->GetGPUDescriptorHandleForHeapStart();
	gpu_handle.ptr += (cbv_srv_uav_amount_ * cbv_srv_uav_descriptor_size_);
	handle.gpu_ = gpu_handle;
	cbv_srv_uav_amount_++;
	device_->CreateConstantBufferView(&cbv_desc, cpu_handle);
	return handle;
}
Handle GHeaps::CreateRTV_CPU(ComPtr<ID3D12Resource> resourse) {
	Handle handle;
	D3D12_CPU_DESCRIPTOR_HANDLE cpu_rtv_handle = rtv_heap_->GetCPUDescriptorHandleForHeapStart();
	cpu_rtv_handle.ptr += (rtv_amount_ * rtv_descriptor_size_);
	handle.cpu_ = cpu_rtv_handle;
	D3D12_GPU_DESCRIPTOR_HANDLE gpu_rtv_handle = rtv_heap_->GetGPUDescriptorHandleForHeapStart();
	gpu_rtv_handle.ptr += (rtv_amount_ * rtv_descriptor_size_);
	handle.gpu_ = gpu_rtv_handle;
	rtv_amount_++;
	device_->CreateRenderTargetView(resourse.Get(), nullptr, cpu_rtv_handle);
	return handle;
}
Handle GHeaps::CreateDSV_CPU(ComPtr<ID3D12Resource> resourse) {
	Handle handle;
	D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle = dsv_heap_->GetCPUDescriptorHandleForHeapStart();
	cpu_handle.ptr += (dsv_amount_ * dsv_descriptor_size_);
	device_->CreateDepthStencilView(resourse.Get(), nullptr, cpu_handle);
	handle.cpu_ = cpu_handle;
	D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle = dsv_heap_->GetGPUDescriptorHandleForHeapStart();
	gpu_handle.ptr += (dsv_amount_ * dsv_descriptor_size_);
	handle.gpu_ = gpu_handle;
	dsv_amount_++;
	return handle;
};
Handle GHeaps::MakeSampler() {
	Handle handle;
	D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle = sampler_heap_->GetCPUDescriptorHandleForHeapStart();
	cpu_handle.ptr += (sampler_amount_ * sampler_descriptor_size_);
	D3D12_SAMPLER_DESC samplerDesc{};
    samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
    samplerDesc.MipLODBias = 0;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	device_->CreateSampler(&samplerDesc, cpu_handle);
	handle.cpu_ = cpu_handle;
	D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle = sampler_heap_->GetGPUDescriptorHandleForHeapStart();
	cpu_handle.ptr += (sampler_amount_ * sampler_descriptor_size_);
	handle.gpu_ = gpu_handle;
	sampler_amount_++;
	return handle;
}