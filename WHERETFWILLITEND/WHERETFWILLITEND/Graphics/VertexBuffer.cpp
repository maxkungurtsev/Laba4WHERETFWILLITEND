#include "VertexBuffer.h"
VertexBuffer::VertexBuffer(model& model, std::shared_ptr<Gdevice> device) {
    const std::vector<Vertex>& vertices = model.GetVertices();
    vertex_count_ = vertices.size();
    UINT bufferSize = vertex_count_ * sizeof(Vertex);
    D3D12_RESOURCE_DESC bufferDesc{};
    bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufferDesc.Width = bufferSize;
    bufferDesc.Height = 1;
    bufferDesc.DepthOrArraySize = 1;
    bufferDesc.MipLevels = 1;
    bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
    bufferDesc.SampleDesc.Count = 1;
    bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    D3D12_HEAP_PROPERTIES heapProps{};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    HRESULT hr = device->GetDXDevice()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertex_buffer_));
    if (FAILED(hr))
        throw std::runtime_error("Failed to create vertex buffer");
    void* mappedData = nullptr;
    D3D12_RANGE readRange{ 0, 0 };
    hr = vertex_buffer_->Map(0, &readRange, &mappedData);
    if (FAILED(hr))
        throw std::runtime_error("Failed to fill vertex buffer");
    memcpy(mappedData, vertices.data(), bufferSize);
    vertex_buffer_->Unmap(0, nullptr);
    vertex_buffer_view_.BufferLocation = vertex_buffer_->GetGPUVirtualAddress();
    vertex_buffer_view_.StrideInBytes = sizeof(Vertex);
    vertex_buffer_view_.SizeInBytes = bufferSize;


};
