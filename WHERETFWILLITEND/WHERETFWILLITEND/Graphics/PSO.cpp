#include "PSO.h"
#include <sstream>
PSO::PSO(std::vector<D3D12_INPUT_ELEMENT_DESC> input_layout, ComPtr<ID3DBlob> vertex_shader, ComPtr<ID3DBlob> pixel_shader, std::shared_ptr<Gdevice> device, std::shared_ptr<RootSignature> root_sign, int rtv_amount, std::vector<DXGI_FORMAT> formats) {
    device_ = device;
    vertex_shader_ = vertex_shader;
    pixel_shader_ = pixel_shader;
    input_layout_ = input_layout;
    ComPtr<ID3DBlob> errorBlob;
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = rtv_amount;
    for (int i = 0; i < rtv_amount; i++) {
        psoDesc.RTVFormats[i] = formats[i];
    }

    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    psoDesc.SampleDesc.Count = device->sample_amount_;
    psoDesc.SampleDesc.Quality = device->msaa_quality_;
    psoDesc.InputLayout = { input_layout_.data(), (UINT)input_layout_.size() };
    psoDesc.pRootSignature = root_sign->GetRootSign().Get();
    psoDesc.VS = { vertex_shader_->GetBufferPointer(), vertex_shader_->GetBufferSize() };
    psoDesc.PS = { pixel_shader_->GetBufferPointer(), pixel_shader_->GetBufferSize() };
    psoDesc.BlendState = CD3DX12_BLEND_DESC();
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC();
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC();


    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
    psoDesc.RasterizerState.DepthClipEnable = TRUE;
    psoDesc.BlendState.AlphaToCoverageEnable = FALSE;
    psoDesc.BlendState.IndependentBlendEnable = FALSE;
    const D3D12_RENDER_TARGET_BLEND_DESC defaultBlend = { FALSE,FALSE,
        D3D12_BLEND_ONE,D3D12_BLEND_ZERO,D3D12_BLEND_OP_ADD,
        D3D12_BLEND_ONE,D3D12_BLEND_ZERO,D3D12_BLEND_OP_ADD,
        D3D12_LOGIC_OP_NOOP,
        D3D12_COLOR_WRITE_ENABLE_ALL
    };
    for (int i = 0; i < 8; ++i) {
        psoDesc.BlendState.RenderTarget[i] = defaultBlend;
    }
    psoDesc.DepthStencilState.DepthEnable = TRUE;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    
    
    //RasterizerState
    D3D12_RASTERIZER_DESC rasterDesc{};
    rasterDesc.FillMode = D3D12_FILL_MODE_SOLID;
    rasterDesc.CullMode = D3D12_CULL_MODE_NONE;
    rasterDesc.FrontCounterClockwise = FALSE;
    rasterDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    rasterDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    rasterDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    rasterDesc.DepthClipEnable = TRUE;
    rasterDesc.MultisampleEnable = FALSE;
    rasterDesc.AntialiasedLineEnable = FALSE;
    rasterDesc.ForcedSampleCount = 0;
    rasterDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
    psoDesc.RasterizerState = rasterDesc;
    //blend state
    D3D12_BLEND_DESC blendDesc{};
    blendDesc.AlphaToCoverageEnable = FALSE;
    blendDesc.IndependentBlendEnable = FALSE;
    D3D12_RENDER_TARGET_BLEND_DESC rtBlend{};
    rtBlend.BlendEnable = FALSE;
    rtBlend.LogicOpEnable = FALSE;
    rtBlend.SrcBlend = D3D12_BLEND_ONE;
    rtBlend.DestBlend = D3D12_BLEND_ZERO;
    rtBlend.BlendOp = D3D12_BLEND_OP_ADD;
    rtBlend.SrcBlendAlpha = D3D12_BLEND_ONE;
    rtBlend.DestBlendAlpha = D3D12_BLEND_ZERO;
    rtBlend.BlendOpAlpha = D3D12_BLEND_OP_ADD;
    rtBlend.LogicOp = D3D12_LOGIC_OP_NOOP;
    rtBlend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    for (int i = 0; i < 8; ++i) {
        blendDesc.RenderTarget[i] = rtBlend;
    }
    psoDesc.BlendState = blendDesc;
    //DepthStencilState
    D3D12_DEPTH_STENCIL_DESC depthDesc{};
    depthDesc.DepthEnable = TRUE;
    depthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    depthDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    depthDesc.StencilEnable = FALSE;
    depthDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
    depthDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
    depthDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    depthDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    depthDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    depthDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    depthDesc.BackFace = depthDesc.FrontFace;
    psoDesc.DepthStencilState = depthDesc;
    // D E B U G  T I M E
    if (!root_sign->GetRootSign()) OutputDebugStringA("root_signature_ == null\n");
    if (!vertex_shader_) OutputDebugStringA("vertex_shader_ == null\n");
    if (!pixel_shader_) OutputDebugStringA("pixel_shader_ == null\n");
    {
        std::ostringstream ss;
        ss << "VS size: " << (vertex_shader_ ? vertex_shader_->GetBufferSize() : 0)
           << ", PS size: " << (pixel_shader_ ? pixel_shader_->GetBufferSize() : 0) << "\n";
        OutputDebugStringA(ss.str().c_str());
    }
    if (input_layout_.empty()) OutputDebugStringA("input_layout_ empty\n");
    {
        std::ostringstream s2;
        s2 << "RTVFormat: " << psoDesc.RTVFormats[0] << " DSVFormat: " << psoDesc.DSVFormat << " SampleCount: " << psoDesc.SampleDesc.Count << "\n";
        OutputDebugStringA(s2.str().c_str());
    }
    std::ostringstream oss;
    oss << "Creating PSO with parameters:\n";
    oss << "NumRenderTargets: " << psoDesc.NumRenderTargets << "\n";
    oss << "RTVFormats[0]: " << psoDesc.RTVFormats[0] << "\n";
    oss << "DSVFormat: " << psoDesc.DSVFormat << "\n";
    oss << "SampleCount: " << psoDesc.SampleDesc.Count << "\n";
    oss << "InputLayout.Elements: " << psoDesc.InputLayout.NumElements << "\n";
    oss << "RootSignature: " << (psoDesc.pRootSignature ? "valid" : "nullptr") << "\n";
    oss << "VS Size: " << psoDesc.VS.BytecodeLength << "\n";
    oss << "PS Size: " << psoDesc.PS.BytecodeLength << "\n";
    OutputDebugStringA(oss.str().c_str());
    HRESULT hr = device_->GetDXDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipeline_state_));
    if (FAILED(hr)) {
        std::ostringstream oss2;
        oss2 << "CreateGraphicsPipelineState failed. HRESULT = 0x" << std::hex << hr << "\n";
        OutputDebugStringA(oss2.str().c_str());
        throw std::runtime_error(oss2.str());
    }
};

