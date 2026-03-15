#pragma once
#include "d3dx12.h" 
#include "tgaimage.h"
#include <d3d12.h> 
#include <dxgi1_6.h>
#include <windows.h>
#include <wrl.h>
#include <iostream>
using Microsoft::WRL::ComPtr;
class Device {
	ComPtr<ID3D12Device> device_;
	int frame_count_;
	ComPtr<ID3D12CommandQueue> command_queue_;
	ComPtr<ID3D12CommandAllocator> command_allocator_;
	ComPtr<ID3D12GraphicsCommandList> command_list_;
	ComPtr<ID3D12DescriptorHeap> rtv_heap_;
	int rtv_amount_ = 0;
	ComPtr<ID3D12DescriptorHeap> dsv_heap_;
	int dsv_amount_ = 0;
	ComPtr<ID3D12DescriptorHeap> cbv_srv_uav_heap_;
	int cbv_srv_uav_amount_ = 0;
	ComPtr<ID3D12DescriptorHeap> sampler_heap_;
	int sampler_amount_ = 0;
	int rtv_descriptor_size_;
	int dsv_descriptor_size_;
	int cbv_srv_uav_descriptor_size_;
	int sampler_descriptor_size_;
	ComPtr<ID3D12Fence> fence_;
	UINT fence_value_ =0;
public:
	UINT width_;
	UINT height_;
	Device(UINT width, UINT height, int frame_count) {
		//create device
		//fence
		HRESULT hr = device_->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
		if (FAILED(hr)) {
			throw std::runtime_error("Failed to create fence");
		}
		{width_ = width;
		height_ = height;
		frame_count_ = frame_count;
		HRESULT hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device_));
		if (FAILED(hr)) {
			throw std::runtime_error("Failed to create graphics device");
		}
		// descriptor sizes
		rtv_descriptor_size_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		dsv_descriptor_size_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		cbv_srv_uav_descriptor_size_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		sampler_descriptor_size_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
		}
		//heaps
		{D3D12_DESCRIPTOR_HEAP_DESC desc{};
			desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			desc.NumDescriptors = frame_count_;
			desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			desc.NodeMask = 0;
			HRESULT hr = device_->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&rtv_heap_));
			if (FAILED(hr)) {
				throw std::runtime_error("Failed to create RTV heap");
			}
			desc.NumDescriptors = 1;
			desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
			hr = device_->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&dsv_heap_));
			if (FAILED(hr)) {
				throw std::runtime_error("Failed to create DSV heap");
			}
			D3D12_DESCRIPTOR_HEAP_DESC cbvDesc{};
			cbvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			// MAY BE ERROR CAUSE HARDCODED AMOUNT BUT I DONT GIVE A FUUUUUUUUUUUCK
			cbvDesc.NumDescriptors = 100;
			cbvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			hr = device_->CreateDescriptorHeap(&cbvDesc, IID_PPV_ARGS(&cbv_srv_uav_heap_));
			if (FAILED(hr)) {
				throw std::runtime_error("Failed to create CBV, SRV and UAV heap");
			}
			D3D12_DESCRIPTOR_HEAP_DESC sampDesc{};
			sampDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
			sampDesc.NumDescriptors = 1;
			sampDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			hr = device_->CreateDescriptorHeap(&sampDesc, IID_PPV_ARGS(&sampler_heap_));
			if (FAILED(hr)) {
				throw std::runtime_error("Failed to create Sampler heap");
			}}
		//command shit
		{D3D12_COMMAND_QUEUE_DESC queueDesc{};
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.NodeMask = 0;
		HRESULT hr = device_->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&command_queue_));
		if (FAILED(hr)) {
			throw std::runtime_error("Failed to create command queue");
		}
		hr = device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&command_allocator_));
		if (FAILED(hr)) {
			throw std::runtime_error("Failed to create command allocator");
		}
		hr = device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, command_allocator_.Get(), nullptr, IID_PPV_ARGS(&command_list_));
		if (FAILED(hr)) {
			throw std::runtime_error("Failed to create command list");
		}
		hr = command_list_->Close();
		if (FAILED(hr)) {
			throw std::runtime_error("Failed to close initial command list");
		}}
	};
	D3D12_CPU_DESCRIPTOR_HANDLE CreateSRVForTGA(ComPtr<ID3D12Resource>& resourse,int width, int height, D3D12_RESOURCE_DESC& texdesc, TGAImage image) {
		command_allocator_->Reset();
		command_list_->Reset(command_allocator_.Get(), nullptr);
		D3D12_HEAP_PROPERTIES heapProps{};
		heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProps.CreationNodeMask = 1;
		heapProps.VisibleNodeMask = 1;
		//creating resourse
		HRESULT hr = device_->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &texdesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(resourse.ReleaseAndGetAddressOf()));
		if (FAILED(hr)) {
			throw std::runtime_error(("Resourse creation error"));
		}
		UINT64 uploadBufferSize;
		device_->GetCopyableFootprints(&texdesc, 0, 1, 0, nullptr, nullptr, nullptr, &uploadBufferSize);
		D3D12_RESOURCE_DESC uploadDesc{};
		uploadDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		uploadDesc.Width = uploadBufferSize;
		uploadDesc.Height = 1;
		uploadDesc.DepthOrArraySize = 1;
		uploadDesc.MipLevels = 1;
		uploadDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		uploadDesc.SampleDesc.Count = 1;
		D3D12_HEAP_PROPERTIES uploadHeapProps{};
		uploadHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
		ComPtr<ID3D12Resource> textureUploadHeap;
		hr = device_->CreateCommittedResource(
			&uploadHeapProps,
			D3D12_HEAP_FLAG_NONE,
			&uploadDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&textureUploadHeap)
		);
		if (FAILED(hr)) {
			throw std::runtime_error("Failed to create texture upload heap");
		}
		//TGA â upload heap
		void* mappedData = nullptr;
		D3D12_RANGE readRange{ 0, 0 };
		textureUploadHeap->Map(0, &readRange, &mappedData);
		for (UINT y = 0; y < height; y++) {
			for (UINT x = 0; x < width; x++) {
				TGAColor color = image.get(x, y);
				UINT idx = (y * width + x) * 4;
				reinterpret_cast<BYTE*>(mappedData)[idx + 0] = color.b;
				reinterpret_cast<BYTE*>(mappedData)[idx + 1] = color.g;
				reinterpret_cast<BYTE*>(mappedData)[idx + 2] = color.r;
				reinterpret_cast<BYTE*>(mappedData)[idx + 3] = color.a;
			}
		}
		textureUploadHeap->Unmap(0, nullptr);
		// data to GPU texture
		D3D12_TEXTURE_COPY_LOCATION dst{};
		dst.pResource = resourse.Get();
		dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dst.SubresourceIndex = 0;
		D3D12_TEXTURE_COPY_LOCATION src{};
		src.pResource = textureUploadHeap.Get();
		src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		src.PlacedFootprint.Offset = 0;
		src.PlacedFootprint.Footprint.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		src.PlacedFootprint.Footprint.Width = width;
		src.PlacedFootprint.Footprint.Height = height;
		src.PlacedFootprint.Footprint.Depth = 1;
		src.PlacedFootprint.Footprint.RowPitch = width * 4;
		command_list_->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = resourse.Get();
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		command_list_->ResourceBarrier(1, &barrier);
		//srv
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = texdesc.Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		D3D12_CPU_DESCRIPTOR_HANDLE handle = cbv_srv_uav_heap_->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += (cbv_srv_uav_amount_* cbv_srv_uav_descriptor_size_);
		cbv_srv_uav_amount_++;
		device_->CreateShaderResourceView(resourse.Get(), &srvDesc, handle);
		command_list_->Close();
		ID3D12CommandList* lists[] = { command_list_.Get() };
		command_queue_->ExecuteCommandLists(1, lists);
		UINT64 fenceValue = ++fence_value_;
		command_queue_->Signal(fence_.Get(), fenceValue);
		if (fence_->GetCompletedValue() < fenceValue) {
			HANDLE eventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			fence_->SetEventOnCompletion(fenceValue, eventHandle);
			WaitForSingleObject(eventHandle, INFINITE);
			CloseHandle(eventHandle);
		}
		return handle;
	}
};
class Texture {
public:
	std::string name_;
	ComPtr<ID3D12Resource> resourse_;
	D3D12_RESOURCE_DESC tex_desc{};
	D3D12_CPU_DESCRIPTOR_HANDLE srv_handle;
	DXGI_FORMAT format_;
	Texture(Device device, std::string name, int width, int height,DXGI_FORMAT format = DXGI_FORMAT_B8G8R8A8_UNORM, TGAImage image){
		format_ = format;
		tex_desc = CD3DX12_RESOURCE_DESC::Tex2D(
			format_, width, height,
			1,      // arraySize
			1,      // mipLevels
			1,      // sampleCount
			0,      // sampleQuality
			D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
			D3D12_TEXTURE_LAYOUT_UNKNOWN,
			0       // alignment
		);
		// make SRV
		srv_handle = device.CreateSRVForTGA(resourse_, width, height, tex_desc);
		if(resourse_==nullptr){ throw std::runtime_error(("Resourse still null moron")); }
	}
};
class RT :public Texture {
public:
	D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle;
	D3D12_RENDER_TARGET_VIEW_DESC rtv_desc_{format_,D3D12_RTV_DIMENSION_TEXTURE2D,{ 0 }};
};