#include "GFence.h"
#include "Gdevice.h"
void GFence::CreateFence(ComPtr<ID3D12Device>  device) {
	HRESULT hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to create fence");
	}
}
UINT GFence::GetFenceValue() {
	return fence_value_;
}
ComPtr<ID3D12Fence> GFence::GetFence() {
	return fence_;
}
void GFence::IncrementFenceValue() {
	fence_value_++;
}