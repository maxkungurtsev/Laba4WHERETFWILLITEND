#include "Cbuffer.h"
template<typename T>
Cbuffer<T>::Cbuffer(std::shared_ptr<Gdevice> device){
	D3D12_HEAP_PROPERTIES heapProps{};
	heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProps.CreationNodeMask = 1;
	heapProps.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC resDesc{};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Alignment = 0;
	resDesc.Width = (sizeof(T) + 255) & ~255;
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.Format = DXGI_FORMAT_UNKNOWN;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	device->CreateID3DResourse(heapProps, resDesc, nullptr, cb_);
	//mapping
	cb_->Map(0, nullptr, &cb_data_mapped_);
	//CBV desc
	D3D12_CONSTANT_BUFFER_VIEW_DESC mvp_cbv_desc{};
	mvp_cbv_desc.BufferLocation = cb_->GetGPUVirtualAddress();
	mvp_cbv_desc.SizeInBytes = resDesc.Width;
	device->heaps_->CreateCBV_CPU(cb_, resDesc.Width);
}