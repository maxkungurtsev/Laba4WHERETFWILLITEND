#include "RenderingSystem.h"

static inline D3D12_RESOURCE_BARRIER Transition(
    ID3D12Resource* res,
    D3D12_RESOURCE_STATES before,
    D3D12_RESOURCE_STATES after)
{
    D3D12_RESOURCE_BARRIER b{};
    b.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    b.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    b.Transition.pResource = res;
    b.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    b.Transition.StateBefore = before;
    b.Transition.StateAfter = after;
    return b;
}


void RenderingSystem::CreateGeomRootSign(int textures_amount) {
    if (geom_root_signature_ == nullptr) {
        geom_root_signature_ = std::make_shared<RootSignature>();
    }
    //cbv
    geom_root_signature_->AddParameter(Type::cbv, 1, D3D12_SHADER_VISIBILITY_ALL);
    // textures diffuse
    geom_root_signature_->AddParameter(Type::srv, 1, D3D12_SHADER_VISIBILITY_ALL);
    //normal
    geom_root_signature_->AddParameter(Type::srv, 1, D3D12_SHADER_VISIBILITY_ALL);
    //sampler
    geom_root_signature_->AddParameter(Type::sampler, 1, D3D12_SHADER_VISIBILITY_ALL);
    OutputDebugStringA("sampler made\n");
    //creating root sign with said params
    geom_root_signature_->CreateRootSignature(device_);
};

void RenderingSystem::CreateLightRootSign() {
    if (light_root_signature_ == nullptr) {
        light_root_signature_ = std::make_shared<RootSignature>();
    }
    //cbv
    light_root_signature_->AddParameter(Type::cbv, 1, D3D12_SHADER_VISIBILITY_PIXEL);
    // g buffer
    light_root_signature_->AddParameter(Type::srv, 1, D3D12_SHADER_VISIBILITY_PIXEL);
    light_root_signature_->AddParameter(Type::srv, 1, D3D12_SHADER_VISIBILITY_PIXEL);
    light_root_signature_->AddParameter(Type::srv, 1, D3D12_SHADER_VISIBILITY_PIXEL);
    light_root_signature_->AddParameter(Type::srv, 1, D3D12_SHADER_VISIBILITY_PIXEL);
    //sampler
    light_root_signature_->AddParameter(Type::sampler, 1, D3D12_SHADER_VISIBILITY_PIXEL);
    //creating root sign with said params
    light_root_signature_->CreateRootSignature(device_);
};
/// <summary>
///  C BUFFER
/// </summary>
void RenderingSystem::AddDirLight(XMFLOAT4 direction, XMFLOAT3 strength) {
    LightData light;
    light.direction = direction;
    light.strength = strength;
    light.type = 0;
    int index = min(127, cbuffer_->GetData().max_lights);
    OutputDebugStringA(std::to_string(index).c_str());
    OutputDebugStringA(" - index of directional light added\n");
    cbuffer_->GetData().lights[index] = light;
    cbuffer_->GetData().max_lights += 1;
    cbuffer_->Save_changes();
};


void RenderingSystem::AddPointLight(float falloff_end, float falloff_start, XMFLOAT4 position, XMFLOAT3 strength) {
    LightData light;
    light.falloff_end = falloff_end;
    light.falloff_start = falloff_start;
    light.position = position;
    light.strength = strength;
    light.type = 1;
    int index = min(127, cbuffer_->GetData().max_lights);
    OutputDebugStringA(std::to_string(index).c_str());
    OutputDebugStringA(" - index of point light added\n");
    cbuffer_->GetData().lights[index] = light;
    cbuffer_->GetData().max_lights += 1;
    cbuffer_->Save_changes();
};
void RenderingSystem::AddSpotLight(XMFLOAT4 direction, float falloff_end, float falloff_start, XMFLOAT4 position, XMFLOAT3 strength, float spot_power) {
    LightData light;
    light.direction = direction;
    light.falloff_end = cos(falloff_end);
    light.falloff_start = cos(falloff_start);
    light.position = position;
    light.spot_power = spot_power;
    light.strength = strength;
    light.type = 2;
    int index = min(127, cbuffer_->GetData().max_lights);
    OutputDebugStringA(std::to_string(index).c_str());
    OutputDebugStringA(" - index of spot light added\n");
    cbuffer_->GetData().lights[index] = light;
    cbuffer_->GetData().max_lights += 1;
    cbuffer_->Save_changes();
};

