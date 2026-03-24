#pragma once
#include <d3d12.h> 
#include <dxgi1_6.h>
#include <windows.h>
#include <wrl.h>
#include <iostream>
using Microsoft::WRL::ComPtr;
class Gdevice;
struct Handle {
	D3D12_CPU_DESCRIPTOR_HANDLE cpu_;
	D3D12_GPU_DESCRIPTOR_HANDLE gpu_;
};
class GHeaps {
private:
	ComPtr<ID3D12Device> device_;
	ComPtr<ID3D12DescriptorHeap> rtv_heap_;
	int rtv_amount_ = 0;
	ComPtr<ID3D12DescriptorHeap> dsv_heap_;
	int dsv_amount_ = 0;
	ComPtr<ID3D12DescriptorHeap> cbv_srv_uav_heap_;
	int cbv_srv_uav_amount_ = 0;
	ComPtr<ID3D12DescriptorHeap> sampler_heap_;
	int sampler_amount_ = 0;
	int rtv_descriptor_size_;
	int dsv_descriptor_size_;
	int cbv_srv_uav_descriptor_size_;
	int sampler_descriptor_size_;
public:
	void CreateGHeaps(int num_descriptors, ComPtr<ID3D12Device> device);
	ComPtr<ID3D12DescriptorHeap> GetRTVHeap();
	ComPtr<ID3D12DescriptorHeap> GetDSVHeap();
	ComPtr<ID3D12DescriptorHeap> GetCBV_SRV_UAV_Heap();
	ComPtr<ID3D12DescriptorHeap> GetSamplerHeap();
	int GetRTVHeapDescriptorSize();
	int GetDSVHeapDescriptorSize();
	int GetCBV_SRV_UAV_HeapDescriptorSize();
	int GetSamplerHeapDescriptorSize();
	Handle CreateSRV_CPU(DXGI_FORMAT Format, ComPtr<ID3D12Resource> resourse);
	Handle CreateDSV_CPU(ComPtr<ID3D12Resource> resourse);
	Handle CreateRTV_CPU(ComPtr<ID3D12Resource> resourse);
	Handle CreateCBV_CPU(ComPtr<ID3D12Resource> resourse, UINT size_in_bytes);
	Handle MakeSampler();
};