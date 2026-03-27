#include "GTexture.h"

GTexture::GTexture(TGAImage image, std::string& name, std::shared_ptr<Gdevice> device, TextureUsage usage) {
	FillData(image.get_width(), image.get_height(), name, device, usage);
	device->cmd_->ResetAllocator();
	UINT64 uploadBufferSize;
	device->GetDXDevice()->GetCopyableFootprints(&(Gresourse_->GetResDesc()), 0, 1, 0, nullptr, nullptr, nullptr, &uploadBufferSize);
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
	HRESULT hr = device->GetDXDevice()->CreateCommittedResource(
		&uploadHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&uploadDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
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
	for (UINT y = 0; y < height_; y++) {
		for (UINT x = 0; x < width_; x++){
			TGAColor color = image.get(x, y);
			UINT idx = (y * width_ + x) * 4;
			reinterpret_cast<BYTE*>(mappedData)[idx + 0] = color.b;
			reinterpret_cast<BYTE*>(mappedData)[idx + 1] = color.g;
			reinterpret_cast<BYTE*>(mappedData)[idx + 2] = color.r;
			reinterpret_cast<BYTE*>(mappedData)[idx + 3] = color.a;
		}
	}
	textureUploadHeap->Unmap(0, nullptr);
	// data to GPU texture
	int index = static_cast<int>(usage);
	D3D12_TEXTURE_COPY_LOCATION dst{};
	dst.pResource = Gresourse_->GetResourse().Get();
	dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	dst.SubresourceIndex = 0;
	D3D12_TEXTURE_COPY_LOCATION src{};
	src.pResource = textureUploadHeap.Get();
	src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	src.PlacedFootprint.Offset = 0;
	src.PlacedFootprint.Footprint.Format = formats[index];
	src.PlacedFootprint.Footprint.Width = width_;
	src.PlacedFootprint.Footprint.Height = height_;
	src.PlacedFootprint.Footprint.Depth = 1;
	src.PlacedFootprint.Footprint.RowPitch =width_ * 4; 
	device->cmd_->command_list_->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = Gresourse_->GetResourse().Get();
	barrier.Transition.StateBefore = initial_states_[index];
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	device->cmd_->command_list_->ResourceBarrier(1, &barrier);
	device->cmd_->command_list_->Close();
	ID3D12CommandList* lists[] = { device->cmd_->command_list_.Get() };
	device->cmd_->command_queue_->ExecuteCommandLists(1, lists);
	device->fence_->IncrementFenceValue();
	device->cmd_->command_queue_->Signal(device->fence_->GetFence().Get(), device->fence_->GetFenceValue());
	if (device->fence_->GetFence()->GetCompletedValue() < device->fence_->GetFenceValue()) {
		HANDLE eventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		device->fence_->GetFence()->SetEventOnCompletion(device->fence_->GetFenceValue(), eventHandle);
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
};

GTexture::GTexture(const DirectX::Image* image, std::string& name, std::shared_ptr<Gdevice> device, TextureUsage usage) {
	FillData(image->width, image->height, name, device, usage);
	device->cmd_->ResetAllocator();
	UINT64 uploadBufferSize;
	device->GetDXDevice()->GetCopyableFootprints(&(Gresourse_->GetResDesc()), 0, 1, 0, nullptr, nullptr, nullptr, &uploadBufferSize);
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
	HRESULT hr = device->GetDXDevice()->CreateCommittedResource(
		&uploadHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&uploadDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
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
	for (UINT y = 0; y < height_; y++) {
		for (UINT x = 0; x < width_; x++) {
			const uint8_t* pixel = image->pixels + y * image->rowPitch + x * 4;
			UINT idx = (y * width_ + x) * 4;

			reinterpret_cast<BYTE*>(mappedData)[idx + 0] = pixel[2]; // B
			reinterpret_cast<BYTE*>(mappedData)[idx + 1] = pixel[1]; // G
			reinterpret_cast<BYTE*>(mappedData)[idx + 2] = pixel[0]; // R
			reinterpret_cast<BYTE*>(mappedData)[idx + 3] = pixel[3]; // A
		}
	}
	textureUploadHeap->Unmap(0, nullptr);
	// data to GPU texture
	int index = static_cast<int>(usage);
	D3D12_TEXTURE_COPY_LOCATION dst{};
	dst.pResource = Gresourse_->GetResourse().Get();
	dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	dst.SubresourceIndex = 0;
	D3D12_TEXTURE_COPY_LOCATION src{};
	src.pResource = textureUploadHeap.Get();
	src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	src.PlacedFootprint.Offset = 0;
	src.PlacedFootprint.Footprint.Format = formats[index];
	src.PlacedFootprint.Footprint.Width = width_;
	src.PlacedFootprint.Footprint.Height = height_;
	src.PlacedFootprint.Footprint.Depth = 1;
	src.PlacedFootprint.Footprint.RowPitch = width_ * 4;
	device->cmd_->command_list_->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = Gresourse_->GetResourse().Get();
	barrier.Transition.StateBefore = initial_states_[index];
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	device->cmd_->command_list_->ResourceBarrier(1, &barrier);
	device->cmd_->command_list_->Close();
	ID3D12CommandList* lists[] = { device->cmd_->command_list_.Get() };
	device->cmd_->command_queue_->ExecuteCommandLists(1, lists);
	device->fence_->IncrementFenceValue();
	device->cmd_->command_queue_->Signal(device->fence_->GetFence().Get(), device->fence_->GetFenceValue());
	if (device->fence_->GetFence()->GetCompletedValue() < device->fence_->GetFenceValue()) {
		HANDLE eventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		device->fence_->GetFence()->SetEventOnCompletion(device->fence_->GetFenceValue(), eventHandle);
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
};
GTexture::GTexture(UINT width, UINT height, std::string& name, std::shared_ptr<Gdevice> device, TextureUsage usage, D3D12_RESOURCE_FLAGS flag) {
	FillData(width, height, name, device, usage, flag);
};
GTexture::GTexture(std::shared_ptr<GResourse> Gresourse, TextureUsage usage):Gresourse_(Gresourse){
	usage_ = usage;
}

std::shared_ptr<GResourse> GTexture::GetResourse() {
	return Gresourse_;
}
void GTexture::FillData(UINT width, UINT height, std::string& name, std::shared_ptr<Gdevice> device, TextureUsage usage, D3D12_RESOURCE_FLAGS flag) {
	width_ = width;
	height_ = height;
	D3D12_HEAP_PROPERTIES heapProps{};
	D3D12_CLEAR_VALUE clear_value;
	D3D12_CLEAR_VALUE* clear_value_pointer=nullptr;
	D3D12_RESOURCE_DESC desc;
	DXGI_FORMAT srv_format;
	int index = static_cast<int>(usage);
	switch (usage) {
	case TextureUsage::Albedo:
		heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProps.CreationNodeMask = 1;
		heapProps.VisibleNodeMask = 1;
		desc = CD3DX12_RESOURCE_DESC::Tex2D(formats[index],
			width, height, 1, 1, 1, 0,
			flag,
			D3D12_TEXTURE_LAYOUT_UNKNOWN, 0);
		srv_format = formats[index];
		break;
	case TextureUsage::Normalmap:
		heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProps.CreationNodeMask = 1;
		heapProps.VisibleNodeMask = 1;
		desc = CD3DX12_RESOURCE_DESC::Tex2D(formats[index],
			width, height,1, 1, 1, 0,
			flag,
			D3D12_TEXTURE_LAYOUT_UNKNOWN, 0);
		srv_format = formats[index];
		break;
	case TextureUsage::Depth:
		desc = CD3DX12_RESOURCE_DESC::Tex2D(formats[index],
			width,height,1,1,device->sample_amount_,device->msaa_quality_,
			D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
		//ClearValue
		clear_value.Format = DXGI_FORMAT_D32_FLOAT;
		clear_value.DepthStencil.Depth = 1.0f;
		clear_value.DepthStencil.Stencil = 0;
		//heap props
		heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProps.CreationNodeMask = 1;
		heapProps.VisibleNodeMask = 1;
		clear_value_pointer = &clear_value;
		srv_format = DXGI_FORMAT_R32_FLOAT;
		break;
	case TextureUsage::MaterialIndex:
		heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProps.CreationNodeMask = 1;
		heapProps.VisibleNodeMask = 1;
		desc = CD3DX12_RESOURCE_DESC::Tex2D(formats[index],
			width, height, 1, 1, 1, 0,
			flag,
			D3D12_TEXTURE_LAYOUT_UNKNOWN, 0);
		srv_format = formats[index];
		break;
	}
	Gresourse_ = std::make_shared<GResourse>(desc, srv_format, heapProps, name, device,initial_states_[index], clear_value_pointer);
}