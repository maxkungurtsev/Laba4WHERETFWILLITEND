#include "GTexture.h"

GTexture::GTexture(TGAImage image, std::string& name, std::shared_ptr<Gdevice> device, TextureUsage usage) {
	width_ = image.get_width();
	height_ = image.get_height();
	FillData(width_, height_, name, device, usage);
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
	D3D12_TEXTURE_COPY_LOCATION dst{};
	dst.pResource = Gresourse_->GetResourse().Get();
	dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	dst.SubresourceIndex = 0;
	D3D12_TEXTURE_COPY_LOCATION src{};
	src.pResource = textureUploadHeap.Get();
	src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	src.PlacedFootprint.Offset = 0;
	src.PlacedFootprint.Footprint.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	src.PlacedFootprint.Footprint.Width = width_;
	src.PlacedFootprint.Footprint.Height = height_;
	src.PlacedFootprint.Footprint.Depth = 1;
	src.PlacedFootprint.Footprint.RowPitch = width_ * 4;
	device->cmd_->command_list_->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = Gresourse_->GetResourse().Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	device->cmd_->command_list_->ResourceBarrier(1, &barrier);
	device->cmd_->command_list_->Close();
	ID3D12CommandList* lists[] = { device->cmd_->command_list_.Get() };
	device->cmd_->command_queue_->ExecuteCommandLists(1, lists);
	UINT64 fenceValue = (device->fence_->GetFenceValue());
	fenceValue++;
	device->cmd_->command_queue_->Signal(device->fence_->GetFence().Get(), fenceValue);
	if (device->fence_->GetFence()->GetCompletedValue() < fenceValue) {
		HANDLE eventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		device->fence_->GetFence()->SetEventOnCompletion(fenceValue, eventHandle);
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
};
GTexture::GTexture(UINT width, UINT height, std::string& name, std::shared_ptr<Gdevice> device, TextureUsage usage) {
	width_ = width;
	height_ = height;
	FillData(width_, height_, name, device, usage);
};
GTexture::GTexture(std::shared_ptr<GResourse> Gresourse, TextureUsage usage):Gresourse_(Gresourse){
	usage_ = usage;
}

std::shared_ptr<GResourse> GTexture::GetResourse() {
	return Gresourse_;
}
void GTexture::FillData(UINT width, UINT height, std::string& name, std::shared_ptr<Gdevice> device, TextureUsage usage) {
	width_ = width;
	height_ = height;
	D3D12_HEAP_PROPERTIES heapProps{};
	D3D12_CLEAR_VALUE clear_value;
	D3D12_CLEAR_VALUE* clear_value_pointer=nullptr;
	D3D12_RESOURCE_DESC desc;
	D3D12_RESOURCE_STATES initial_state= D3D12_RESOURCE_STATE_COPY_DEST;
	switch (usage) {
	case TextureUsage::Albedo:
		heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProps.CreationNodeMask = 1;
		heapProps.VisibleNodeMask = 1;
		desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_B8G8R8A8_UNORM,
			width, height, 1, 1, 1, 0,
			D3D12_RESOURCE_FLAG_NONE,
			D3D12_TEXTURE_LAYOUT_UNKNOWN, 0);
		break;
	case TextureUsage::Normalmap:
		heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProps.CreationNodeMask = 1;
		heapProps.VisibleNodeMask = 1;
		desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_B8G8R8A8_UNORM,
			width, height,1, 1, 1, 0,
			D3D12_RESOURCE_FLAG_NONE,
			D3D12_TEXTURE_LAYOUT_UNKNOWN, 0);
		break;
	case TextureUsage::Depth:
		desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT,
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
		initial_state = D3D12_RESOURCE_STATE_DEPTH_WRITE;
		break;
	}
	Gresourse_ = std::make_shared<GResourse>(desc, heapProps, name, device,initial_state, clear_value_pointer);
}