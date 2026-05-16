#include "Lights.h"
Lights::Lights(std::shared_ptr<Gdevice> device) {
	lights_ =std::make_shared<StructBuffer<LightData>>(device, 200, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	max_lights_ = std::make_shared<Cbuffer<XMFLOAT4>>(device);
	max_lights_->GetData() = XMFLOAT4{0,0,0,0};
	device_ = device;
	current_viewProj = std::make_shared<Cbuffer<XMFLOAT4X4>>(device);
	shadow_constants_ = std::make_shared<Cbuffer<ShadowConstants>>(device);
	XMStoreFloat4x4(&current_viewProj->GetData(), XMMatrixIdentity());
	for (int i = 0; i < 4; ++i) {
		XMStoreFloat4x4(&shadow_constants_->GetData().viewProj_mat[i], XMMatrixIdentity());
	}
	current_viewProj->Save_changes();
	shadow_constants_->Save_changes();
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
	lights_->AddElement(newlight, max_lights_->GetData().x, in_render_frame);
	max_lights_->GetData().x += 1;
	max_lights_->Save_changes();
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
	lights_->AddElement(newlight, max_lights_->GetData().x, in_render_frame);
	max_lights_->GetData().x += 1;
	max_lights_->Save_changes();
    // shad map init
	std::shared_ptr<ShadowMap> sm;
	XMVECTOR dir = XMLoadFloat4(&direction);
	XMVECTOR pos = -dir*5000.0f;
	sm = std::make_shared<ShadowMap>(1024, 1024, 4, device_, pos, XMVectorSet(0, 0, 0, 1), 1.0f, 0.75f);
	casc_shad_map_=sm;
	casc_shad_map_handles.clear();
	casc_shad_map_handles.push_back(casc_shad_map_->GetSRVHandle(0).gpu_);
	casc_shad_map_handles.push_back(casc_shad_map_->GetSRVHandle(1).gpu_);
	casc_shad_map_handles.push_back(casc_shad_map_->GetSRVHandle(2).gpu_);
	casc_shad_map_handles.push_back(casc_shad_map_->GetSRVHandle(3).gpu_);
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
	lights_->AddElement(newlight, max_lights_->GetData().x, in_render_frame);
	max_lights_->GetData().x += 1;
	max_lights_->Save_changes();
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


void Lights::SetViewProj(XMFLOAT4X4 new_view_proj) {
	current_viewProj->GetData() = new_view_proj;
	current_viewProj->Save_changes();
};

void Lights::UpdateShadowMatricies(XMVECTOR camera_target, XMVECTOR camera_pos, XMVECTOR camera_up_, float fov_y) {
	for (int i = 0; i < max_lights_->GetData().x; i++) {
		if (lights_->GetData()[i].type == 0) {
			casc_shad_map_->UpdateMatricies(camera_target, camera_pos, camera_up_, fov_y);
			for (int cascade = 0; cascade < 4; ++cascade) {
				shadow_constants_->GetData().viewProj_mat[cascade] = casc_shad_map_->GetCascade(cascade)->GetViewProj();
			}
			shadow_constants_->Save_changes();
		}
	}
}