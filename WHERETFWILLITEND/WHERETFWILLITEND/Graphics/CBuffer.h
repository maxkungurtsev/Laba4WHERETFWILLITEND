#pragma once
#include "Gdevice.h"
template<typename T>
class Cbuffer {
private:
	std::shared_ptr<Gdevice> device_;
	ComPtr<ID3D12Resource>  cb_;
	T cb_data_;
	void* cb_data_mapped_;
public:
	Cbuffer(std::shared_ptr<Gdevice> device);
};