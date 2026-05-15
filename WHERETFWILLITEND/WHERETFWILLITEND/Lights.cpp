#include "Lights.h"
Lights::Lights(std::shared_ptr<Gdevice> device) {
	lights_ =std::make_shared<StructBuffer<LightData>>(device, 200, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	max_lights_ = std::make_shared<Cbuffer<XMFLOAT4>>(device);
	max_lights_->GetData() = XMFLOAT4{0,0,0,0};
	device_ = device;
}


void Lights::AddSpotlight(XMFLOAT3 strength, float falloff_start, XMFLOAT4 direction, XMFLOAT4 position, float falloff_end, float spot_power, bool in_render_frame, float velocity, float spawn_time, XMFLOAT4 movement_direction) {
	LightData newlight;
	newlight.direction = direction;
	newlight.strength = strength;
	newlight.falloff_start = falloff_start;
	newlight.position = position;
	newlight.falloff_end = falloff_end;
	newlight.spot_power = spot_power;
	newlight.type = 2;
	newlight.velocity = velocity;
	newlight.spawn_time=spawn_time;
	newlight.movement_direction = movement_direction;
	newlight.shad_map_index = shad_maps_.size();
	lights_->AddElement(newlight, max_lights_->GetData().x, in_render_frame);
	max_lights_->GetData().x += 1;
	max_lights_->Save_changes();
	// shad map init
	std::array<std::shared_ptr<ShadowMap>, 6> sm;
	XMVECTOR pos = XMLoadFloat4(&position); 
	XMVECTOR dir = XMLoadFloat4(&direction);
	XMVECTOR target = pos + dir;
	sm[0] = std::make_shared<ShadowMap>(1024, 1024, 4, device_, pos, target, 1.0f, 0.75f);
	shad_maps_.push_back(sm);
};
void Lights::AddAmbientlight(XMFLOAT3 strength, bool in_render_frame) {
	LightData newlight;
	newlight.direction = {0,0,0,0};
	newlight.strength = strength;
	newlight.falloff_start = 0;
	newlight.position = { 0,0,0,0 };
	newlight.falloff_end = 0;
	newlight.spot_power = 0;
	newlight.type = 3;
	newlight.velocity = 0;
	lights_->AddElement(newlight, max_lights_->GetData().x, in_render_frame);
	max_lights_->GetData().x += 1;
	max_lights_->Save_changes();
}

void Lights::AddDirlight(XMFLOAT3 strength, XMFLOAT4 direction, bool in_render_frame) {
	LightData newlight;
	newlight.direction = direction;
	newlight.strength = strength;
	newlight.falloff_start = 0;
	newlight.position = { 0,0,0,0 };
	newlight.falloff_end = 0;
	newlight.spot_power = 0;
	newlight.type = 0;
	newlight.velocity = 0;
	newlight.shad_map_index = shad_maps_.size();
	lights_->AddElement(newlight, max_lights_->GetData().x, in_render_frame);
	max_lights_->GetData().x += 1;
	max_lights_->Save_changes();
    // shad map init
	std::array<std::shared_ptr<ShadowMap>, 6> sm;
	XMVECTOR dir = XMLoadFloat4(&direction);
	XMVECTOR pos = -dir*500.0f;
	sm[0] = std::make_shared<ShadowMap>(1024, 1024, 4, device_, pos, XMVectorSet(0, 0, 0, 1), 1.0f, 0.75f);
	shad_maps_.push_back(sm);
}
void Lights::AddPointlight(XMFLOAT3 strength, XMFLOAT4 position, float falloff_start, float falloff_end, bool in_render_frame, float velocity, float spawn_time, XMFLOAT4 movement_direction) {
	LightData newlight;
	newlight.direction = { 0,0,0,0 };
	newlight.strength = strength;
	newlight.falloff_start = falloff_start;
	newlight.position = position;
	newlight.falloff_end = falloff_end;
	newlight.spot_power = 0;
	newlight.type = 1;
	newlight.velocity = velocity;
	newlight.spawn_time = spawn_time;
	newlight.movement_direction = movement_direction;
	newlight.shad_map_index = shad_maps_.size();
	lights_->AddElement(newlight, max_lights_->GetData().x, in_render_frame);
	max_lights_->GetData().x += 1;
	max_lights_->Save_changes();
	// shad map init
	std::array<std::shared_ptr<ShadowMap>, 6> sm;
	XMVECTOR pos = XMLoadFloat4(&position);
	XMVECTOR target = pos + XMVectorSet(0, 1, 0, 0);
	sm[0] = std::make_shared<ShadowMap>(1024, 1024, 4, device_, pos, target, 1.0f, 0.75f);
	target = pos + XMVectorSet(0, -1, 0, 0);
	sm[1] = std::make_shared<ShadowMap>(1024, 1024, 4, device_, pos, target, 1.0f, 0.75f);
	target = pos + XMVectorSet(1, 0, 0, 0);
	sm[2] = std::make_shared<ShadowMap>(1024, 1024, 4, device_, pos, target, 1.0f, 0.75f);
	target = pos + XMVectorSet(-1, 0, 0, 0);
	sm[3] = std::make_shared<ShadowMap>(1024, 1024, 4, device_, pos, target, 1.0f, 0.75f);
	target = pos + XMVectorSet( 0, 0, 1, 0);
	sm[4] = std::make_shared<ShadowMap>(1024, 1024, 4, device_, pos, target, 1.0f, 0.75f);
	target = pos + XMVectorSet(0, 0, -1, 0);
	sm[5] = std::make_shared<ShadowMap>(1024, 1024, 4, device_, pos, target, 1.0f, 0.75f);
	shad_maps_.push_back(sm);
}

void Lights::RemoveLastLight(bool in_render_frame) {
	lights_->RemoveLastElement(in_render_frame);
	max_lights_->GetData().x -= 1;
	max_lights_->Save_changes();
}
std::shared_ptr<StructBuffer<LightData>> Lights::GetBuffer() {
	return lights_;
}
std::shared_ptr <Cbuffer<XMFLOAT4>> Lights::GetMaxLights() {
	return max_lights_;
}


std::array<std::shared_ptr<ShadowMap>, 6>& Lights::GetShadowMap(int ind) {
	if (ind < shad_maps_.size()) {
		return shad_maps_[ind];
	}
	else {
		throw std::runtime_error("ind of shadow map out of range");
	}
}

void Lights::UpdateShadowMatricies(XMVECTOR camera_target, XMVECTOR camera_pos, XMVECTOR camera_up_, float fov_y) {
	for (int i = 0; i < max_lights_->GetData().x; i++) {
		// if point light update all 6
		if (lights_->GetData()[i].type != 3) {
			shad_maps_[lights_->GetData()[i].shad_map_index][0]->UpdateMatricies(camera_target, camera_pos, camera_up_, fov_y);
			if (lights_->GetData()[i].type == 1) {
				for (int j = 1; j < 6; j++) {
					shad_maps_[lights_->GetData()[i].shad_map_index][j]->UpdateMatricies(camera_target, camera_pos, camera_up_, fov_y);
				}
			}
		}
	}
}