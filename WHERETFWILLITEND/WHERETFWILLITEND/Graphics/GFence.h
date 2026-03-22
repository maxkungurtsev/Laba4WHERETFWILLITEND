#pragma once
#include <d3d12.h> 
#include <dxgi1_6.h>
#include <windows.h>
#include <wrl.h>
#include <iostream>
using Microsoft::WRL::ComPtr;
class Gdevice;
class GFence {
private:
	ComPtr<ID3D12Fence> fence_;
	UINT fence_value_ = 0;
public:
	UINT GetFenceValue();
	void IncrementFenceValue();
	ComPtr<ID3D12Fence> GetFence();
	void CreateFence(std::shared_ptr<Gdevice> device);
};