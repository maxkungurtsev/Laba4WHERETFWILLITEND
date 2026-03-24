#include "Gdevice.h"
#include <sstream>
void Gdevice::ViewportScissorSetup() {
    viewport_ = {};
    viewport_.TopLeftX = 0;
    viewport_.TopLeftY = 0;
    viewport_.Width = static_cast<float>(width_);
    viewport_.Height = static_cast<float>(height_);
    viewport_.MinDepth = 0.0f;
    viewport_.MaxDepth = 1.0f;
    scissor_rect_ = {};
    scissor_rect_.left = 0;
    scissor_rect_.top = 0;
    scissor_rect_.right = width_;
    scissor_rect_.bottom = height_;
}
ComPtr<ID3D12Device> Gdevice::GetDXDevice() {
    return device_;
};
void Gdevice::CreateID3DResourse(D3D12_HEAP_PROPERTIES& heapProps, D3D12_RESOURCE_DESC& resdesc, ComPtr<ID3D12Resource>& resourse, D3D12_RESOURCE_STATES initial_state, D3D12_CLEAR_VALUE* clear_value) {
    HRESULT hr = device_->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resdesc, initial_state, clear_value, IID_PPV_ARGS(resourse.ReleaseAndGetAddressOf()));
    if (FAILED(hr)) {
        std::ostringstream ss;
        ss << "CreateTexture FAILED: 0x" << std::hex << hr << "\n";
        OutputDebugStringA(ss.str().c_str());
        throw std::runtime_error(ss.str());
    }
}

Gdevice::Gdevice(UINT width, UINT height, int num_descriptors) {
    width_ = width;
    height_ = height;
    HRESULT hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device_));
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create graphics device");
    }
    fence_= std::make_shared<GFence>(device_);
    cmd_= std::make_shared<CMD>(device_);
    heaps_=std::make_shared<GHeaps>(num_descriptors, device_);
    ViewportScissorSetup();
}