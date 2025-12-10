#include "Renderer.h"
HRESULT Renderer::InitDevice() {
    return D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
};