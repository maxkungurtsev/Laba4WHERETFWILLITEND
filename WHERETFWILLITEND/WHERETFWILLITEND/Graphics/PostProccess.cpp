#include "PostProccess.h"
void PostProccess::CompileShader(std::wstring path, ComPtr<ID3DBlob>& shader, std::string& type) {
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


PostProccess::PostProccess(std::shared_ptr<Gdevice> device, std::vector<Type>& type_array, std::vector<int>& amount_array, std::vector<D3D12_SHADER_VISIBILITY>& visibility_array, std::string& pixel_shader,
    std::vector<D3D12_INPUT_ELEMENT_DESC>& input_layout, std::vector<DXGI_FORMAT>& formats){
	// root signature
    device_ = device;
    type_array_ = type_array;
    amount_array_ = amount_array;
    visibility_array_ = visibility_array;
    if (root_sign_ == nullptr) {
        root_sign_ = std::make_shared<RootSignature>();
    }
    if (type_array.size() == amount_array.size() and visibility_array.size() == amount_array.size()) {
        parameter_amount_ = amount_array.size();
        root_sign_->AddParameter(Type::srv, 1, D3D12_SHADER_VISIBILITY_PIXEL);
        for (int i = 0; i < type_array.size(); i++) {
            root_sign_->AddParameter(type_array[i], amount_array[i], visibility_array[i]);
        }
        root_sign_->AddParameter(Type::sampler, 1, D3D12_SHADER_VISIBILITY_PIXEL);

        root_sign_->CreateRootSignature(device);
    }
    else {
        throw std::runtime_error("type, visibility and amount array sizes are NOT equal");
    }
    std::wstring ps(pixel_shader.begin(), pixel_shader.end());
    std::string type = "vs_5_0"; 
    CompileShader(L"EmptyVertexShader.hlsl", vertex_shader_, type);
    type = "ps_5_0";
    CompileShader(ps, pixel_shader_, type);
    pso_ = std::make_shared<PSO>(input_layout, vertex_shader_, pixel_shader_, device_, root_sign_, formats.size(), formats);
}

void PostProccess::ApplyPostProc(const float clearColor[4], D3D12_GPU_DESCRIPTOR_HANDLE base, D3D12_GPU_DESCRIPTOR_HANDLE sampler, std::vector<D3D12_GPU_DESCRIPTOR_HANDLE>& parameters, D3D12_CPU_DESCRIPTOR_HANDLE& rtvHandle) {

    device_->cmd_->command_list_->SetPipelineState(pso_->GetPSO().Get());
    device_->cmd_->command_list_->SetGraphicsRootSignature(root_sign_->GetRootSign().Get());
    // set & cler dsv, rtv
    //D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = g_buffer_->depth_->handle_.cpu_;
    device_->cmd_->command_list_->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
    device_->cmd_->command_list_->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    //set desc tables
    device_->cmd_->command_list_->SetGraphicsRootDescriptorTable(0, base);
    if (parameter_amount_ == parameters.size()) {
        for (int i = 0; i < parameters.size(); i++) {
            device_->cmd_->command_list_->SetGraphicsRootDescriptorTable(i+1, parameters[i]);
        }
    }
    device_->cmd_->command_list_->SetGraphicsRootDescriptorTable(parameter_amount_+1, sampler);
    device_->cmd_->command_list_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    device_->cmd_->command_list_->DrawInstanced(3, 1, 0, 0);
    
}
