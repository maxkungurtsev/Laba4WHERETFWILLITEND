#include "Cascade.h"

Cascade::Cascade(UINT width, 
	UINT height, 
	std::shared_ptr<Gdevice> device, 
	XMVECTOR light_pos, 
	XMVECTOR target, 
	UINT cascade_index, 
	UINT cascade_count,
	float distance,
	float split_lambda): width_(width), height_(height), light_pos_(light_pos), light_target_(target),
	cascade_index_(cascade_index), cascade_count_(cascade_count) {
	pov_buffer_ = std::make_shared<Cbuffer<POVConstants>>(device);
	if (width_ == 0 || height_ == 0) {
			throw std::runtime_error("ShadowMap dimensions must be greater than zero");
	}
	std::string cascade_name = "cascade_";
	buffer_ = std::make_shared<Zbuffer>(width_, height_, cascade_name, device, TextureUsage::Depth);
    distance_ = distance;
	split_lambda_ = max(split_lambda, 0.01f);
	const XMMATRIX view = XMMatrixLookAtLH(light_pos_ , target, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	XMStoreFloat4x4(&pov_buffer_->GetData().view, view);
	XMStoreFloat4x4(&pov_buffer_->GetData().inv_view, XMMatrixInverse(nullptr, XMLoadFloat4x4(&pov_buffer_->GetData().view)));
	XMStoreFloat4x4(&pov_buffer_->GetData().projection, XMMatrixIdentity());
	XMStoreFloat4x4(&pov_buffer_->GetData().inv_projection, XMMatrixInverse(nullptr, XMLoadFloat4x4(&pov_buffer_->GetData().projection)));
	XMStoreFloat4x4(&pov_buffer_->GetData().model, XMMatrixIdentity());
	XMStoreFloat4x4(&pov_buffer_->GetData().inv_model, XMMatrixInverse(nullptr, XMLoadFloat4x4(&pov_buffer_->GetData().model)));
	split_lambda_ = min(split_lambda_, 1.0f);
	pov_buffer_->Save_changes();
}



XMFLOAT4X4& Cascade::GetViewProj(){
	return view_proj_;
}
float Cascade::CalculateSplitDepth(UINT split_index) {
	if (split_index == 0) {
		return camera_near_;
	}
	if (split_index >= cascade_count_) {
		return camera_far_;
	}

	const float ratio = camera_far_ / camera_near_;
	const float cascade_ratio = static_cast<float>(split_index) / static_cast<float>(cascade_count_);
	const float logarithmic_split = camera_near_ * std::pow(ratio, cascade_ratio);
	const float uniform_split = camera_near_ + (camera_far_ - camera_near_) * cascade_ratio;
	return split_lambda_ * logarithmic_split + (1.0f - split_lambda_) * uniform_split;
}
void Cascade::UpdateMatrix(XMMATRIX& cameraView, float cameraFovY, float cameraAspect){
    prev_split_depth_ = CalculateSplitDepth(cascade_index_);
    split_depth_ = CalculateSplitDepth(cascade_index_ + 1);
    XMMATRIX invView = XMMatrixInverse(nullptr, cameraView);
    float tanHalfFovY = tanf(cameraFovY * 0.5f);
    float tanHalfFovX = tanHalfFovY * cameraAspect;
    float nearY = prev_split_depth_ * tanHalfFovY;
    float nearX = prev_split_depth_ * tanHalfFovX;
    float farY = split_depth_ * tanHalfFovY;
    float farX = split_depth_ * tanHalfFovX;
    XMVECTOR frustumCornersVS[8] =
    {
        XMVectorSet(-nearX,  nearY, prev_split_depth_, 1.0f),
        XMVectorSet(nearX,   nearY, prev_split_depth_, 1.0f),
        XMVectorSet(nearX,  -nearY, prev_split_depth_, 1.0f),
        XMVectorSet(-nearX, -nearY, prev_split_depth_, 1.0f),
        XMVectorSet(-farX,   farY, split_depth_, 1.0f),
        XMVectorSet(farX,    farY, split_depth_, 1.0f),
        XMVectorSet(farX,   -farY, split_depth_, 1.0f),
        XMVectorSet(-farX,  -farY, split_depth_, 1.0f),
    };
    XMVECTOR frustumCornersWS[8];
    for (int i = 0; i < 8; ++i)
    {
        frustumCornersWS[i] =
            XMVector4Transform(
                frustumCornersVS[i],
                invView);
    }
    XMVECTOR cascadeCenter = XMVectorZero();

    for (int i = 0; i < 8; ++i)
    {
        cascadeCenter += frustumCornersWS[i];
    }
    cascadeCenter /= 8.0f;
    XMVECTOR lightDir =XMVector3Normalize(light_target_ - light_pos_);
    XMVECTOR lightPos =cascadeCenter - (lightDir*distance_);
    XMVECTOR up = XMVectorSet(0, 1, 0, 0);
    XMMATRIX lightView = XMMatrixLookAtLH(lightPos,cascadeCenter,up);
    float minX = FLT_MAX;
    float maxX = -FLT_MAX;
    float minY = FLT_MAX;
    float maxY = -FLT_MAX;
    float minZ = FLT_MAX;
    float maxZ = -FLT_MAX;
    for (int i = 0; i < 8; ++i)
    {
        XMVECTOR cornerLS =XMVector4Transform(frustumCornersWS[i],lightView);
        XMFLOAT3 p;
        XMStoreFloat3(&p, cornerLS);
        minX = min(minX, p.x);
        maxX = max(maxX, p.x);
        minY = min(minY, p.y);
        maxY = max(maxY, p.y);
        minZ = min(minZ, p.z);
        maxZ = max(maxZ, p.z);
    }
    constexpr float zMult = 10.0f;
    if (minZ < 0.0f){
        //minZ *= zMult;
    }
    else{
        //minZ /= zMult;
    }
    if (maxZ < 0.0f){
        //maxZ /= zMult;
    }
    else{
        //maxZ *= zMult;
    }
    XMMATRIX lightProj =XMMatrixOrthographicOffCenterLH(minX,maxX,minY,maxY,minZ,maxZ);
    XMStoreFloat4x4(&pov_buffer_->GetData().view,lightView);
    XMStoreFloat4x4(&pov_buffer_->GetData().inv_view,XMMatrixInverse(nullptr, lightView));
    XMStoreFloat4x4(&pov_buffer_->GetData().projection,lightProj);
    XMStoreFloat4x4(&pov_buffer_->GetData().inv_projection,XMMatrixInverse(nullptr, lightProj));
    XMStoreFloat4x4(&view_proj_, XMMatrixMultiply(lightProj, lightView));
    pov_buffer_->Save_changes();
}