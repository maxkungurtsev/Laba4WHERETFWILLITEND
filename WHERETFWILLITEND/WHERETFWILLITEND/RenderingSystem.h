#pragma once
#include "G_buffer.h"
#include "Model.h"
using Microsoft::WRL::ComPtr;
class RenderingSystem
{
public:
    void InitGbuffer(ID3D12Device* device, uint32_t width, uint32_t height,
        const D3D12_CPU_DESCRIPTOR_HANDLE* rtvHandles,
        D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle,
        const D3D12_CPU_DESCRIPTOR_HANDLE* srvHandles);
    void RenderFrame(ID3D12GraphicsCommandList* cmdList, Model& mesh);
    void CompileShaders();
    void CreateInputLayout();
    void Create_RootSignature();
    void Create_PSOs();
private:
    void GeometryPass(ID3D12GraphicsCommandList* cmdList, Model& mesh);
    void LightingPass(ID3D12GraphicsCommandList* cmdList);
    ID3D12Device* device_;
    GBuffer gbuffer_;
    ID3D12PipelineState* geometryPSO_;
    ID3D12PipelineState* lightingPSO_;
    ID3D12RootSignature* rootSignature_;
    ComPtr<ID3DBlob> geom_vertex_shader_;
    ComPtr<ID3DBlob> geom_pixel_shader_;
    ComPtr<ID3DBlob> light_vertex_shader_;
    ComPtr<ID3DBlob> light_pixel_shader_;
    std::vector<D3D12_INPUT_ELEMENT_DESC> geom_input_layout_;
};