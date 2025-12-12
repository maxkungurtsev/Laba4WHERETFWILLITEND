#include "Camera.h"
Camera::Camera(Vec3f cam_pos, Vec3f look, Vec3f Up) {
	cam_pos.x = -cam_pos.x;
	cam_pos.y = -cam_pos.y;
	cam_pos.z = -cam_pos.z;
	camera_position = cam_pos;
	look_at = look;
	forward = (look - cam_pos).normalize();
	if (forward.length() == 0.0) {
		forward = Vec3f(0.0, 0.0, 1.0);
	}
	float zoom = 1 / ((look - cam_pos).length());
	//std::cout << ((look - cam_pos).length()) << '\n';
	right = (Up.normalize()) ^ forward;
	up = forward ^ right;
	world_transform << right.x,   right.y,   right.z,   -(camera_position * right),
					   up.x,      up.y,      up.z,      -(camera_position * up),
					   forward.x, forward.y, forward.z, -(camera_position * forward),
					   0.0,       0.0,       0.0,       1.0;
	world_transform *= zoom;
}
Eigen::Matrix4f Camera::getWorldTransform() {
	return world_transform;
}