void RenderingSystem::FillCbuffer(XMVECTOR cam_pos, XMVECTOR look_at, XMVECTOR up, int time, XMFLOAT3 amb_light) {
    //identity
    XMStoreFloat4x4(&cbuffer_->GetData().model, XMMatrixIdentity());
    //view
    XMStoreFloat4x4(&cbuffer_->GetData().view, XMMatrixLookAtLH(cam_pos, look_at, up));
    //projection
    XMStoreFloat4x4(&cbuffer_->GetData().projection, XMMatrixPerspectiveFovLH(XM_PIDIV4, float(device_->width_) / float(device_->height_), 0.1f, 10000.0f));
    //inv identity
    XMStoreFloat4x4(&cbuffer_->GetData().inv_model, XMMatrixInverse(nullptr, XMLoadFloat4x4(&cbuffer_->GetData().model)));
    //inv view
    XMStoreFloat4x4(&cbuffer_->GetData().inv_view, XMMatrixInverse(nullptr, XMLoadFloat4x4(&cbuffer_->GetData().view)));
    //inv projection
    XMStoreFloat4x4(&cbuffer_->GetData().inv_projection, XMMatrixInverse(nullptr, XMLoadFloat4x4(&cbuffer_->GetData().projection)));
    cbuffer_->GetData().time = time;
    cbuffer_->GetData().amb_light = amb_light;
    XMStoreFloat4(&cbuffer_->GetData().cam_pos, cam_pos);
    XMStoreFloat4(&cbuffer_->GetData().cam_forward, XMVector3Normalize(look_at - cam_pos));
    cbuffer_->Save_changes();
};
void RenderingSystem::ParseModelToCBuffer() {
    for (int i = 0; i < mesh_->GetMaterials().size(); i++) {
        cbuffer_->GetData().mats[i].ambient_ = mesh_->GetMaterials()[i].ambient_k;
        cbuffer_->GetData().mats[i].diffuse_ = mesh_->GetMaterials()[i].diffuse_k;
        cbuffer_->GetData().mats[i].spec_ = mesh_->GetMaterials()[i].specular_k;
        cbuffer_->GetData().mats[i].shiny_ = mesh_->GetMaterials()[i].shiny_k;
        if (mesh_->GetMaterials()[i].hasHeightTexture) {
            cbuffer_->GetData().mats[i].NormalType = 2;
        }
        else if (mesh_->GetMaterials()[i].hasNormTexture) {
            cbuffer_->GetData().mats[i].NormalType = 1;
        }
        else {
            cbuffer_->GetData().mats[i].NormalType = 0;
        }
    }
    cbuffer_->Save_changes();
}
void RenderingSystem::CreateInputLayout() {
    input_layout_ =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}, 
        {"TEXCOORD", 1, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 2, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };
}

void RenderingSystem::CreateVertexBuffer(std::shared_ptr<Model> model) {
    const std::vector<Vertex>& vertices = model->GetVertices();
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
    HRESULT hr = device_->GetDXDevice()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertex_buffer_));
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
}

void RenderingSystem::CreateIndexBuffer(std::shared_ptr<Model> model){
    //device_->cmd_->ResetAllocator();
    std::vector<uint32_t> indices = model->Getindices();

    UINT32 bufferSize = static_cast<UINT32>(indices.size() * sizeof(uint32_t));
    D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
    D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    device_->GetDXDevice()->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&index_buffer_));

    void* mapped = nullptr;
    CD3DX12_RANGE readRange(0, 0);
    index_buffer_->Map(0, &readRange, &mapped);
    memcpy(mapped, indices.data(), static_cast<size_t>(bufferSize));
    index_buffer_->Unmap(0, nullptr);
    index_buffer_view_.BufferLocation = index_buffer_->GetGPUVirtualAddress();
    index_buffer_view_.SizeInBytes = bufferSize;
    index_buffer_view_.Format = DXGI_FORMAT_R32_UINT;
}

