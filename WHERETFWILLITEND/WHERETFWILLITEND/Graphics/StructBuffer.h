#pragma once
#include "GResourse.h"
/// <summary>
///  directx collisions.h
/// </summary>
/// <typeparam name="T"></typeparam>
template<typename T>
class StructBuffer {
private:
    std::shared_ptr<GResourse>  structb_;
	std::vector<T> structb_data_;
	UINT max_element_count_;
	UINT element_stride_;
    ComPtr<ID3D12Resource> upload_;
    D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc_ = {};
    D3D12_UNORDERED_ACCESS_VIEW_DESC  uav_desc_ = {};
    Handle UAV_handle;
public:	
    void SaveChanges(bool InRenderFrame) {
        if (!InRenderFrame){
            structb_->GetDevice()->cmd_->ResetAllocator();
        }
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource = structb_->GetResourse().Get();
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

        structb_->GetDevice()->cmd_->command_list_->ResourceBarrier(1, &barrier);
        void* structb_data_mapped_ = nullptr;
        D3D12_RANGE range = { 0, 0 };

        upload_->Map(0, &range, &structb_data_mapped_);
        UINT64 buffersize = static_cast<UINT64>(max_element_count_) * element_stride_; 
        memcpy(structb_data_mapped_, structb_data_.data(), static_cast<size_t>(buffersize));
        upload_->Unmap(0, nullptr);

        structb_->GetDevice()->cmd_->command_list_->CopyBufferRegion(structb_->GetResourse().Get(), 0, upload_.Get(), 0, buffersize);

        // === 5. Barrier ===
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource = structb_->GetResourse().Get();
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        if (!InRenderFrame) {
            structb_->GetDevice()->cmd_->command_list_->ResourceBarrier(1, &barrier);
            structb_->GetDevice()->cmd_->Execute();
            structb_->GetDevice()->WaitForGpu();
        }
	};

    StructBuffer(std::shared_ptr<Gdevice> device, UINT max_element_count)
        : element_stride_(sizeof(T)), max_element_count_(max_element_count)
    {
        UINT64 bufferSize = static_cast<UINT64>(max_element_count_) * element_stride_;
        structb_data_.resize(max_element_count_);

        // =========================
        // 1. GPU buffer (DEFAULT heap)
        // =========================
        D3D12_HEAP_PROPERTIES defaultHeap = {};
        defaultHeap.Type = D3D12_HEAP_TYPE_DEFAULT;
        defaultHeap.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        defaultHeap.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        defaultHeap.CreationNodeMask = 1;
        defaultHeap.VisibleNodeMask = 1;

        // GPU resource description (can be used for both SRV and UAV)
        D3D12_RESOURCE_DESC gpu_desc = {};
        gpu_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        gpu_desc.Width = bufferSize;
        gpu_desc.Height = 1;
        gpu_desc.DepthOrArraySize = 1;
        gpu_desc.MipLevels = 1;
        gpu_desc.SampleDesc.Count = 1;
        gpu_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        gpu_desc.Format = DXGI_FORMAT_UNKNOWN;
        gpu_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

        // SRV desc
        srv_desc_.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        srv_desc_.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srv_desc_.Format = DXGI_FORMAT_UNKNOWN;
        srv_desc_.Buffer.FirstElement = 0;
        srv_desc_.Buffer.NumElements = max_element_count_;
        srv_desc_.Buffer.StructureByteStride = element_stride_;
        srv_desc_.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

        std::string name = "struct buffer";

        structb_ = std::make_shared<GResourse>(
            gpu_desc,
            srv_desc_,
            defaultHeap,
            name,
            device,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
        );

        // =========================
        // 2. UAV desc + UAV creation
        // =========================
        D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc = {};
        uav_desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
        uav_desc.Format = DXGI_FORMAT_UNKNOWN;
        uav_desc.Buffer.FirstElement = 0;
        uav_desc.Buffer.NumElements = max_element_count_;
        uav_desc.Buffer.StructureByteStride = element_stride_;
        uav_desc.Buffer.CounterOffsetInBytes = 0;
        uav_desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

        // В GResourse должен быть выделен descriptor heap slot под UAV
        ComPtr<ID3D12Resource> res = structb_->GetResourse();
        UAV_handle=device->heaps_->CreateUAV_CPU(uav_desc, res);
         
        // =========================
        // 3. Upload buffer (UPLOAD heap)
        // =========================
        D3D12_HEAP_PROPERTIES uploadHeap = {};
        uploadHeap.Type = D3D12_HEAP_TYPE_UPLOAD;
        uploadHeap.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        uploadHeap.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        uploadHeap.CreationNodeMask = 1;
        uploadHeap.VisibleNodeMask = 1;

        D3D12_RESOURCE_DESC upload_desc = gpu_desc;
        upload_desc.Flags = D3D12_RESOURCE_FLAG_NONE; // важно: без UAV

        HRESULT hr = device->GetDXDevice()->CreateCommittedResource(
            &uploadHeap,
            D3D12_HEAP_FLAG_NONE,
            &upload_desc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&upload_)
        );

