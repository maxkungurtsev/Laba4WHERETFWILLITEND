#pragma once
#include "Gdevice.h"
#include <vector>
class BackBuffer {
	std::vector<ComPtr<ID3D12Resource>> back_buffer_;
	std::vector<Handle> handles_;
	ComPtr<IDXGISwapChain3> swap_chain_;
	UINT current_backbuffer_ = 0;
public:
	BackBuffer(int frame_count, ComPtr<IDXGISwapChain3>& swap_chain, std::shared_ptr<Gdevice> device);
	Handle& GetCurrentHandle();
	void SetCurrentBackBuffer();
	ComPtr<ID3D12Resource>& GetCurrentBackBuffer();
};