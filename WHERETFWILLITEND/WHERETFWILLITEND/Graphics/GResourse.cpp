#include "GResourse.h"
GResourse::GResourse(D3D12_RESOURCE_DESC res_desc, D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc, D3D12_HEAP_PROPERTIES heapProps, std::string& name, std::shared_ptr<Gdevice> device, D3D12_RESOURCE_STATES initial_state, D3D12_CLEAR_VALUE* clear_value) :
	name_(name), res_desc_(res_desc){
	//creating resourse
	
	device_ = device;
	device->CreateID3DResourse(heapProps, res_desc_, resourse_, initial_state, clear_value);
	srv_handle=device->heaps_->CreateSRV_CPU(srvDesc, resourse_);
}
ComPtr<ID3D12Resource> GResourse::GetResourse() {
	return resourse_;
};

std::shared_ptr<Gdevice> GResourse::GetDevice() {
	return device_;
};
D3D12_RESOURCE_DESC& GResourse::GetResDesc() {
	return res_desc_;
}
Handle GResourse::GetHandle() {
	return srv_handle;
};
