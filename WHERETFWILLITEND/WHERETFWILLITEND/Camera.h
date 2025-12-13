#pragma once
#include <iostream>
#include <windows.h>
#include <DirectXMath.h>
class Camera {
private:
	DirectX::XMFLOAT3 camera_position_;
	DirectX::XMFLOAT3 look_at_;
	DirectX::XMFLOAT3 up_;
	DirectX::XMFLOAT4X4 view_;
public:
	Camera(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 target, DirectX::XMFLOAT3 up);
	void UpdateView();
	const XMFLOAT4X4& GetViewMatrix() const { return view_; }
};