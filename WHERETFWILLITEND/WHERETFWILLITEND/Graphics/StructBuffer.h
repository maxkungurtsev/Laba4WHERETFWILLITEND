#pragma once
#include "GResourse.h"
/// <summary>
///  directx collisions.h
/// </summary>
/// <typeparam name="T"></typeparam>
template<typename T>
class StructBuffer {
private:
    static constexpr UINT64 kCounterBufferSize = 4096; // D3D12 UAV counter alignment
    std::shared_ptr<GResourse>  structb_;
	std::vector<T> structb_data_;
	UINT max_element_count_;
	UINT element_stride_;
    ComPtr<ID3D12Resource> upload_;
    ComPtr<ID3D12Resource> counter_resource_;
    ComPtr<ID3D12Resource> counter_upload_;
    ComPtr<ID3D12Resource> counter_readback_;
    UINT cached_counter_value_ = 0;
    D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc_ = {};
    D3D12_UNORDERED_ACCESS_VIEW_DESC  uav_desc_ = {};
    D3D12_RESOURCE_STATES base_state_;
    Handle UAV_handle;
    void WriteCounterUpload(UINT value) {
        void* mapped = nullptr;
        D3D12_RANGE range = { 0, 0 };
        counter_upload_->Map(0, &range, &mapped);
        *reinterpret_cast<UINT*>(mapped) = value;
        counter_upload_->Unmap(0, nullptr);
    }
public:	
    void SaveChanges(bool InRenderFrame) {
        if (!InRenderFrame){
            structb_->GetDevice()->cmd_->ResetAllocator();
        }
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource = structb_->GetResourse().Get();
        barrier.Transition.StateBefore = base_state_;
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
        barrier.Transition.StateAfter = base_state_;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        if (!InRenderFrame) {
            structb_->GetDevice()->cmd_->command_list_->ResourceBarrier(1, &barrier);
            structb_->GetDevice()->cmd_->Execute();
            structb_->GetDevice()->WaitForGpu();
        }
	};
    void SetCounterValue(UINT value, bool InRenderFrame) {
        if (!InRenderFrame) {
            structb_->GetDevice()->cmd_->ResetAllocator();
        }

        cached_counter_value_ = value;
        WriteCounterUpload(value);

        auto cmdList = structb_->GetDevice()->cmd_->command_list_;

        D3D12_RESOURCE_BARRIER toCopy = {};
        toCopy.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        toCopy.Transition.pResource = counter_resource_.Get();
        toCopy.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        toCopy.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
        toCopy.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        cmdList->ResourceBarrier(1, &toCopy);

        cmdList->CopyBufferRegion(counter_resource_.Get(), 0, counter_upload_.Get(), 0, sizeof(UINT));

        D3D12_RESOURCE_BARRIER toUav = {};
        toUav.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        toUav.Transition.pResource = counter_resource_.Get();
        toUav.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        toUav.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        toUav.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        cmdList->ResourceBarrier(1, &toUav);

        if (!InRenderFrame) {
            structb_->GetDevice()->cmd_->Execute();
            structb_->GetDevice()->WaitForGpu();
        }
    }

    void QueueCounterReadback() {
        auto cmdList = structb_->GetDevice()->cmd_->command_list_;

        D3D12_RESOURCE_BARRIER toCopy = {};
        toCopy.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        toCopy.Transition.pResource = counter_resource_.Get();
        toCopy.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        toCopy.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
        toCopy.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        cmdList->ResourceBarrier(1, &toCopy);

        cmdList->CopyBufferRegion(counter_readback_.Get(), 0, counter_resource_.Get(), 0, sizeof(UINT));

        D3D12_RESOURCE_BARRIER toUav = {};
        toUav.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        toUav.Transition.pResource = counter_resource_.Get();
        toUav.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
        toUav.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        toUav.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        cmdList->ResourceBarrier(1, &toUav);
    }

    void UpdateCachedCounterFromReadback() {
        void* mapped = nullptr;
        D3D12_RANGE range = { 0, sizeof(UINT) };
        counter_readback_->Map(0, &range, &mapped);
        cached_counter_value_ = *reinterpret_cast<UINT*>(mapped);
        counter_readback_->Unmap(0, nullptr);
    }

