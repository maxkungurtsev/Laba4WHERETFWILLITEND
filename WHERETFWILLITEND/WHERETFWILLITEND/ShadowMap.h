#pragma once
#include "Graphics\Cascade.h"

class ShadowMap {
private:
	std::vector<std::shared_ptr<Cascade>> cascades_;
	UINT cascade_count_ = 4;
	void CheckCascadeIndex(UINT cascade_index) const;
public:
	ShadowMap(UINT width, UINT height, UINT cascade_count, std::shared_ptr<Gdevice> device, XMVECTOR light_pos, 
		XMVECTOR target, float aspect_ratio, float split_lambda);
	std::shared_ptr<Cascade> GetCascade(UINT cascade_index) const;
	Handle GetSRVHandle(UINT cascade_index) const;
	Handle GetDSVHandle(UINT cascade_index) const;
	UINT GetCascadeCount() const;
	void UpdateMatricies(XMMATRIX& cameraView, XMMATRIX& cameraProj, float cameraFovY, float cameraAspect);
};