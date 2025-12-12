#pragma once
#include"vectors.h"
#include "eigen/Eigen/dense"

class Camera {
private:
	Vec3f camera_position;
	Vec3f look_at;
	Vec3f forward;
	Vec3f right;
	Vec3f up;
	Eigen::Matrix4f world_transform;
public:
	Camera(Vec3f cam_pos, Vec3f look, Vec3f Up);
	Eigen::Matrix4f getWorldTransform();
	Vec3f FixVectors(Vec3f vec);
};