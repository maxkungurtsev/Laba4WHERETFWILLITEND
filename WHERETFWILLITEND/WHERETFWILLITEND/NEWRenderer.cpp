#include "NEWRenderer.h"
void NewRenderer::Initialize(UINT width, UINT height, int num_descriptors, Window hwnd) {
	//device, cmd, fence, heaps
	device_ = std::make_shared<Gdevice>(width, height, num_descriptors);
	//swapchain
	hwnd.CreateSwapChain(device_);
	//placeholder texture
	dummy_.read_tga_file("dummy.tga");
	//
	std::string name = "zbuffer";
	g_buffer_.depth_=std::make_shared<Zbuffer>(width, height, name, device_, TextureUsage::Depth);
}