PSO::PSO(std::vector<D3D12_INPUT_ELEMENT_DESC> input_layout, ComPtr<ID3DBlob> vertex_shader, ComPtr<ID3DBlob> hull_shader, ComPtr<ID3DBlob> domain_shader, ComPtr<ID3DBlob> pixel_shader, std::shared_ptr<Gdevice> device, std::shared_ptr<RootSignature> root_sign, int rtv_amount, std::vector<DXGI_FORMAT> formats) 
{
    device_ = device;
    vertex_shader_ = vertex_shader;
    pixel_shader_ = pixel_shader;
    input_layout_ = input_layout;
    hull_shader_ = hull_shader;
    domain_shader_ = domain_shader;
      
    ComPtr<ID3DBlob> errorBlob;
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
    psoDesc.NumRenderTargets = rtv_amount;
    for (int i = 0; i < rtv_amount; i++) {
        psoDesc.RTVFormats[i] = formats[i];
    }

    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    psoDesc.SampleDesc.Count = device->sample_amount_;
    psoDesc.SampleDesc.Quality = device->msaa_quality_;
    psoDesc.InputLayout = { input_layout_.data(), (UINT)input_layout_.size() };
    psoDesc.pRootSignature = root_sign->GetRootSign().Get();
    psoDesc.VS = { vertex_shader_->GetBufferPointer(), vertex_shader_->GetBufferSize() };
    psoDesc.PS = { pixel_shader_->GetBufferPointer(), pixel_shader_->GetBufferSize() };
    psoDesc.HS = { hull_shader_->GetBufferPointer(), hull_shader_->GetBufferSize() };
    psoDesc.DS = { domain_shader_->GetBufferPointer(), domain_shader_->GetBufferSize() };
    psoDesc.BlendState = CD3DX12_BLEND_DESC();
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC();
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC();


    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
    psoDesc.RasterizerState.DepthClipEnable = TRUE;
    psoDesc.BlendState.AlphaToCoverageEnable = FALSE;
    psoDesc.BlendState.IndependentBlendEnable = FALSE;
    const D3D12_RENDER_TARGET_BLEND_DESC defaultBlend = { FALSE,FALSE,
        D3D12_BLEND_ONE,D3D12_BLEND_ZERO,D3D12_BLEND_OP_ADD,
        D3D12_BLEND_ONE,D3D12_BLEND_ZERO,D3D12_BLEND_OP_ADD,
        D3D12_LOGIC_OP_NOOP,
        D3D12_COLOR_WRITE_ENABLE_ALL
    };
    for (int i = 0; i < 8; ++i) {
        psoDesc.BlendState.RenderTarget[i] = defaultBlend;
    }
    psoDesc.DepthStencilState.DepthEnable = TRUE;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    psoDesc.DepthStencilState.StencilEnable = FALSE;


    //RasterizerState
    D3D12_RASTERIZER_DESC rasterDesc{};
    rasterDesc.FillMode = D3D12_FILL_MODE_SOLID;
    rasterDesc.CullMode = D3D12_CULL_MODE_NONE;
    rasterDesc.FrontCounterClockwise = FALSE;
    rasterDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    rasterDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    rasterDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    rasterDesc.DepthClipEnable = TRUE;
    rasterDesc.MultisampleEnable = FALSE;
    rasterDesc.AntialiasedLineEnable = FALSE;
    rasterDesc.ForcedSampleCount = 0;
    rasterDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
    psoDesc.RasterizerState = rasterDesc;
    //blend state
    D3D12_BLEND_DESC blendDesc{};
    blendDesc.AlphaToCoverageEnable = FALSE;
    blendDesc.IndependentBlendEnable = FALSE;
    D3D12_RENDER_TARGET_BLEND_DESC rtBlend{};
    rtBlend.BlendEnable = FALSE;
    rtBlend.LogicOpEnable = FALSE;
    rtBlend.SrcBlend = D3D12_BLEND_ONE;
    rtBlend.DestBlend = D3D12_BLEND_ZERO;
    rtBlend.BlendOp = D3D12_BLEND_OP_ADD;
    rtBlend.SrcBlendAlpha = D3D12_BLEND_ONE;
    rtBlend.DestBlendAlpha = D3D12_BLEND_ZERO;
    rtBlend.BlendOpAlpha = D3D12_BLEND_OP_ADD;
    rtBlend.LogicOp = D3D12_LOGIC_OP_NOOP;
    rtBlend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    for (int i = 0; i < 8; ++i) {
        blendDesc.RenderTarget[i] = rtBlend;
    }
    psoDesc.BlendState = blendDesc;
    //DepthStencilState
    D3D12_DEPTH_STENCIL_DESC depthDesc{};
    depthDesc.DepthEnable = TRUE;
    depthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    depthDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    depthDesc.StencilEnable = FALSE;
    depthDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
    depthDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
    depthDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    depthDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    depthDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    depthDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    depthDesc.BackFace = depthDesc.FrontFace;
    psoDesc.DepthStencilState = depthDesc;
    // D E B U G  T I M E
    if (!root_sign->GetRootSign()) OutputDebugStringA("root_signature_ == null\n");
    if (!vertex_shader_) OutputDebugStringA("vertex_shader_ == null\n");
    if (!pixel_shader_) OutputDebugStringA("pixel_shader_ == null\n");
    {
        std::ostringstream ss;
        ss << "VS size: " << (vertex_shader_ ? vertex_shader_->GetBufferSize() : 0)
            << ", PS size: " << (pixel_shader_ ? pixel_shader_->GetBufferSize() : 0) << "\n";
        OutputDebugStringA(ss.str().c_str());
    }
    if (input_layout_.empty()) OutputDebugStringA("input_layout_ empty\n");
    {
        std::ostringstream s2;
        s2 << "RTVFormat: " << psoDesc.RTVFormats[0] << " DSVFormat: " << psoDesc.DSVFormat << " SampleCount: " << psoDesc.SampleDesc.Count << "\n";
        OutputDebugStringA(s2.str().c_str());
    }
    std::ostringstream oss;
    oss << "Creating PSO with parameters:\n";
    oss << "NumRenderTargets: " << psoDesc.NumRenderTargets << "\n";
    oss << "RTVFormats[0]: " << psoDesc.RTVFormats[0] << "\n";
    oss << "DSVFormat: " << psoDesc.DSVFormat << "\n";
    oss << "SampleCount: " << psoDesc.SampleDesc.Count << "\n";
    oss << "InputLayout.Elements: " << psoDesc.InputLayout.NumElements << "\n";
    oss << "RootSignature: " << (psoDesc.pRootSignature ? "valid" : "nullptr") << "\n";
    oss << "VS Size: " << psoDesc.VS.BytecodeLength << "\n";
    oss << "PS Size: " << psoDesc.PS.BytecodeLength << "\n";
    OutputDebugStringA(oss.str().c_str());
    HRESULT hr = device_->GetDXDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipeline_state_));
    if (FAILED(hr)) {
        std::ostringstream oss2;
        oss2 << "CreateGraphicsPipelineState failed. HRESULT = 0x" << std::hex << hr << "\n";
        OutputDebugStringA(oss2.str().c_str());
        throw std::runtime_error(oss2.str());
    }


    //ComPtr<ID3DBlob> errorBlob;
    //D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    //psoDesc.InputLayout = { input_layout_.data(), (UINT)input_layout_.size() };
    //psoDesc.pRootSignature = root_sign->GetRootSign().Get();
    //psoDesc.VS = { vertex_shader_->GetBufferPointer(), vertex_shader_->GetBufferSize() };
    //psoDesc.PS = { pixel_shader_->GetBufferPointer(), pixel_shader_->GetBufferSize() };
    //psoDesc.HS = { hull_shader_->GetBufferPointer(), hull_shader_->GetBufferSize() };
    //psoDesc.DS = { domain_shader_->GetBufferPointer(), domain_shader_->GetBufferSize() };

    //psoDesc.SampleMask = UINT_MAX;
    //psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
    //psoDesc.NumRenderTargets = rtv_amount;
    //for (int i = 0; i < rtv_amount; i++) {
    //    psoDesc.RTVFormats[i] = formats[i];
    //}
    //psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    //psoDesc.SampleDesc.Count = device->sample_amount_;
    //psoDesc.SampleDesc.Quality = device->msaa_quality_;

    //psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    //psoDesc.SampleDesc.Count = device->sample_amount_;
    //psoDesc.SampleDesc.Quality = device->msaa_quality_;
    //psoDesc.BlendState = CD3DX12_BLEND_DESC();
    //psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC();
    //psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC();

    ///*psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    //psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
    //psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
    //psoDesc.RasterizerState.DepthClipEnable = TRUE;
    //psoDesc.BlendState.AlphaToCoverageEnable = FALSE;
    //psoDesc.BlendState.IndependentBlendEnable = FALSE;
    //const D3D12_RENDER_TARGET_BLEND_DESC defaultBlend = { FALSE,FALSE,
    //    D3D12_BLEND_ONE,D3D12_BLEND_ZERO,D3D12_BLEND_OP_ADD,
    //    D3D12_BLEND_ONE,D3D12_BLEND_ZERO,D3D12_BLEND_OP_ADD,
    //    D3D12_LOGIC_OP_NOOP,
    //    D3D12_COLOR_WRITE_ENABLE_ALL
    //};
    //for (int i = 0; i < 8; ++i) {
    //    psoDesc.BlendState.RenderTarget[i] = defaultBlend;
    //}
    //psoDesc.DepthStencilState.DepthEnable = TRUE;
    //psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    //psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    //psoDesc.DepthStencilState.StencilEnable = FALSE;*/

    ////RasterizerState
    ////D3D12_RASTERIZER_DESC rasterDesc{};
    ////rasterDesc.FillMode = D3D12_FILL_MODE_SOLID;
    ////rasterDesc.CullMode = D3D12_CULL_MODE_BACK;
    ////rasterDesc.FrontCounterClockwise = FALSE;
    ////rasterDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    ////rasterDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    ////rasterDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    ////rasterDesc.DepthClipEnable = TRUE;
    ////rasterDesc.MultisampleEnable = FALSE;
    ////rasterDesc.AntialiasedLineEnable = FALSE;
    ////rasterDesc.ForcedSampleCount = 0;
    ////rasterDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
    ////psoDesc.RasterizerState = rasterDesc;
    //////blend state
    ////D3D12_BLEND_DESC blendDesc{};
    ////blendDesc.AlphaToCoverageEnable = FALSE;
    ////blendDesc.IndependentBlendEnable = FALSE;
    ////D3D12_RENDER_TARGET_BLEND_DESC rtBlend{};
    ////rtBlend.BlendEnable = FALSE;
    ////rtBlend.LogicOpEnable = FALSE;
    ////rtBlend.SrcBlend = D3D12_BLEND_ONE;
    ////rtBlend.DestBlend = D3D12_BLEND_ZERO;
    ////rtBlend.BlendOp = D3D12_BLEND_OP_ADD;
    ////rtBlend.SrcBlendAlpha = D3D12_BLEND_ONE;
    ////rtBlend.DestBlendAlpha = D3D12_BLEND_ZERO;
    ////rtBlend.BlendOpAlpha = D3D12_BLEND_OP_ADD;
    ////rtBlend.LogicOp = D3D12_LOGIC_OP_NOOP;
    ////rtBlend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    ////for (int i = 0; i < 8; ++i) {
    ////    blendDesc.RenderTarget[i] = rtBlend;
    ////}
    ////psoDesc.BlendState = blendDesc;
    //////DepthStencilState
    ////D3D12_DEPTH_STENCIL_DESC depthDesc{};
    ////depthDesc.DepthEnable = TRUE;
    ////depthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    ////depthDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    ////depthDesc.StencilEnable = FALSE;
    ////depthDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
    ////depthDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
    ////depthDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    ////depthDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    ////depthDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    ////depthDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    ////depthDesc.BackFace = depthDesc.FrontFace;
    ////psoDesc.DepthStencilState = depthDesc;
    //// D E B U G  T I M E
    //if (!root_sign->GetRootSign()) OutputDebugStringA("root_signature_ == null\n");
    //if (!vertex_shader_) OutputDebugStringA("vertex_shader_ == null\n");
    //if (!pixel_shader_) OutputDebugStringA("pixel_shader_ == null\n");
    //{
    //    std::ostringstream ss;
    //    ss << "Shader sizes: VS=" << (vertex_shader_ ? vertex_shader_->GetBufferSize() : 0)
    //        << ", HS=" << (hull_shader_ ? hull_shader_->GetBufferSize() : 0)
    //        << ", DS=" << (domain_shader_ ? domain_shader_->GetBufferSize() : 0)
    //        << ", PS=" << (pixel_shader_ ? pixel_shader_->GetBufferSize() : 0) << "\n";
    //    OutputDebugStringA(ss.str().c_str());
    //}
    //if (input_layout_.empty()) OutputDebugStringA("input_layout_ empty\n");
    //{
    //    std::ostringstream s2;
    //    s2 << "RTVFormat: " << psoDesc.RTVFormats[0] << " DSVFormat: " << psoDesc.DSVFormat << " SampleCount: " << psoDesc.SampleDesc.Count << "\n";
    //    OutputDebugStringA(s2.str().c_str());
    //}
    //std::ostringstream oss;
    //oss << "Creating PSO with parameters:\n";
    //oss << "NumRenderTargets: " << psoDesc.NumRenderTargets << "\n";
    //oss << "RTVFormats[0]: " << psoDesc.RTVFormats[0] << "\n";
    //oss << "DSVFormat: " << psoDesc.DSVFormat << "\n";
    //oss << "SampleCount: " << psoDesc.SampleDesc.Count << "\n";
    //oss << "InputLayout.Elements: " << psoDesc.InputLayout.NumElements << "\n";
    //oss << "RootSignature: " << (psoDesc.pRootSignature ? "valid" : "nullptr") << "\n";
    //oss << "VS Size: " << psoDesc.VS.BytecodeLength << "\n";
    //oss << "PS Size: " << psoDesc.PS.BytecodeLength << "\n";
    //oss << "HS Size: " << psoDesc.HS.BytecodeLength << "\n";
    //oss << "DS Size: " << psoDesc.DS.BytecodeLength << "\n";
    //OutputDebugStringA(oss.str().c_str());
    //HRESULT hr = device_->GetDXDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipeline_state_));
    //if (FAILED(hr)) {
    //    std::ostringstream oss2;
    //    oss2 << "CreateGraphicsPipelineState failed. HRESULT = 0x" << std::hex << hr << "\n";
    //    OutputDebugStringA(oss2.str().c_str());
    //    throw std::runtime_error(oss2.str());
    //}
};
ComPtr<ID3D12PipelineState> PSO::GetPSO() {
    return pipeline_state_;
};