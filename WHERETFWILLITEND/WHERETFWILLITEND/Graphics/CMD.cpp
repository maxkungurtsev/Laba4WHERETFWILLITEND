#include "CMD.h"
#include "Gdevice.h"
CMD::CMD(ComPtr<ID3D12Device> device, D3D12_COMMAND_LIST_TYPE type) {
	device_ = device;
	CreateCMDList(type);
	CreateCMDQueue(type);
	CreateCMDAllocator(type);
}
void CMD::CreateCMDList(D3D12_COMMAND_LIST_TYPE type) {
	HRESULT hr = device_->CreateCommandList(0, type, command_allocator_.Get(), nullptr, IID_PPV_ARGS(&command_list_));
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to create command list");
	}
}
void CMD::CreateCMDQueue(D3D12_COMMAND_LIST_TYPE type) {
	D3D12_COMMAND_QUEUE_DESC queueDesc{};
	queueDesc.Type = type;
	queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.NodeMask = 0;
	HRESULT hr = device_->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&command_queue_));
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to create command queue");
	}
}
void CMD::CreateCMDAllocator(D3D12_COMMAND_LIST_TYPE type) {
	HRESULT hr = device_->CreateCommandAllocator(type, IID_PPV_ARGS(&command_allocator_));
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to create command allocator");
	}
}
void CMD::ResetAllocator() {
	command_allocator_->Reset();
	command_list_->Reset(command_allocator_.Get(), nullptr);
}