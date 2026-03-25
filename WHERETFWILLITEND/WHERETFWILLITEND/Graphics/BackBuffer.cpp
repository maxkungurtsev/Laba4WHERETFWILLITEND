#include "BackBuffer.h"

BackBuffer::BackBuffer(int frame_count, ComPtr<IDXGISwapChain3>& swap_chain, std::shared_ptr<Gdevice> device) {
    swap_chain_ = swap_chain;
    back_buffer_ = std::vector<ComPtr<ID3D12Resource>>(frame_count);
    handles_ = std::vector<Handle>(frame_count);
    for (UINT i = 0; i < frame_count; i++) {
        HRESULT hr = swap_chain->GetBuffer(i, IID_PPV_ARGS(&back_buffer_[i]));
        if (FAILED(hr)) {
            throw std::runtime_error("Failed to get swapchain buffer");
        }
        handles_[i]=device->heaps_->CreateRTV_CPU(back_buffer_[i]);
    }
}
Handle& BackBuffer::GetCurrentHandle() {
    return handles_[current_backbuffer_];
};
ComPtr<ID3D12Resource>& BackBuffer::GetCurrentBackBuffer() {
    return back_buffer_[current_backbuffer_];
};
void BackBuffer::SetCurrentBackBuffer() {
    current_backbuffer_= swap_chain_->GetCurrentBackBufferIndex();
}