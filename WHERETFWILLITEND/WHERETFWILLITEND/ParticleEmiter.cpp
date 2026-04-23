#pragma once
#include "ParticleEmiter.h"
ParticleEmiter::ParticleEmiter(std::string& texture_name, std::shared_ptr<Gdevice> device, XMFLOAT4 pos, float dt, float time, int max_particles, int emit_count) {
	TGAImage image_tga;
	const Image* image_png;
	//choose between parsers
	if (texture_name.substr(texture_name.size() - 3) == "tga") {
		image_tga.read_tga_file(texture_name.c_str());
		texture = std::make_shared<GTexture>(image_tga, texture_name, device, TextureUsage::Albedo);
	}
	else {
		ScratchImage image;
		std::wstring wpath(texture_name.begin(), texture_name.end());
		HRESULT hr = LoadFromWICFile(wpath.c_str(), WIC_FLAGS_NONE, nullptr, image);
		if (FAILED(hr)) {
			throw std::runtime_error("failed loading texture from png");
		}
		image_png = image.GetImage(0, 0, 0);
		texture = std::make_shared<GTexture>(image_png, texture_name, device, TextureUsage::Albedo);
	}
	position = pos;
	append = std::make_shared<StructBuffer<Particle>>(device, max_particles/2);
	consume = std::make_shared<StructBuffer<Particle>>(device, max_particles/2);
	for (int i = 0; i < max_particles; i++) {
		append->GetData()[i].SetPos(position);
		consume->GetData()[i].SetPos(position);
	}
	emiter_cb_ = std::make_shared<Cbuffer<ParticleSimCB>>(device);
	UpdateCbuffer(time);
	emiter_cb_->GetData().dt_ = dt;
	emiter_cb_->GetData().emiter_pos_ = pos;
	emiter_cb_->GetData().emitCount_ = emit_count;
	emiter_cb_->Save_changes();
}
void ParticleEmiter::UpdateCbuffer(float time) {
	emiter_cb_->GetData().dt_ = time- emiter_cb_->GetData().time_;
	emiter_cb_->GetData().time_ = time;
	emiter_cb_->Save_changes();
};
