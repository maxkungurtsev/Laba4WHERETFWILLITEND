#include "Camera.h"
Camera::Camera(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 target, DirectX::XMFLOAT3 up) : camera_position_(position), look_at_(target), up_(up) {
    UpdateView();
}

void Camera::UpdateView() {
    DirectX::XMVECTOR eye = XMLoadFloat3(&camera_position_);
    DirectX::XMVECTOR at = XMLoadFloat3(&look_at_);
    DirectX::XMVECTOR up = XMLoadFloat3(&up_);
    DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(eye, at, up);
    memcpy(&view_, &view, sizeof(DirectX::XMFLOAT4X4));
};