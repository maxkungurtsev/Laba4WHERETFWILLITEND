#include "Cascade.h"

Cascade::Cascade(UINT width, 
	UINT height, 
	std::shared_ptr<Gdevice> device, 
	XMVECTOR light_pos, 
	XMVECTOR target, 
	UINT cascade_index, 
	UINT cascade_count,
	float aspect_ratio,
	float split_lambda): width_(width), height_(height), light_pos_(light_pos), light_target_(target),
	cascade_index_(cascade_index), cascade_count_(cascade_count) {
		if (width_ == 0 || height_ == 0) {
			throw std::runtime_error("ShadowMap dimensions must be greater than zero");
		}
		std::string cascade_name = "cascade_";
		buffer_ = std::make_shared<Zbuffer>(width_, height_, cascade_name, device, TextureUsage::Depth);
		aspect_ratio_ = max(aspect_ratio, 0.01f);
		split_lambda_ = max(split_lambda, 0.01f);
		split_lambda_ = min(split_lambda_, 1.0f);
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

void Cascade::UpdateMatrix(XMVECTOR camera_target, XMVECTOR camera_pos, XMVECTOR camera_up_, float fov_y) {
	prev_split_depth_ = CalculateSplitDepth(cascade_index_);
	split_depth_ = CalculateSplitDepth(cascade_index_ + 1);

	XMVECTOR camera_forward = XMVector3Normalize(camera_target - camera_pos);
	if (XMVectorGetX(XMVector3LengthSq(camera_forward)) <= 0.000001f) {
		camera_forward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	}

	XMVECTOR camera_right = XMVector3Normalize(XMVector3Cross(camera_up_, camera_forward));
	if (XMVectorGetX(XMVector3LengthSq(camera_right)) <= 0.000001f) {
		camera_right = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	}
	XMVECTOR camera_up = XMVector3Normalize(XMVector3Cross(camera_forward, camera_right));

	const float near_height = 2.0f * std::tan(fov_y * 0.5f) * prev_split_depth_;
	const float near_width = near_height * aspect_ratio_;
	const float far_height = 2.0f * std::tan(fov_y * 0.5f) * split_depth_;
	const float far_width = far_height * aspect_ratio_;

	const XMVECTOR near_center = camera_pos + camera_forward * prev_split_depth_;
	const XMVECTOR far_center = camera_pos + camera_forward * split_depth_;

	XMVECTOR corners[8] = {
		near_center + camera_up * (near_height * 0.5f) - camera_right * (near_width * 0.5f),
		near_center + camera_up * (near_height * 0.5f) + camera_right * (near_width * 0.5f),
		near_center - camera_up * (near_height * 0.5f) - camera_right * (near_width * 0.5f),
		near_center - camera_up * (near_height * 0.5f) + camera_right * (near_width * 0.5f),
		far_center + camera_up * (far_height * 0.5f) - camera_right * (far_width * 0.5f),
		far_center + camera_up * (far_height * 0.5f) + camera_right * (far_width * 0.5f),
		far_center - camera_up * (far_height * 0.5f) - camera_right * (far_width * 0.5f),
		far_center - camera_up * (far_height * 0.5f) + camera_right * (far_width * 0.5f),
	};

	XMVECTOR cascade_center = XMVectorZero();
	for (int i = 0; i < 8; ++i) {
		cascade_center += corners[i];
	}
	cascade_center /= 8.0f;

	XMVECTOR light_direction = XMVector3Normalize(light_target_ - light_pos_);
	if (XMVectorGetX(XMVector3LengthSq(light_direction)) <= 0.000001f) {
		light_direction = XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f);
	}

	float cascade_radius = 0.0f;
	for (int i = 0; i < 8; ++i) {
		const float corner_distance = XMVectorGetX(XMVector3Length(corners[i] - cascade_center));
		cascade_radius = max(cascade_radius, corner_distance);
	}
	cascade_radius = std::ceil(cascade_radius * 16.0f) / 16.0f;

	const XMVECTOR light_eye = cascade_center - light_direction * (cascade_radius + split_depth_);
	const XMMATRIX view = XMMatrixLookAtLH(light_eye, cascade_center, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));

	XMVECTOR min_bounds = XMVectorSet(FLT_MAX, FLT_MAX, FLT_MAX, 1.0f);
	XMVECTOR max_bounds = XMVectorSet(-FLT_MAX, -FLT_MAX, -FLT_MAX, 1.0f);
	for (int i = 0; i < 8; ++i) {
		const XMVECTOR corner_light_space = XMVector3TransformCoord(corners[i], view);
		min_bounds = XMVectorMin(min_bounds, corner_light_space);
		max_bounds = XMVectorMax(max_bounds, corner_light_space);
	}

	const float min_x = XMVectorGetX(min_bounds);
	const float max_x = XMVectorGetX(max_bounds);
	const float min_y = XMVectorGetY(min_bounds);
	const float max_y = XMVectorGetY(max_bounds);
	const float min_z = XMVectorGetZ(min_bounds);
	const float max_z = XMVectorGetZ(max_bounds);

	const float near_z = max(0.0f, min_z - cascade_radius);
	const float far_z = max(near_z + 0.1f, max_z + cascade_radius);
	const XMMATRIX proj = XMMatrixOrthographicOffCenterLH(min_x, max_x, min_y, max_y, near_z, far_z);
	const XMMATRIX view_proj = view * proj;

	XMStoreFloat4x4(&view_mat, view);
	XMStoreFloat4x4(&proj_mat, proj);
	XMStoreFloat4x4(&viewProj_mat, view_proj);
};