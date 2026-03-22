#pragma once
#include "ZBuffer.h"
#include "RenderTarget.h"
struct GBuffer {
	std::shared_ptr<RenderTarget> albedo_;
	std::shared_ptr<RenderTarget> normal_;
	std::shared_ptr<Zbuffer> depth_;
};