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


void RenderingSystem::CreateGeomRootSign() {
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


void RenderingSystem::CreateComputeRootSign() {
    if (compute_root_signature_ == nullptr) {
        compute_root_signature_ = std::make_shared<RootSignature>();
    }

    compute_root_signature_->AddParameter(Type::cbv, 1, D3D12_SHADER_VISIBILITY_ALL);
    compute_root_signature_->AddParameter(Type::uav, 1, D3D12_SHADER_VISIBILITY_ALL); // u0 AliveIn
    compute_root_signature_->AddParameter(Type::uav, 1, D3D12_SHADER_VISIBILITY_ALL); // u1 AliveOut
    compute_root_signature_->AddParameter(Type::uav, 1, D3D12_SHADER_VISIBILITY_ALL); // u2 DeadIn
    compute_root_signature_->AddParameter(Type::uav, 1, D3D12_SHADER_VISIBILITY_ALL); // u3 DeadOut

    compute_root_signature_->CreateRootSignature(device_);
}

void RenderingSystem::CreateParticleRootSign() {
    if (particle_root_signature_ == nullptr) {
        particle_root_signature_ = std::make_shared<RootSignature>();
    }

    // b0 : ParticleRenderCB (used by VS)
    particle_root_signature_->AddParameter(Type::cbv, 1, D3D12_SHADER_VISIBILITY_ALL);

    // t0 : StructuredBuffer<Particle> Particles (used by VS)
    particle_root_signature_->AddParameter(Type::srv, 1, D3D12_SHADER_VISIBILITY_ALL);

    // t1 : Texture2D ParticleTex (used by PS)
    particle_root_signature_->AddParameter(Type::srv, 1, D3D12_SHADER_VISIBILITY_ALL);

    // s0 : sampler (used by PS)
    particle_root_signature_->AddParameter(Type::sampler, 1, D3D12_SHADER_VISIBILITY_PIXEL);

    particle_root_signature_->CreateRootSignature(device_);
}


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
    // Lights 
    light_root_signature_->AddParameter(Type::srv, 1, D3D12_SHADER_VISIBILITY_PIXEL);
    //lights amount
    light_root_signature_->AddParameter(Type::cbv, 1, D3D12_SHADER_VISIBILITY_PIXEL);

    //sampler
    light_root_signature_->AddParameter(Type::sampler, 1, D3D12_SHADER_VISIBILITY_PIXEL);
    //creating root sign with said params
    light_root_signature_->CreateRootSignature(device_);
};

void RenderingSystem::CreateComputePSO() {
    std::string type = "cs_5_0";
    CompileShader(L"ParticleComputeShader.hlsl", p_compute_shader_, type);
    compute_pso_ = std::make_shared<PSO>(device_, compute_root_signature_, p_compute_shader_);
}

void RenderingSystem::CreateParticlePSO() {
    std::vector<D3D12_INPUT_ELEMENT_DESC> input_layout ={};
    std::string type = "vs_5_0";
    CompileShader(L"ParticleVertexShader.hlsl", p_vertex_shader_, type);
    type = "ps_5_0";
    CompileShader(L"ParticlePixelShader.hlsl", p_pixel_shader_, type); 

    particle_pso_ = std::make_shared<PSO>(input_layout, p_vertex_shader_, p_pixel_shader_, device_, particle_root_signature_, 3, PSO_formats_);
}


