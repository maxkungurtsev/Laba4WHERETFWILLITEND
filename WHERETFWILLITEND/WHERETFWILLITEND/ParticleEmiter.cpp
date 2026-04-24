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
	const int poolSize = max_particles / 2;
	D3D12_RESOURCE_STATES state = static_cast<D3D12_RESOURCE_STATES>(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	alive_in_ = std::make_shared<StructBuffer<Particle>>(device, poolSize, state);
	alive_out_ = std::make_shared<StructBuffer<Particle>>(device, poolSize, state);
	dead_in_ = std::make_shared<StructBuffer<Particle>>(device, poolSize, state);
	dead_out_ = std::make_shared<StructBuffer<Particle>>(device, poolSize, state);

	for (int i = 0; i < poolSize; i++) {
		alive_in_->GetData()[i].SetPos(position);
		alive_in_->GetData()[i].SetVelocity(XMFLOAT3(0, 0, 0));
		alive_in_->GetData()[i].SetLife(0.0f);
		alive_out_->GetData()[i] = alive_in_->GetData()[i];
		dead_in_->GetData()[i] = alive_in_->GetData()[i];
		dead_out_->GetData()[i] = alive_in_->GetData()[i];
	}
	emiter_cb_ = std::make_shared<Cbuffer<ParticleSimCB>>(device);
	emiter_render_cb_ = std::make_shared<Cbuffer<ParticleRenderCB>>(device);

	emiter_cb_->GetData().dt_ = dt; 
	emiter_cb_->GetData().time_ = time;
	emiter_cb_->GetData().aliveInCount_ = 0;
	emiter_cb_->GetData().deadInCount_ = poolSize;
	emiter_cb_->GetData().emiter_pos_ = pos;
	emiter_cb_->GetData().emitCount_ = emit_count;
	emiter_cb_->Save_changes();
	alive_in_->SaveChanges(false);
	alive_out_->SaveChanges(false);
	dead_in_->SaveChanges(false);
	dead_out_->SaveChanges(false);
	UpdateCbuffer(time);
}

void ParticleEmiter::SwapSimulationBuffers() {
	std::swap(alive_in_, alive_out_);
	std::swap(dead_in_, dead_out_);
}

void ParticleEmiter::UpdateCbuffer(float time) {
	ParticleSimCB& sim = emiter_cb_->GetData();
	sim.dt_ = max(0.0f, time - sim.time_);
	sim.time_ = time;
	sim.emiter_pos_ = position;

	const int maxParticles = static_cast<int>(alive_in_->GetData().size());
	const int spawnCount = min(sim.emitCount_, sim.deadInCount_);
	sim.aliveInCount_ = min(maxParticles, sim.aliveInCount_ + spawnCount);
	sim.deadInCount_ = maxParticles - sim.aliveInCount_;
	emiter_cb_->Save_changes();
};
