#pragma once
#include <d3d12.h> 
#include <dxgi1_6.h>
#include <windows.h>
#include <wrl.h>
#include <iostream>
using Microsoft::WRL::ComPtr;
class Gdevice;
class CMD {
public:
	ComPtr<ID3D12CommandQueue> command_queue_;
	ComPtr<ID3D12CommandAllocator> command_allocator_;
	ComPtr<ID3D12GraphicsCommandList> command_list_;
	ComPtr<ID3D12Device> device_;
	CMD(ComPtr<ID3D12Device> device, D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT);
	void ResetAllocator();
	void Execute();
private:
	void CreateCMDList(D3D12_COMMAND_LIST_TYPE type);
	void CreateCMDQueue(D3D12_COMMAND_LIST_TYPE type);
	void CreateCMDAllocator(D3D12_COMMAND_LIST_TYPE type);
};