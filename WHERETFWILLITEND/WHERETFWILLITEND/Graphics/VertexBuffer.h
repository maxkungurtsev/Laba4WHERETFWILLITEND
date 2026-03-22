#pragma once
#include "Gdevice.h"
#include "../Model.h"

class VertexBuffer {
private:
    ComPtr<ID3D12Resource> vertex_buffer_;
    D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view_;
    UINT vertex_count_=0;
public:
    VertexBuffer(model& model);
};