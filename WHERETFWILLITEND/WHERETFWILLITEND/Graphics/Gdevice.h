#pragma once
#include "d3dx12.h"
#include <d3d12.h> 
#include <dxgi1_6.h>
#include <windows.h>
#include <wrl.h>
#include <iostream>
#include "GFence.h"
#include "CMD.h"
#include "Heaps.h"
using Microsoft::WRL::ComPtr;
class Gdevice : public std::enable_shared_from_this<Gdevice> {
public:
	ComPtr<ID3D12Device> device_;
	std::shared_ptr<CMD> cmd_;
	std::shared_ptr<GFence> fence_;
	std::shared_ptr<GHeaps> heaps_;
	UINT sample_amount_ = 1;
	UINT msaa_quality_ = 0;
	UINT width_;
	UINT height_;
	D3D12_VIEWPORT viewport_;
	D3D12_RECT scissor_rect_;
	Gdevice(UINT width, UINT height, int num_descriptors);
	ComPtr<ID3D12Device> GetDXDevice();
	void ViewportScissorSetup();
	void CreateID3DResourse(D3D12_HEAP_PROPERTIES& heapProps, D3D12_RESOURCE_DESC& resdesc, ComPtr<ID3D12Resource> resourse, D3D12_CLEAR_VALUE* clear_value = nullptr);
};