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
public:	
    void SaveChanges() {
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource = structb_->GetResourse().Get();
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
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
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

        structb_->GetDevice()->cmd_->command_list_->ResourceBarrier(1, &barrier);
	};
	StructBuffer(std::shared_ptr<Gdevice> device, UINT max_element_count):element_stride_(sizeof(T)), max_element_count_(max_element_count) {
        UINT64 bufferSize = static_cast<UINT64>(max_element_count_) * element_stride_;

        // === 1. GPU buffer (DEFAULT heap) ===
        D3D12_HEAP_PROPERTIES defaultHeap = {};
        defaultHeap.Type = D3D12_HEAP_TYPE_DEFAULT;

        //res_desc
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
        //srv desc
        srv_desc_.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        srv_desc_.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srv_desc_.Format = DXGI_FORMAT_UNKNOWN;
        srv_desc_.Buffer.FirstElement = 0;
        srv_desc_.Buffer.NumElements = max_element_count_;
        srv_desc_.Buffer.StructureByteStride = element_stride_;
        srv_desc_.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

        D3D12_HEAP_PROPERTIES heapProps = {};
        heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
        heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapProps.CreationNodeMask = 1;
        heapProps.VisibleNodeMask = 1;
        std::string name = "struct buffer";
        structb_ = std::make_shared<GResourse>(res_desc, srv_desc_, heapProps, name, device, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        

        // === 2. Upload buffer ===
        D3D12_HEAP_PROPERTIES uploadHeap = {};
        uploadHeap.Type = D3D12_HEAP_TYPE_UPLOAD;
        HRESULT hr = structb_->GetDevice()->GetDXDevice()->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &res_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&upload_));
        if (FAILED(hr)){
                throw std::runtime_error("Failed to create upload buffer");
        }

	};
    std::vector<T>& GetData() {
		return structb_data_;
	};


	Handle GetHandle() {
		return structb_->GetHandle();
	};
    void AddElement(T& element) {
        if (max_element_count_ > structb_data_.size()) {
            structb_data_.push_back(element);
            SaveChanges();
        }
        else {
            OutputDebugStringA("struct buffer limit Reached!");
        }
    }
    void RemoveLastElement() {
        if (0 < structb_data_.size()) {
            structb_data_.pop_back();
            SaveChanges();
        }
        else {
            OutputDebugStringA("struct buffer is empty!");
        }
    }
};