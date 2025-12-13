#include "Camera.h"
using namespace DirectX;
Camera::Camera(XMFLOAT3 position, XMFLOAT3 target, XMFLOAT3 up): camera_position_(position), look_at_(target), up_(up){
    UpdateView();
}

void Camera::UpdateView() {
    XMVECTOR eye = XMLoadFloat3(&camera_position_);
    XMVECTOR at = XMLoadFloat3(&look_at_);
    XMVECTOR up = XMLoadFloat3(&up_);
    XMMATRIX view = XMMatrixLookAtLH(eye, at, up);
    XMStoreFloat4X4(&view_, view);
};