    UINT GetCachedCounterValue() const {
        return cached_counter_value_;
    }
    StructBuffer(std::shared_ptr<Gdevice> device, UINT max_element_count, D3D12_RESOURCE_STATES base_state)
        : element_stride_(sizeof(T)), max_element_count_(max_element_count), base_state_(base_state)
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
            base_state_
        );

        // create counter resourse
        D3D12_RESOURCE_DESC counter_desc = {};
        counter_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        counter_desc.Width = kCounterBufferSize;
        counter_desc.Height = 1;
        counter_desc.DepthOrArraySize = 1;
        counter_desc.MipLevels = 1;
        counter_desc.SampleDesc.Count = 1;
        counter_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        counter_desc.Format = DXGI_FORMAT_UNKNOWN;
        counter_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

        HRESULT hr = device->GetDXDevice()->CreateCommittedResource(
            &defaultHeap,
            D3D12_HEAP_FLAG_NONE,
            &counter_desc,
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
            nullptr,
            IID_PPV_ARGS(&counter_resource_)
        );
        if (FAILED(hr)) {
            throw std::runtime_error("Failed to create counter resource");
        }
        //create UAV HANDLE
        uav_desc_.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
        uav_desc_.Format = DXGI_FORMAT_UNKNOWN;
        uav_desc_.Buffer.FirstElement = 0;
        uav_desc_.Buffer.NumElements = max_element_count_;
        uav_desc_.Buffer.StructureByteStride = element_stride_;
        uav_desc_.Buffer.CounterOffsetInBytes = 0;
        uav_desc_.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
        ComPtr<ID3D12Resource> structbuffer = structb_->GetResourse();
        UAV_handle = device->heaps_->CreateUAV_CPU(uav_desc_, structbuffer, counter_resource_);
         
        // =========================
        // 3. Upload buffer (UPLOAD heap)
        // =========================
        D3D12_HEAP_PROPERTIES uploadHeap = {};
        uploadHeap.Type = D3D12_HEAP_TYPE_UPLOAD;
        uploadHeap.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        uploadHeap.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        uploadHeap.CreationNodeMask = 1;
        uploadHeap.VisibleNodeMask = 1;

        D3D12_HEAP_PROPERTIES readbackHeap = {};
        readbackHeap.Type = D3D12_HEAP_TYPE_READBACK;
        readbackHeap.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        readbackHeap.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        readbackHeap.CreationNodeMask = 1;
        readbackHeap.VisibleNodeMask = 1;

        D3D12_RESOURCE_DESC upload_desc = gpu_desc;
        upload_desc.Flags = D3D12_RESOURCE_FLAG_NONE; // важно: без UAV

        hr = device->GetDXDevice()->CreateCommittedResource(
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

        D3D12_RESOURCE_DESC counter_upload_desc = {};
        counter_upload_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        counter_upload_desc.Width = kCounterBufferSize;
        counter_upload_desc.Height = 1;
        counter_upload_desc.DepthOrArraySize = 1;
        counter_upload_desc.MipLevels = 1;
        counter_upload_desc.SampleDesc.Count = 1;
        counter_upload_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        counter_upload_desc.Format = DXGI_FORMAT_UNKNOWN;
        counter_upload_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

        hr = device->GetDXDevice()->CreateCommittedResource(
            &uploadHeap,
            D3D12_HEAP_FLAG_NONE,
            &counter_upload_desc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&counter_upload_)
        );
        if (FAILED(hr)) {
            throw std::runtime_error("Failed to create counter upload buffer");
        }

        hr = device->GetDXDevice()->CreateCommittedResource(
            &readbackHeap,
            D3D12_HEAP_FLAG_NONE,
            &counter_upload_desc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&counter_readback_)
        );
        if (FAILED(hr)) {
            throw std::runtime_error("Failed to create counter readback buffer");
        }
    }

    std::vector<T>& GetData() {
		return structb_data_;
	};

    std::shared_ptr<GResourse> GetResourse() {
        return structb_;
    }

	Handle GetHandle() {
		return structb_->GetHandle();
	};

    D3D12_RESOURCE_STATES GetBaseState() { return base_state_; }
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