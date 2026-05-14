#pragma once
#include <DirectXMath.h>
#include "ZBuffer.h"
using namespace DirectX;


class Cascade {
private:
    XMFLOAT4X4 view_mat;
    XMFLOAT4X4 proj_mat;
    XMFLOAT4X4 viewProj_mat;
    float splitDepth;
    UINT width_ = 0;
    UINT height_ = 0;
    std::shared_ptr<Zbuffer> buffer_;  
public:
    Cascade(UINT width, UINT height, std::shared_ptr<Gdevice> device, XMVECTOR light_pos, XMVECTOR target);
    XMFLOAT4X4& GetViewProj() { return viewProj_mat; }
    std::shared_ptr<Zbuffer> GetZbuffer() { return buffer_; }
    void UpdateMatrix();
};