void RenderingSystem::CompileShader(std::wstring path, ComPtr<ID3DBlob>& shader, std::string& type) {
    ComPtr<ID3DBlob> errorBlob;
    HRESULT hr = D3DCompileFromFile(path.c_str(), nullptr, nullptr, "main", type.c_str(), 0, 0, &shader, &errorBlob);
    if (FAILED(hr)) {
        if (errorBlob) {
            std::cerr << "Shader compile error: ";
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        }
        else {
            OutputDebugStringA("Shader compile failed, but no error message was produced.");
        }
        throw std::runtime_error("Failed to compile shader");
    }
};

void RenderingSystem::GeomPass(const float clearColor[4]) {
    device_->cmd_->command_list_->SetPipelineState(geom_pso_->GetPSO().Get());
    device_->cmd_->command_list_->SetGraphicsRootSignature(geom_root_signature_->GetRootSign().Get());

    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = g_buffer_->depth_->handle_.cpu_;

    //clearing
    device_->cmd_->command_list_->ClearRenderTargetView(g_buffer_->albedo_->handle_.cpu_, clearColor, 0, nullptr);
    device_->cmd_->command_list_->ClearRenderTargetView(g_buffer_->normal_->handle_.cpu_, clearColor, 0, nullptr);
    device_->cmd_->command_list_->ClearRenderTargetView(g_buffer_->material_index_->handle_.cpu_, clearColor, 0, nullptr);
    device_->cmd_->command_list_->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // setting gbuffer as render target
    D3D12_CPU_DESCRIPTOR_HANDLE handles[3] = { g_buffer_->albedo_->handle_.cpu_, g_buffer_->normal_->handle_.cpu_, g_buffer_->material_index_->handle_.cpu_};
    device_->cmd_->command_list_->OMSetRenderTargets(3, &g_buffer_->albedo_->handle_.cpu_, TRUE, &dsvHandle);

    //desc tables setup
    device_->cmd_->command_list_->SetGraphicsRootDescriptorTable(0, cbuffer_->GetHandle().gpu_);
    device_->cmd_->command_list_->SetGraphicsRootDescriptorTable(3, Sampler_handle_.gpu_);
    device_->cmd_->command_list_->IASetVertexBuffers(0, 1, &vertex_buffer_view_);
    device_->cmd_->command_list_->IASetIndexBuffer(&index_buffer_view_);
    //OutputDebugStringA();
    for (const auto& submesh : mesh_->GetSubMeshes()) {
        //diffuse textures
       //OutputDebugStringA(std::to_string(current_mat).c_str());
        //OutputDebugStringA("\n");
        cbuffer_->GetData().current_mat = submesh.materialIndex;
        cbuffer_->Save_changes();
        device_->cmd_->command_list_->SetGraphicsRootDescriptorTable(1, mesh_->GetMaterials()[submesh.materialIndex].diffuseTexture->GetResourse()->GetHandle().gpu_);
        //normal textures
        device_->cmd_->command_list_->SetGraphicsRootDescriptorTable(2, mesh_->GetMaterials()[submesh.materialIndex].HeightNormTexture->GetResourse()->GetHandle().gpu_);
        if (mesh_->GetMaterials()[submesh.materialIndex].diffuseTexPath == "textures/sponza_thorn_diff.tga" or mesh_->GetMaterials()[submesh.materialIndex].diffuseTexPath == "textures/vase_plant.tga")
        {
            device_->cmd_->command_list_->SetPipelineState(geom_pso_anim_->GetPSO().Get());
        }
        else {
            if (mesh_->GetMaterials()[submesh.materialIndex].hasHeightTexture){
                //device_->cmd_->command_list_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                //device_->cmd_->command_list_->SetPipelineState(geom_pso_->GetPSO().Get());
                device_->cmd_->command_list_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
                device_->cmd_->command_list_->SetPipelineState(geom_pso_tes_->GetPSO().Get());
            }
            else {
                device_->cmd_->command_list_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                device_->cmd_->command_list_->SetPipelineState(geom_pso_->GetPSO().Get());
            }
        }
        device_->cmd_->command_list_->DrawIndexedInstanced(static_cast<UINT>(submesh.indexCount), 1, static_cast<UINT>(submesh.firstIndex), 0, 0);
    }
}
void RenderingSystem::LightPass(const float clearColor[4], D3D12_CPU_DESCRIPTOR_HANDLE& rtvHandle) {
    // set pso
    device_->cmd_->command_list_->SetPipelineState(light_pso_->GetPSO().Get());
    device_->cmd_->command_list_->SetGraphicsRootSignature(light_root_signature_->GetRootSign().Get());
    // set & cler dsv, rtv
    //D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = g_buffer_->depth_->handle_.cpu_;
    device_->cmd_->command_list_->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
    device_->cmd_->command_list_->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    //set desc tables
    device_->cmd_->command_list_->SetGraphicsRootDescriptorTable(0, cbuffer_->GetHandle().gpu_);
    device_->cmd_->command_list_->SetGraphicsRootDescriptorTable(1, g_buffer_->albedo_->texture_->GetResourse()->GetHandle().gpu_);
    device_->cmd_->command_list_->SetGraphicsRootDescriptorTable(2, g_buffer_->normal_->texture_->GetResourse()->GetHandle().gpu_);
    device_->cmd_->command_list_->SetGraphicsRootDescriptorTable(3, g_buffer_->depth_->z_buffer_->GetResourse()->GetHandle().gpu_);
    device_->cmd_->command_list_->SetGraphicsRootDescriptorTable(4, g_buffer_->material_index_->texture_->GetResourse()->GetHandle().gpu_);
    device_->cmd_->command_list_->SetGraphicsRootDescriptorTable(5, Sampler_handle_.gpu_);
    // draw
    device_->cmd_->command_list_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    device_->cmd_->command_list_->DrawInstanced(3, 1, 0, 0);
}
RenderingSystem::RenderingSystem(std::shared_ptr<Gdevice> device, std::string mesh_path, XMVECTOR cam_pos, XMVECTOR look_at, XMVECTOR up, int time) {
    device_ = device;
    mesh_ = std::make_shared<Model>(mesh_path, device_);
    OutputDebugStringA("model loaded\n");
    //g buffer
    g_buffer_ = std::make_shared<GBuffer>(device_->width_, device_->height_, device_);
    OutputDebugStringA("g buffer created\n");
    Sampler_handle_ = device_->heaps_->MakeSampler();
    OutputDebugStringA("sampler made\n");
    // make root signs
    CreateGeomRootSign(mesh_->GetMaterials().size());
    OutputDebugStringA("geom root sign made\n");
    CreateLightRootSign();
    OutputDebugStringA("light root sign made\n");
    CreateInputLayout();
    cbuffer_ = std::make_shared<Cbuffer<PassConstants>>(device_);
    cbuffer_->GetData().max_lights = 0;
    FillCbuffer(cam_pos, look_at, up, time);
    ParseModelToCBuffer();
    // let there be light
    XMFLOAT4 dir = { -1,-1,0,0 };
    XMFLOAT3 str = { 1,1,1 };
    AddDirLight(dir, str);
    //str = { 1,1,1 };
    XMFLOAT4 pos = { 10,10,10,0 };
    //AddPointLight(400,200, pos, str);
    str = { 0,1,0 };
    dir = { 1,0,0,0 };
    pos = { 1100,50,-10,0 };
    //AddSpotLight(dir, 30,20, pos, str, 10);
    CreateVertexBuffer(mesh_);
    CreateIndexBuffer(mesh_);

    std::string type = "vs_5_0";
    CompileShader(L"GeomVertexShader.hlsl", geom_vertex_shader_, type);
    CompileShader(L"GeomVertexShader_anim_.hlsl", geom_vertex_shader_anim_, type);
    type = "ps_5_0";
    CompileShader(L"GeomPixelShader.hlsl", geom_pixel_shader_, type);
    OutputDebugStringA("geom shaders compiled\n");

    type = "vs_5_0";
    CompileShader(L"LightVertexShader.hlsl", light_vertex_shader_, type);
    type = "ps_5_0";
    CompileShader(L"LightPixelShader.hlsl", light_pixel_shader_, type);
    type = "ps_5_0";
    CompileShader(L"LightPixelShaderWire.hlsl", light_pixel_shader_wire_, type);

    OutputDebugStringA("light shaders compiled\n");
    type = "hs_5_0";
    CompileShader(L"HullShader.hlsl", hull_shader_, type);
    type = "ds_5_0";
    CompileShader(L"DomainShader.hlsl", domain_shader_, type);
    OutputDebugStringA("hull and domain shaders compiled\n");
    // formats of bullshit ima use as rtv// ¬ инициализации устройства (один раз):
    
    std::vector<DXGI_FORMAT> formats = {DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_B8G8R8A8_UNORM ,DXGI_FORMAT_R32_SINT };
    geom_pso_ = std::make_shared<PSO>(input_layout_, geom_vertex_shader_, geom_pixel_shader_, device_, geom_root_signature_, 3, formats);
    geom_pso_ = std::make_shared<PSO>(input_layout_, geom_vertex_shader_, geom_pixel_shader_, device_, geom_root_signature_, 3, formats);
    OutputDebugStringA("geom pso 1 made\n");
    geom_pso_tes_ = std::make_shared<PSO>(input_layout_, geom_vertex_shader_, hull_shader_, domain_shader_ , geom_pixel_shader_, device_, geom_root_signature_, 3, formats);;
    OutputDebugStringA("geom pso with tesselation made\n");
    geom_pso_anim_ = std::make_shared<PSO>(input_layout_, geom_vertex_shader_anim_, geom_pixel_shader_, device_, geom_root_signature_, 3, formats);
    OutputDebugStringA("geom pso 2 made\n");
    formats = { DXGI_FORMAT_R8G8B8A8_UNORM };
    std::vector<D3D12_INPUT_ELEMENT_DESC> input_layout;
    light_pso_ = std::make_shared<PSO>(input_layout, light_vertex_shader_, light_pixel_shader_, device_, light_root_signature_, 1, formats);
    OutputDebugStringA("light pso made\n");


}
void RenderingSystem::RenderFrame(float time, XMVECTOR look_at, XMVECTOR cam_pos, XMVECTOR up, D3D12_CPU_DESCRIPTOR_HANDLE& rtvHandle) {
    FillCbuffer(cam_pos, look_at, up, time);
    ID3D12DescriptorHeap* heaps[] = {
        device_->heaps_->GetCBV_SRV_UAV_Heap().Get(),
        device_->heaps_->GetSamplerHeap().Get()
    };
    device_->cmd_->command_list_->SetDescriptorHeaps(_countof(heaps), heaps);
    const float clearColor[4] = { 0.2f, 0.4f, 0.6f, 1.0f };
    if (first_frame_) {
        D3D12_RESOURCE_BARRIER toRenderframe[] =
        {
            Transition(g_buffer_->albedo_->texture_->GetResourse()->GetResourse().Get(),
                       D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET),
            Transition(g_buffer_->normal_->texture_->GetResourse()->GetResourse().Get(),
                       D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET),
            Transition(g_buffer_->material_index_->texture_->GetResourse()->GetResourse().Get(),
                       D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET)
        };
        device_->cmd_->command_list_.Get()->ResourceBarrier(3, toRenderframe);
        first_frame_ = false;
    }else{
        D3D12_RESOURCE_BARRIER toGeom[] =
        {
            Transition(g_buffer_->albedo_->texture_->GetResourse()->GetResourse().Get(),
                       D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET),
            Transition(g_buffer_->normal_->texture_->GetResourse()->GetResourse().Get(),
                       D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET),
            Transition(g_buffer_->material_index_->texture_->GetResourse()->GetResourse().Get(),
                       D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET),
            Transition(g_buffer_->depth_->z_buffer_->GetResourse()->GetResourse().Get(),
                       D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE)
        };
        device_->cmd_->command_list_.Get()->ResourceBarrier(4, toGeom);
    }
    GeomPass(clearColor);
    D3D12_RESOURCE_BARRIER toLight[] =
    {
        Transition(g_buffer_->albedo_->texture_->GetResourse()->GetResourse().Get(),
                   D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
        Transition(g_buffer_->normal_->texture_->GetResourse()->GetResourse().Get(),
                   D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
        Transition(g_buffer_->material_index_->texture_->GetResourse()->GetResourse().Get(),
                   D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
            Transition(g_buffer_->depth_->z_buffer_->GetResourse()->GetResourse().Get(),
                       D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
    };
    device_->cmd_->command_list_.Get()->ResourceBarrier(4, toLight);
    LightPass(clearColor, rtvHandle);

}