///  C BUFFER
void RenderingSystem::FillCbuffers(XMVECTOR cam_pos, XMVECTOR look_at, XMVECTOR up, float time, XMMATRIX world) {
    //identity
    XMStoreFloat4x4(&cbuffer_->GetData().model, world);
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
    XMStoreFloat4(&cbuffer_->GetData().cam_pos, cam_pos);
    XMStoreFloat4(&cbuffer_->GetData().cam_forward, XMVector3Normalize(look_at - cam_pos));
    cbuffer_->Save_changes();
    if (emiters_.size() >0) {
        for (int i = 0; i < emiters_.size(); i++) {
            std::shared_ptr<ParticleEmiter> emiter = emiters_[i];
            XMStoreFloat4x4(&emiter->GetParticleRenderCB()->GetData().view, XMMatrixLookAtLH(cam_pos, look_at, up));
            XMStoreFloat4x4(&emiter->GetParticleRenderCB()->GetData().projection, XMMatrixPerspectiveFovLH(XM_PIDIV4, float(device_->width_) / float(device_->height_), 0.1f, 10000.0f));
            emiter->GetParticleRenderCB()->Save_changes();
        }
    }
};
void RenderingSystem::ParseModelToCBuffer(std::shared_ptr<Model> mesh) {
    for (int i = 0; i < mesh->GetMaterials().size(); i++) {
        cbuffer_->GetData().mats[i].ambient_ = mesh->GetMaterials()[i].ambient_k;
        cbuffer_->GetData().mats[i].diffuse_ = mesh->GetMaterials()[i].diffuse_k;
        cbuffer_->GetData().mats[i].spec_ = mesh->GetMaterials()[i].specular_k;
        cbuffer_->GetData().mats[i].shiny_ = mesh->GetMaterials()[i].shiny_k;
        if (mesh->GetMaterials()[i].hasHeightTexture) {
            cbuffer_->GetData().mats[i].NormalType = 2;
        }
        else if (mesh->GetMaterials()[i].hasNormTexture) {
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


// PASSES 

void RenderingSystem::SetupGeomPass(const float clearColor[4]) {
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
}


void RenderingSystem::GeomPass(std::shared_ptr<Model> mesh) {

    D3D12_INDEX_BUFFER_VIEW ibv = mesh->GetIBV();
    D3D12_VERTEX_BUFFER_VIEW vbv = mesh->GetVBV();
    device_->cmd_->command_list_->IASetVertexBuffers(0, 1, &vbv);
    device_->cmd_->command_list_->IASetIndexBuffer(&ibv);
    // culling bullshit
    std::vector<int> submeshes;

    if (!culling_enabled_ or mesh->IsBilboard()) {
        OutputDebugStringA("CULLING DISABLED\n");
            for (int i = 0; i < mesh->GetSubMeshes().size(); i++) {
                submeshes.push_back(i);
            }
    }
    else {
        BoundingFrustum frust;
        XMMATRIX invworld = XMLoadFloat4x4(&cbuffer_->GetData().inv_view);
        frustum_.Transform(frust, invworld);
        invworld = XMLoadFloat4x4(&cbuffer_->GetData().inv_model);
        frust.Transform(frust, invworld);
        mesh->GetOctree()->GetIndeciesToDraw(submeshes, frust);
        OutputDebugStringA("meshes drawn this frame: ");
        for (int i = 0; i < submeshes.size(); i++) {
            OutputDebugStringA((std::to_string(submeshes[i]) + " ").c_str());
        }
        OutputDebugStringA("others are culled\n");
    }

    // drawing cycle

    for (int i = 0; i < submeshes.size(); i++) {
        //diffuse textures/
        SubMesh submesh = mesh->GetSubMeshes()[submeshes[i]];
        cbuffer_->GetData().current_mat = submesh.materialIndex;
        cbuffer_->Save_changes();
        device_->cmd_->command_list_->SetGraphicsRootDescriptorTable(1, mesh->GetMaterials()[submesh.materialIndex].diffuseTexture->GetResourse()->GetHandle().gpu_);
        //normal textures
        device_->cmd_->command_list_->SetGraphicsRootDescriptorTable(2, mesh->GetMaterials()[submesh.materialIndex].HeightNormTexture->GetResourse()->GetHandle().gpu_);
        if (mesh->GetName() == "water.obj") {
            device_->cmd_->command_list_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
            device_->cmd_->command_list_->SetPipelineState(geom_pso_water_tes_->GetPSO().Get());
        }
        else{
            if (mesh->GetMaterials()[submesh.materialIndex].diffuseTexPath == "textures/sponza_thorn_diff.tga" or mesh->GetMaterials()[submesh.materialIndex].diffuseTexPath == "textures/vase_plant.tga")
            {
                device_->cmd_->command_list_->SetPipelineState(geom_pso_anim_->GetPSO().Get());
            }
            else {
                if (mesh->GetMaterials()[submesh.materialIndex].hasHeightTexture) {
                    device_->cmd_->command_list_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
                    device_->cmd_->command_list_->SetPipelineState(geom_pso_tes_->GetPSO().Get());
                }
                else {
                    device_->cmd_->command_list_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                    device_->cmd_->command_list_->SetPipelineState(geom_pso_->GetPSO().Get());
                }
            }
        }
        device_->cmd_->command_list_->DrawIndexedInstanced(static_cast<UINT>(submesh.indexCount), 1, static_cast<UINT>(submesh.firstIndex), 0, 0);
    }
}


void RenderingSystem::ComputePass(std::shared_ptr<ParticleEmiter> emiter) {
    emiter->UpdateCbuffer(cbuffer_->GetData().time);

    auto aliveIn = emiter->GetAliveIn();
    auto aliveOut = emiter->GetAliveOut();
    auto deadIn = emiter->GetDeadIn();
    auto deadOut = emiter->GetDeadOut();
    auto& simCB = emiter->GetParticleSimCB()->GetData();
    device_->WaitForGpu();
    // Read counters generated by the previous frame (queued before previous swap).
    // On the very first frame counters are still initialized from CPU bootstrap values.
    const int prevAliveCount = simCB.aliveInCount_;
    const int prevDeadCount = simCB.deadInCount_;
    const int maxParticles = static_cast<int>(aliveIn->GetData().size());

    aliveIn->UpdateCachedCounterFromReadback();
    deadIn->UpdateCachedCounterFromReadback();
    const int readAliveCount = static_cast<int>(aliveIn->GetCachedCounterValue());
    const int readDeadCount = static_cast<int>(deadIn->GetCachedCounterValue());

    // Readback can be stale for a frame; only accept it when pool conservation holds.
    if (readAliveCount >= 0 && readDeadCount >= 0 && (readAliveCount + readDeadCount) == maxParticles) {
        simCB.aliveInCount_ = readAliveCount;
        simCB.deadInCount_ = readDeadCount;
    }
    else {
        simCB.aliveInCount_ = max(0, min(maxParticles, prevAliveCount));
        simCB.deadInCount_ = max(0, min(maxParticles - simCB.aliveInCount_, prevDeadCount));
    }

    simCB.emitCount_ = min(simCB.emitCount_, simCB.deadInCount_);
    emiter->GetParticleSimCB()->Save_changes();
    D3D12_RESOURCE_BARRIER toUav[] = {
        Transition(aliveIn->GetResourse()->GetResourse().Get(), aliveIn->GetBaseState(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS),
        Transition(aliveOut->GetResourse()->GetResourse().Get(), aliveOut->GetBaseState(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS),
        Transition(deadIn->GetResourse()->GetResourse().Get(), deadIn->GetBaseState(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS),
        Transition(deadOut->GetResourse()->GetResourse().Get(), deadOut->GetBaseState(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
    };
    device_->cmd_->command_list_->ResourceBarrier(4, toUav);
    device_->cmd_->command_list_->SetPipelineState(compute_pso_->GetPSO().Get());
    device_->cmd_->command_list_->SetComputeRootSignature(compute_root_signature_->GetRootSign().Get());

    
    device_->cmd_->command_list_->SetComputeRootDescriptorTable(0, emiter->GetParticleSimCB()->GetHandle().gpu_);
    device_->cmd_->command_list_->SetComputeRootDescriptorTable(1, aliveIn->GetUAVHandle().gpu_);
    device_->cmd_->command_list_->SetComputeRootDescriptorTable(2, aliveOut->GetUAVHandle().gpu_);
    device_->cmd_->command_list_->SetComputeRootDescriptorTable(3, deadIn->GetUAVHandle().gpu_);
    device_->cmd_->command_list_->SetComputeRootDescriptorTable(4, deadOut->GetUAVHandle().gpu_);

    // Explicitly seed append/consume counters.
    aliveIn->SetCounterValue(static_cast<UINT>(simCB.aliveInCount_), true);
    deadIn->SetCounterValue(static_cast<UINT>(simCB.deadInCount_), true);
    aliveOut->SetCounterValue(0, true);
    deadOut->SetCounterValue(0, true);
    int workItems = max(max(simCB.aliveInCount_, simCB.deadInCount_), simCB.emitCount_);
    UINT groupCount = static_cast<UINT>((workItems + 127) / 128);
    if (groupCount > 0) {
        device_->cmd_->command_list_->Dispatch(groupCount, 1, 1);
    }

    D3D12_RESOURCE_BARRIER uavBarrier[] = {
        CD3DX12_RESOURCE_BARRIER::UAV(aliveIn->GetResourse()->GetResourse().Get()),
        CD3DX12_RESOURCE_BARRIER::UAV(aliveOut->GetResourse()->GetResourse().Get()),
        CD3DX12_RESOURCE_BARRIER::UAV(deadIn->GetResourse()->GetResourse().Get()),
        CD3DX12_RESOURCE_BARRIER::UAV(deadOut->GetResourse()->GetResourse().Get())
    };
    device_->cmd_->command_list_->ResourceBarrier(4, uavBarrier);
    aliveOut->QueueCounterReadback();
    deadOut->QueueCounterReadback();
    D3D12_RESOURCE_BARRIER toSrv[] = {
        Transition(aliveIn->GetResourse()->GetResourse().Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, aliveIn->GetBaseState()),
        Transition(aliveOut->GetResourse()->GetResourse().Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, aliveOut->GetBaseState()),
        Transition(deadIn->GetResourse()->GetResourse().Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, deadIn->GetBaseState()),
        Transition(deadOut->GetResourse()->GetResourse().Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, deadOut->GetBaseState())
    };
    device_->cmd_->command_list_->ResourceBarrier(4, toSrv);

    emiter->SwapSimulationBuffers();
}
void RenderingSystem::ParticlePass(std::shared_ptr<ParticleEmiter> emiter) {
    if (emiter == nullptr) {
        OutputDebugStringA("Emiter is null JACKASS\n");
        return;
    }
    // ComputePass swaps ping-pong buffers before we render.
    // After that swap, AliveOut points to the previous frame's stable alive buffer
    // (the one whose counter we read back at the beginning of ComputePass).
    auto renderBuffer = emiter->GetAliveOut();
    const int renderAliveCount = emiter->GetParticleSimCB()->GetData().aliveInCount_;
    //const int renderAliveCount = emiter_->GetParticleSimCB()->GetData().aliveInCount_;

    emiter->GetParticleRenderCB()->GetData().aliveCount = max(0, renderAliveCount);
    emiter->GetParticleRenderCB()->Save_changes();
    const UINT aliveCount = static_cast<UINT>(emiter->GetParticleRenderCB()->GetData().aliveCount);
    {
        auto& sim = emiter->GetParticleSimCB()->GetData();
        OutputDebugStringA(("ParticlePass drawSlots=" + std::to_string(aliveCount)
            + " simAlive=" + std::to_string(sim.aliveInCount_)
            + " simDead=" + std::to_string(sim.deadInCount_)
            + " simEmit=" + std::to_string(sim.emitCount_) + "\n").c_str());
    }
    if (aliveCount == 0) {
        return;
    }
    /*
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = g_buffer_->depth_->handle_.cpu_;
    D3D12_CPU_DESCRIPTOR_HANDLE handles[3] = {
        g_buffer_->albedo_->handle_.cpu_,
        g_buffer_->normal_->handle_.cpu_,
        g_buffer_->material_index_->handle_.cpu_
    };
    */
    device_->cmd_->command_list_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    device_->cmd_->command_list_->SetPipelineState(particle_pso_->GetPSO().Get());
    device_->cmd_->command_list_->SetGraphicsRootSignature(particle_root_signature_->GetRootSign().Get());
    //device_->cmd_->command_list_->OMSetRenderTargets(3, handles, TRUE, &dsvHandle);

    device_->cmd_->command_list_->SetGraphicsRootDescriptorTable(0, emiter->GetParticleRenderCB()->GetHandle().gpu_);
    device_->cmd_->command_list_->SetGraphicsRootDescriptorTable(1, renderBuffer->GetHandle().gpu_);
    device_->cmd_->command_list_->SetGraphicsRootDescriptorTable(2, emiter->GetTexture()->GetResourse()->GetHandle().gpu_);
    device_->cmd_->command_list_->SetGraphicsRootDescriptorTable(3, Sampler_handle_.gpu_);

    device_->cmd_->command_list_->DrawInstanced(aliveCount * 6, 1, 0, 0);
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
    device_->cmd_->command_list_->SetGraphicsRootDescriptorTable(5, light_buffer_->GetBuffer()->GetHandle().gpu_);
    device_->cmd_->command_list_->SetGraphicsRootDescriptorTable(6, light_buffer_->GetMaxLights()->GetHandle().gpu_);
    device_->cmd_->command_list_->SetGraphicsRootDescriptorTable(7, Sampler_handle_.gpu_);
    // draw
    device_->cmd_->command_list_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    device_->cmd_->command_list_->DrawInstanced(3, 1, 0, 0);
}


//INIT
RenderingSystem::RenderingSystem(std::shared_ptr<Gdevice> device, std::vector<std::string> mesh_pathes, XMVECTOR cam_pos, XMVECTOR look_at, XMVECTOR up, float time) {
    device_ = device;
        std::vector<std::string> texnames = { "jagertree.png","dark.png"};
    for (int i=0;i<texnames.size();i++){
        XMFLOAT4 emiterpos = XMFLOAT4(100*i, 0, 0, 1);
        XMFLOAT3 emiterdir = XMFLOAT3(10*i, 10, 0);
        emiters_.push_back(std::make_shared<ParticleEmiter>(texnames[i], device_, emiterpos, 0.1f, 0.0f, 100000, 100, 10, emiterdir, 0.2618f));
    }
    // initializing meshes
    for (int i = 0; i < mesh_pathes.size(); i++){
        XMFLOAT3 pos = XMFLOAT3(0, 0, 0);
        XMFLOAT3 rot = XMFLOAT3(0, 0, 0);
        XMFLOAT3 scale = XMFLOAT3(1, 1, 1);
        std::shared_ptr<Model> mesh = std::make_shared<Model>(mesh_pathes[i], device_, true, false,pos,rot,scale);
            meshes_.push_back(mesh);
        if (mesh->GetBillBoardable()) {
            OutputDebugStringA("model bilboardable\n");
            mesh->SetBilboard(std::make_shared<Model>("sponza_bilboard.obj", device_, false, true, pos, rot, scale));
        }

    }
    // making up "all submesh indices" array for octree to be based on.
    OutputDebugStringA("model loaded\n");
    OutputDebugStringA((std::to_string(meshes_.size()) + "\n").c_str());
    //g buffer
    g_buffer_ = std::make_shared<GBuffer>(device_->width_, device_->height_, device_);
    OutputDebugStringA("g buffer created\n");
    Sampler_handle_ = device_->heaps_->MakeSampler();
    OutputDebugStringA("sampler made\n");
    // make root signs
    CreateGeomRootSign();
    OutputDebugStringA("geom root sign made\n");
    CreateLightRootSign();
    OutputDebugStringA("light root sign made\n");
    CreateInputLayout();
    cbuffer_ = std::make_shared<Cbuffer<PassConstants>>(device_);
    FillCbuffers(cam_pos, look_at, up, time);
    // let there be light
    light_buffer_ = std::make_shared<Lights>(device_);
    light_buffer_->AddAmbientlight({ 0.6,0.6,0.6 }, false);
    OutputDebugStringA((std::to_string(light_buffer_->GetBuffer()->GetData()[0].type)+'\n').c_str());

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
    OutputDebugStringA("light shaders compiled\n");
    type = "hs_5_0";
    CompileShader(L"HullShader.hlsl", hull_shader_, type);
    type = "ds_5_0";
    CompileShader(L"DomainShaderWater.hlsl", water_domain_shader_, type);
    CompileShader(L"DomainShader.hlsl", domain_shader_, type);
    // particle pso, root
    CreateParticleRootSign();
    CreateParticlePSO();
    //compute pso, root
    CreateComputeRootSign();
    CreateComputePSO();
    OutputDebugStringA("hull and domain shaders compiled\n");
    // formats of bullshit ima use as rtv// ¬ инициализации устройства (один раз):
    
    geom_pso_ = std::make_shared<PSO>(input_layout_, geom_vertex_shader_, geom_pixel_shader_, device_, geom_root_signature_, 3, PSO_formats_);
    OutputDebugStringA("geom pso 1 made\n");


    geom_pso_tes_ = std::make_shared<PSO>(input_layout_, geom_vertex_shader_, hull_shader_, domain_shader_, geom_pixel_shader_, device_, geom_root_signature_, 3, PSO_formats_);
    geom_pso_water_tes_ = std::make_shared<PSO>(input_layout_, geom_vertex_shader_, hull_shader_, water_domain_shader_, geom_pixel_shader_, device_, geom_root_signature_, 3, PSO_formats_);
    OutputDebugStringA("geom pso with tesselation made\n");


    geom_pso_anim_ = std::make_shared<PSO>(input_layout_, geom_vertex_shader_anim_, geom_pixel_shader_, device_, geom_root_signature_, 3, PSO_formats_);
    OutputDebugStringA("geom pso 2 made\n");
    


    /// light pass
    std::vector<DXGI_FORMAT> format = { DXGI_FORMAT_R8G8B8A8_UNORM };
    std::vector<D3D12_INPUT_ELEMENT_DESC> input_layout;
    light_pso_ = std::make_shared<PSO>(input_layout, light_vertex_shader_, light_pixel_shader_, device_, light_root_signature_, 1, format);
    OutputDebugStringA("light pso made\n");


}
void RenderingSystem::RenderFrame(float time, XMVECTOR look_at, XMVECTOR cam_pos, XMVECTOR up, D3D12_CPU_DESCRIPTOR_HANDLE& rtvHandle, bool shootlight, bool culling_enabled) {
    const float clearColor[4] = { 0.2f, 0.4f, 0.6f, 1.0f };
    {
    if (shootlight) {
        XMFLOAT4 camera_pos;
        XMStoreFloat4(&camera_pos, cam_pos);
        XMVECTOR forward = look_at - cam_pos;
        XMFLOAT4 forw;
        XMStoreFloat4(&forw, forward);
        light_buffer_->AddPointlight({ 0.0,0.8,0.8 }, camera_pos, 100, 300, true, 50, time, forw);
    }
    culling_enabled_ = culling_enabled;

    //heaps shi
    ID3D12DescriptorHeap* heaps[] = {
        device_->heaps_->GetCBV_SRV_UAV_Heap().Get(),
        device_->heaps_->GetSamplerHeap().Get()
    };
    device_->cmd_->command_list_->SetDescriptorHeaps(_countof(heaps), heaps);

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
    }
    else {
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
    }

    SetupGeomPass(clearColor);
    for (int i = 0; i < meshes_.size(); i++) {
        //fill buffer
        std::shared_ptr<Model> mesh = meshes_[i];
        ParseModelToCBuffer(mesh);
        XMFLOAT4 distance = XMFLOAT4(mesh->GetPosition().x - cbuffer_->GetData().cam_pos.x, mesh->GetPosition().y - cbuffer_->GetData().cam_pos.y, mesh->GetPosition().z - cbuffer_->GetData().cam_pos.z, mesh->GetPosition().w - cbuffer_->GetData().cam_pos.w);
        float dist = XMVectorGetX(XMVector4Length(XMLoadFloat4(&distance)));
        // if mesh needs to be bilboarded like an idiot
        if ((mesh->GetBillBoardable() and dist > 5000) or mesh->IsBilboard()) {
            XMFLOAT4 obj = mesh->GetPosition();
            XMVECTOR objpos = XMLoadFloat4(&obj);
            XMVECTOR forward = XMVector3Normalize(cam_pos - objpos);
            XMVECTOR right = XMVector3Normalize(XMVector3Cross(up, forward));
            XMVECTOR newUp = XMVector3Normalize(XMVector3Cross(forward, right));
            XMMATRIX rotation = XMMATRIX(
                right,
                forward,
                newUp,
                XMVectorSet(0, 0, 0, 1)
            );
            XMMATRIX translation = XMMatrixTranslationFromVector(objpos);
            XMMATRIX world = rotation * translation;
            FillCbuffers(cam_pos, look_at, up, time, world);
            if (!(mesh->IsBilboard())){
                mesh = mesh->GetBilboard();
            }
        }
        else {
            XMFLOAT3 pos = XMFLOAT3(mesh->GetPosition().x, mesh->GetPosition().y, mesh->GetPosition().z);
            XMFLOAT3 rot = mesh->GetRotation();
            XMFLOAT3 sca = mesh->GetScale();
            XMMATRIX S = XMMatrixScaling(
                sca.x,
                sca.y,
                sca.z);

            XMMATRIX R = XMMatrixRotationRollPitchYaw(
                rot.x,
                rot.y,
                rot.z);

            XMMATRIX T = XMMatrixTranslation(
                pos.x,
                pos.y,
                pos.z);

            XMMATRIX world = T * R * S;
            FillCbuffers(cam_pos, look_at, up, time, world);
        }

        // make frustum
        XMMATRIX proj = XMLoadFloat4x4(&(cbuffer_->GetData().projection));
        BoundingFrustum::CreateFromMatrix(frustum_, proj);
        GeomPass(mesh);
    }
    
    if (emiters_.size() > 0) {
        for(int i=0; i<emiters_.size();i++){
            std::shared_ptr<ParticleEmiter> emiter = emiters_[i];
            cbuffer_->GetData().mats[0].ambient_ = XMFLOAT3(1.0f, 1.0f, 1.0f);
            cbuffer_->GetData().mats[0].diffuse_ = XMFLOAT3(1.0f, 1.0f, 1.0f);
            cbuffer_->GetData().mats[0].spec_ = XMFLOAT3(0.0f, 0.0f, 0.0f);
            cbuffer_->GetData().mats[0].shiny_ = 1.0f;
            FillCbuffers(cam_pos, look_at, up, time);
            ComputePass(emiter);
            /*
            OutputDebugStringA("\n");
            OutputDebugStringA((std::to_string(emiter->GetAliveIn()->GetCachedCounterValue()) + '\n').c_str());
            OutputDebugStringA((std::to_string(emiter->GetAliveOut()->GetCachedCounterValue()) + '\n').c_str());
            OutputDebugStringA((std::to_string(emiter->GetDeadIn()->GetCachedCounterValue()) + '\n').c_str());
            OutputDebugStringA((std::to_string(emiter->GetDeadOut()->GetCachedCounterValue()) + '\n').c_str());
            */
            ParticlePass(emiter);
        }
    }


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