        if (FAILED(hr)) {
            throw std::runtime_error("Failed to create upload buffer");
        }
    }

    /*

	StructBuffer(std::shared_ptr<Gdevice> device, UINT max_element_count):element_stride_(sizeof(T)), max_element_count_(max_element_count) {
        UINT64 bufferSize = static_cast<UINT64>(max_element_count_) * element_stride_;
        structb_data_.resize(max_element_count_);
        // === 1. GPU buffer (DEFAULT heap) ===
        D3D12_HEAP_PROPERTIES defaultHeap = {};
        defaultHeap.Type = D3D12_HEAP_TYPE_DEFAULT;

        //res_desc
        // for srv
        D3D12_RESOURCE_DESC res_desc = {};
        res_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        res_desc.Width = bufferSize;
        res_desc.Height = 1;
        res_desc.DepthOrArraySize = 1;
        res_desc.MipLevels = 1;
        res_desc.SampleDesc.Count = 1;
        res_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        res_desc.Format = DXGI_FORMAT_UNKNOWN;
        res_desc.Flags = D3D12_RESOURCE_FLAG_NONE;
        // for uav
        D3D12_RESOURCE_DESC uav_res_desc = res_desc;
        uav_res_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        //srv desc
        srv_desc_.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        srv_desc_.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srv_desc_.Format = DXGI_FORMAT_UNKNOWN;
        srv_desc_.Buffer.FirstElement = 0;
        srv_desc_.Buffer.NumElements = max_element_count_;
        srv_desc_.Buffer.StructureByteStride = element_stride_;
        srv_desc_.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
        // upload heap
        D3D12_HEAP_PROPERTIES heapProps = {};
        heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
        heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapProps.CreationNodeMask = 1;
        heapProps.VisibleNodeMask = 1;
        std::string name = "struct buffer";
        structb_ = std::make_shared<GResourse>(res_desc, srv_desc_, heapProps, name, device, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        

        // === 2. Upload buffer ===
        D3D12_HEAP_PROPERTIES uploadHeap = {};
        uploadHeap.Type = D3D12_HEAP_TYPE_UPLOAD;
        HRESULT hr = structb_->GetDevice()->GetDXDevice()->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &res_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&upload_));
        if (FAILED(hr)){
                throw std::runtime_error("Failed to create upload buffer");
        }

	};

    */

    std::vector<T>& GetData() {
		return structb_data_;
	};


	Handle GetHandle() {
		return structb_->GetHandle();
	};

    Handle GetUAVHandle() {
        return UAV_handle;
    }
    void AddElement(T& element, int max_elements, bool in_render_frame) {
        if (max_element_count_ > max_elements) {
            structb_data_[max_elements]= element;
            SaveChanges(in_render_frame);
        }
        else {
            OutputDebugStringA("struct buffer limit Reached!");
        }
    }
    void RemoveLastElement(bool in_render_frame) {
        if (0 < structb_data_.size()) {
            structb_data_.pop_back();
            SaveChanges(in_render_frame);
        }
        else {
            OutputDebugStringA("struct buffer is empty!");
        }
    }
};