#pragma once
#include <DirectXMath.h>
#include "ZBuffer.h"
#include "CBuffer.h"
#include "Constants.h"
using namespace DirectX;


class Cascade {
private:
    XMFLOAT4X4 view_mat;
    XMFLOAT4X4 proj_mat;
    XMFLOAT4X4 view_proj_;
    std::shared_ptr<Cbuffer<POVConstants>> pov_buffer_;
    float split_depth_ = 0.0f;
    float prev_split_depth_=0.0f;
    UINT width_ = 0;
    UINT height_ = 0;
    float split_lambda_ = 0.75f;
    std::shared_ptr<Zbuffer> buffer_;
    XMVECTOR light_pos_;
    XMVECTOR light_target_;
    UINT cascade_index_;
    UINT cascade_count_;
    float camera_near_ = 0.1f;
    float camera_far_ = 10000.0f;
    float distance_;
public:
    Cascade(UINT width, UINT height, std::shared_ptr<Gdevice> device, XMVECTOR light_pos,XMVECTOR target,UINT cascade_index,UINT cascade_count,
        float aspect_ratio, float split_lambda);
    XMFLOAT4X4& GetView() { return view_mat; }
    XMFLOAT4X4& GetProj() { return proj_mat; }
    XMFLOAT4X4& GetViewProj();
    std::shared_ptr<Zbuffer> GetZbuffer() { return buffer_; }
    std::shared_ptr<Cbuffer<POVConstants>> GetPovBuffer() { return pov_buffer_; }
    void UpdateMatrix(XMMATRIX& cameraView, float cameraFovY, float cameraAspect);
    float CalculateSplitDepth(UINT split_index);
    float GetSplitDepth() const { return split_depth_; }
};