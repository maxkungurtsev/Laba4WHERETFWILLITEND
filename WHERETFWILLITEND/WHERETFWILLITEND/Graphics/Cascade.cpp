#include "Cascade.h"

Cascade::Cascade(UINT width, UINT height, std::shared_ptr<Gdevice> device, XMVECTOR light_pos, XMVECTOR target): width_(width), height_(height) {
		if (width_ == 0 || height_ == 0) {
			throw std::runtime_error("ShadowMap dimensions must be greater than zero");
		}
		std::string cascade_name = "cascade_";
		XMMATRIX view = XMMatrixLookAtLH(light_pos,target,XMVectorSet(0, 1, 0, 0));
		XMMATRIX proj = XMMatrixOrthographicLH(200.0f,200.0f,0.1f,1000.0f);
		//XMMATRIX viewproj = view * proj;
		//view_mat = XMStoreFloat4x4(view);
		//proj_mat = XMStoreFloat4x4(proj);
		buffer_ = std::make_shared<Zbuffer>(width_, height_, cascade_name, device, TextureUsage::Depth);
}