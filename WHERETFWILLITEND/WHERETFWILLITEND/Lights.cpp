#include "Lights.h"
Lights::Lights(std::shared_ptr<Gdevice> device) {
	lights_ =std::make_shared<StructBuffer<LightData>>(device, 200, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	max_lights_ = std::make_shared<Cbuffer<XMFLOAT4>>(device);
	max_lights_->GetData() = XMFLOAT4{0,0,0,0};
	device_ = device;
	TGAImage dummy_;
	dummy_.read_tga_file("dummy.tga");
	std::string name = "dummy_sm";
	dummy_shad_map_ = std::make_shared<GTexture>(dummy_, name, device, TextureUsage::Depth);
	dummy_shad_map_handles = { dummy_shad_map_->GetResourse()->GetHandle().gpu_, dummy_shad_map_->GetResourse()->GetHandle().gpu_, dummy_shad_map_->GetResourse()->GetHandle().gpu_, dummy_shad_map_->GetResourse()->GetHandle().gpu_};
	shadow_constants_ = std::make_shared<Cbuffer<ShadowConstants>>(device);
	for (int i = 0; i < 4; ++i) {
		XMStoreFloat4x4(&shadow_constants_->GetData().view_proj_mat[i], XMMatrixIdentity());
	}
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


std::vector<D3D12_GPU_DESCRIPTOR_HANDLE>& Lights::GetShadowMapHandles() {
	if (casc_shad_map_handles.size() == 0) {
		OutputDebugStringA("shad maps empty!\n");
		return dummy_shad_map_handles;
	}
	return casc_shad_map_handles;
};

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
	XMVECTOR pos = -dir*2000.0f; 
	const float aspect_ratio = static_cast<float>(device_->width_) / static_cast<float>(device_->height_);
	sm = std::make_shared<ShadowMap>(4096, 4096, 4, device_, pos, XMVectorSet(0, 0, 0, 0), aspect_ratio, 0.75f);
	casc_shad_map_=sm;
	casc_shad_map_handles.clear();
	casc_shad_map_handles.push_back(casc_shad_map_->GetSRVHandle(0).gpu_);
	casc_shad_map_handles.push_back(casc_shad_map_->GetSRVHandle(1).gpu_);
	casc_shad_map_handles.push_back(casc_shad_map_->GetSRVHandle(2).gpu_);
	casc_shad_map_handles.push_back(casc_shad_map_->GetSRVHandle(3).gpu_);

	shadow_constants_->GetData().view_proj_mat[0] = casc_shad_map_->GetCascade(0)->GetViewProj();
	shadow_constants_->GetData().view_proj_mat[1] = casc_shad_map_->GetCascade(1)->GetViewProj();
	shadow_constants_->GetData().view_proj_mat[2] = casc_shad_map_->GetCascade(2)->GetViewProj();
	shadow_constants_->GetData().view_proj_mat[3] = casc_shad_map_->GetCascade(3)->GetViewProj();
	shadow_constants_->Save_changes();
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


void Lights::UpdateShadowMatricies(XMMATRIX& cameraView, XMMATRIX& cameraProj, float cameraFovY, float cameraAspect) {
	for (int i = 0; i < max_lights_->GetData().x; i++) {
		if (lights_->GetData()[i].type == 0) {
			casc_shad_map_->UpdateMatricies(cameraView, cameraProj, cameraFovY, cameraAspect);
			for (int cascade = 0; cascade < 4; ++cascade) {
				std::shared_ptr<Cascade> cascade_data = casc_shad_map_->GetCascade(cascade);
				shadow_constants_->GetData().view_proj_mat[cascade] = cascade_data->GetViewProj();
				(&shadow_constants_->GetData().split_depths.x)[cascade] = cascade_data->GetSplitDepth();
			}
			OutputDebugStringA((std::to_string(shadow_constants_->GetData().split_depths.x) + " " + std::to_string(shadow_constants_->GetData().split_depths.y) + " " + std::to_string(shadow_constants_->GetData().split_depths.z) + " " + std::to_string(shadow_constants_->GetData().split_depths.w)).c_str());
			shadow_constants_->Save_changes();
		}
	}
}