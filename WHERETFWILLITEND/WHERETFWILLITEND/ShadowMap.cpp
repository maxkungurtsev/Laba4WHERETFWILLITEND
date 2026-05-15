#include "ShadowMap.h"

ShadowMap::ShadowMap(UINT width, UINT height, UINT cascade_count, std::shared_ptr<Gdevice> device, XMVECTOR light_pos, XMVECTOR target, float aspect_ratio, float split_lambda){
	for (UINT i = 0; i < cascade_count_; i++) {
		cascades_.push_back(std::make_shared<Cascade>(width, height, device, light_pos, target, i, cascade_count_, aspect_ratio, split_lambda));
	}
}
void ShadowMap::CheckCascadeIndex(UINT cascade_index) const {
	if (cascade_index >= cascade_count_) {
		throw std::out_of_range("ShadowMap cascade index is out of range");
	}
}

std::shared_ptr<Cascade> ShadowMap::GetCascade(UINT cascade_index) const {
	CheckCascadeIndex(cascade_index);
	return cascades_[cascade_index];
}

Handle ShadowMap::GetSRVHandle(UINT cascade_index) const {
	CheckCascadeIndex(cascade_index);
	return cascades_[cascade_index]->GetZbuffer()->z_buffer_->GetResourse()->GetHandle();
}

Handle ShadowMap::GetDSVHandle(UINT cascade_index) const {
	CheckCascadeIndex(cascade_index);
	return cascades_[cascade_index]->GetZbuffer()->handle_;
}

UINT ShadowMap::GetCascadeCount() const {
	return cascade_count_;
}
void ShadowMap::UpdateMatricies(XMVECTOR camera_target, XMVECTOR camera_pos, XMVECTOR camera_up_, float fov_y){
	for (int i = 0; i < cascades_.size(); i++) {
		cascades_[i]->UpdateMatrix(camera_target, camera_pos, camera_up_, fov_y);
	}
}