#pragma once
#include "ZBuffer.h"
#include "RenderTarget.h"
class GBuffer {
public:
	std::shared_ptr<RenderTarget> albedo_;
	std::shared_ptr<RenderTarget> normal_;
	std::shared_ptr<Zbuffer> depth_;
	GBuffer(UINT width, UINT height, std::shared_ptr<Gdevice> device);
};