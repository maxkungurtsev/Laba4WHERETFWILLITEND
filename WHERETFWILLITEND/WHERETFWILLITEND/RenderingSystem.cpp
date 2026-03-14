#include "RenderingSystem.h"
void RenderingSystem::InitGbuffer(ID3D12Device* device, uint32_t width, uint32_t height,
    const D3D12_CPU_DESCRIPTOR_HANDLE* rtvHandles,
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle,
    const D3D12_CPU_DESCRIPTOR_HANDLE* srvHandles)
{
    device_ = device;
    // 1. Создаём GBuffer
    gbuffer_.Create(device_, width, height, rtvHandles, dsvHandle, srvHandles);
}

void RenderingSystem::Create_RootSignature() {
    D3D12_ROOT_PARAMETER rootParams[3] = {};

    // CBV для pass constants
    rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParams[0].Descriptor.ShaderRegister = 0;
    rootParams[0].Descriptor.RegisterSpace = 0;
    rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    // CBV для light constants
    rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParams[1].Descriptor.ShaderRegister = 1;
    rootParams[1].Descriptor.RegisterSpace = 0;
    rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    // SRV таблица для GBuffer в Lighting Pass
    D3D12_DESCRIPTOR_RANGE srvRange = {};
    srvRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    srvRange.NumDescriptors = 3; // Diffuse + Normal + Depth
    srvRange.BaseShaderRegister = 0;
    srvRange.RegisterSpace = 0;
    srvRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    rootParams[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParams[2].DescriptorTable.NumDescriptorRanges = 1;
    rootParams[2].DescriptorTable.pDescriptorRanges = &srvRange;
    rootParams[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    D3D12_ROOT_SIGNATURE_DESC rootSigDesc = {};
    rootSigDesc.NumParameters = _countof(rootParams);
    rootSigDesc.pParameters = rootParams;
    rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;

    HRESULT hr = D3D12SerializeRootSignature(
        &rootSigDesc,
        D3D_ROOT_SIGNATURE_VERSION_1,
        &serializedRootSig,
        &errorBlob
    );

    if (FAILED(hr)) {
        if (errorBlob) {
            std::cerr << "RootSignature Error: " << (char*)errorBlob->GetBufferPointer() << std::endl;
        }
        throw std::runtime_error("Failed to serialize root signature");
    }

    hr = device_->CreateRootSignature(
        0,
        serializedRootSig->GetBufferPointer(),
        serializedRootSig->GetBufferSize(),
        IID_PPV_ARGS(&rootSignature_)
    );
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create root signature");
    }
}

void RenderingSystem::CompileShaders() {
    ComPtr<ID3DBlob> errorBlob;
    HRESULT hr;
    //geom VS
    hr = D3DCompileFromFile(L"Geometry_VS.hlsl", nullptr, nullptr, "main", "vs_5_0", 0, 0, &geom_vertex_shader_, &errorBlob);
    if (FAILED(hr)) {
        if (errorBlob) {
            std::cerr << "Shader compile error: ";
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        }
        else {
            OutputDebugStringA("Shader compile failed, but no error message was produced.");
        }
        throw std::runtime_error("Failed to compile geom vertex shader");
    }
    //PS
    hr = D3DCompileFromFile(L"Geometry_PS.hlsl", nullptr, nullptr, "main", "ps_5_0", 0, 0, &geom_pixel_shader_, &errorBlob);
    if (FAILED(hr)) {
        if (errorBlob) {
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        }
        throw std::runtime_error("Failed to compile geom pixel shader");
    }
    //pixel VS
    hr = D3DCompileFromFile(L"Lighting_VS.hlsl", nullptr, nullptr, "main", "vs_5_0", 0, 0, &light_vertex_shader_, &errorBlob);
    if (FAILED(hr)) {
        if (errorBlob) {
            std::cerr << "Shader compile error: ";
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        }
        else {
            OutputDebugStringA("Shader compile failed, but no error message was produced.");
        }
        throw std::runtime_error("Failed to compile light vertex shader");
    }
    //PS
    hr = D3DCompileFromFile(L"Lighting_PS.hlsl", nullptr, nullptr, "main", "ps_5_0", 0, 0, &light_pixel_shader_, &errorBlob);
    if (FAILED(hr)) {
        if (errorBlob) {
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        }
        throw std::runtime_error("Failed to compile light pixel shader");
    }
};

void RenderingSystem::CreateInputLayout() {
    geom_input_layout_ =
    {
        {
            "POSITION",
            0,
            DXGI_FORMAT_R32G32B32_FLOAT,
            0,
            0,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
            0
        },
        {
            "NORMAL",
            0,
            DXGI_FORMAT_R32G32B32_FLOAT,
            0,
            12,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
            0
        },
        {
            "TEXCOORD",
            0,
            DXGI_FORMAT_R32G32_FLOAT,
            0,
            24,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
            0
        }
    };
}

void RenderingSystem::Create_PSOs() {
    // Geometry Pass PSO
        D3D12_GRAPHICS_PIPELINE_STATE_DESC geomDesc = {};
        geomDesc.pRootSignature = rootSignature_.Get();
        geomDesc.VS = { geom_vertex_shader_->GetBufferPointer(), geom_vertex_shader_->GetBufferSize() };
        geomDesc.PS = { geom_pixel_shader_->GetBufferPointer(), geom_pixel_shader_->GetBufferSize() };
        // Input Layout для вершин
        geomDesc.InputLayout = { geom_input_layout_.data(), (UINT)geom_input_layout_.size() };
        geomDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        geomDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        geomDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
        geomDesc.SampleMask = UINT_MAX;
        geomDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        geomDesc.NumRenderTargets = 2;
        geomDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        geomDesc.RTVFormats[1] = DXGI_FORMAT_R16G16B16A16_FLOAT;  
        geomDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;   
        geomDesc.SampleDesc.Count = 1;

        HRESULT hr = device_->CreateGraphicsPipelineState(&geomDesc, IID_PPV_ARGS(&geometryPSO_));
        if (FAILED(hr)) throw std::runtime_error("Failed to create geometry PSO");
        D3D12_GRAPHICS_PIPELINE_STATE_DESC lightDesc = {};
        lightDesc.pRootSignature = rootSignature_.Get();
        lightDesc.VS = { light_vertex_shader_->GetBufferPointer(), light_vertex_shader_->GetBufferSize() };
        lightDesc.PS = { light_pixel_shader_->GetBufferPointer(), light_pixel_shader_->GetBufferSize() };
        lightDesc.InputLayout = { nullptr, 0 };
        // Растеризация: не отсекать полноэкранный треугольник
        lightDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
        lightDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
        lightDesc.RasterizerState.FrontCounterClockwise = FALSE;
        lightDesc.RasterizerState.DepthClipEnable = TRUE;

        // Блендинг: по умолчанию (запись цвета)
        lightDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

        // Глубина: НЕ тестируется и НЕ пишется (читаем из текстуры)
        lightDesc.DepthStencilState.DepthEnable = FALSE;
        lightDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
        lightDesc.DepthStencilState.StencilEnable = FALSE;

        lightDesc.SampleMask = UINT_MAX;
        lightDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        lightDesc.NumRenderTargets = 1;
        lightDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        lightDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
        lightDesc.SampleDesc.Count = 1;

        HRESULT hr = device_->CreateGraphicsPipelineState(&lightDesc, IID_PPV_ARGS(&lightingPSO_));
        if (FAILED(hr)) throw std::runtime_error("Failed to create lighting PSO");
}

void RenderingSystem::GeometryPass(ID3D12GraphicsCommandList* cmdList, Model& mesh)
{
    gbuffer_.BindForGeometryPass(cmdList);
    // 3. Bind PSO и Root Signature
    cmdList->SetPipelineState(geometryPSO_.Get());
    cmdList->SetGraphicsRootSignature(rootSigGeometry_.Get());

    // 4. Bind Constant Buffers (CBV)
    cmdList->SetGraphicsRootConstantBufferView(0, cbPerPass_->GetGPUVirtualAddress());
    cmdList->SetGraphicsRootConstantBufferView(1, cbLight_->GetGPUVirtualAddress());
    // 5. Топология
    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    // 6. Draw Meshes
    for (auto& submesh : mesh){
        cmdList->IASetVertexBuffers(0, 1, &submesh.vertexBufferView);
        cmdList->IASetIndexBuffer(&submesh.indexBufferView);
        cmdList->DrawIndexedInstanced(submesh.indexCount, 1, 0, 0, 0);
    }
}

void RenderingSystem::LightingPass(ID3D12GraphicsCommandList * cmdList){
    // 1. Подготовка G-Buffer для чтения (барьеры + привязка кучи)
    gbuffer_.BindForLightingPass(cmdList, srvHeap_.Get());

    // 2. Привязка бэк-буфера для записи итогового цвета
    cmdList->OMSetRenderTargets(1, &currentBackBufferRTV_, FALSE, nullptr);

    // 3. Очистка бэк-буфера (фон для неосвещённых областей)
    const FLOAT clearColor[4] = { 0.05f, 0.05f, 0.1f, 1.0f };
    cmdList->ClearRenderTargetView(currentBackBufferRTV_, clearColor, 0, nullptr);

    // 4. Привязка пайплайна и корневой сигнатуры
    cmdList->SetPipelineState(lightingPSO_.Get());
    cmdList->SetGraphicsRootSignature(rootSigLighting_.Get());

    // 5. Привязка константных буферов
    cmdList->SetGraphicsRootConstantBufferView(0, cbPerPass_->GetGPUVirtualAddress());
    cmdList->SetGraphicsRootConstantBufferView(1, cbLight_->GetGPUVirtualAddress());

    // 6. Привязка таблицы дескрипторов для текстур (РАСКОММЕНТИРОВАТЬ при готовности!)
    cmdList->SetGraphicsRootDescriptorTable(2,srvHeap_->GetGPUDescriptorHandleForHeapStart());
    // 7. Отрисовка полноэкранного треугольника
    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    cmdList->DrawInstanced(3, 1, 0, 0);
}

void RenderingSystem::RenderFrame(ID3D12GraphicsCommandList* cmdList, Model& mesh){
    // 1. Переход backbuffer из PRESENT -> RENDER_TARGET
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = currentBackBuffer_;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    cmdList->ResourceBarrier(1, &barrier);

    // 2. Geometry Pass (запись в GBuffer)
    GeometryPass(cmdList, mesh);

    // 3. Lighting Pass (GBuffer -> backbuffer)
    LightingPass(cmdList);

    // 4. Переход backbuffer обратно в PRESENT
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

    cmdList->ResourceBarrier(1, &barrier);
}