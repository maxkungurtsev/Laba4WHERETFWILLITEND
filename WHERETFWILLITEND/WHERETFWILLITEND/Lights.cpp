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
		XMStoreFloat4x4(&shadow_constants_->GetData().shad_view[i], XMMatrixIdentity());
		XMStoreFloat4x4(&shadow_constants_->GetData().shad_proj[i], XMMatrixIdentity());
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
	float distance = 5000.0f;
	XMVECTOR pos = -dir* distance;
	sm = std::make_shared<ShadowMap>(4096, 4096, 4, device_, pos, XMVectorSet(0, 0, 0, 0), distance, 0.75f);
	casc_shad_map_=sm;
	casc_shad_map_handles.clear();
	casc_shad_map_handles.push_back(casc_shad_map_->GetSRVHandle(0).gpu_);
	casc_shad_map_handles.push_back(casc_shad_map_->GetSRVHandle(1).gpu_);
	casc_shad_map_handles.push_back(casc_shad_map_->GetSRVHandle(2).gpu_);
	casc_shad_map_handles.push_back(casc_shad_map_->GetSRVHandle(3).gpu_);

	shadow_constants_->GetData().shad_view[0] = casc_shad_map_->GetCascade(0)->GetView();
	shadow_constants_->GetData().shad_view[1] = casc_shad_map_->GetCascade(1)->GetView();
	shadow_constants_->GetData().shad_view[2] = casc_shad_map_->GetCascade(2)->GetView();
	shadow_constants_->GetData().shad_view[3] = casc_shad_map_->GetCascade(3)->GetView();

	shadow_constants_->GetData().shad_proj[0] = casc_shad_map_->GetCascade(0)->GetProj();
	shadow_constants_->GetData().shad_proj[1] = casc_shad_map_->GetCascade(1)->GetProj();
	shadow_constants_->GetData().shad_proj[2] = casc_shad_map_->GetCascade(2)->GetProj();
	shadow_constants_->GetData().shad_proj[3] = casc_shad_map_->GetCascade(3)->GetProj();
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


void Lights::UpdateShadowMatricies(XMMATRIX& cameraView, float cameraFovY, float cameraAspect) {
	for (int i = 0; i < max_lights_->GetData().x; i++) {
		if (lights_->GetData()[i].type == 0) {
			casc_shad_map_->UpdateMatricies(cameraView, cameraFovY, cameraAspect);
			for (int cascade = 0; cascade < 4; ++cascade) {
				std::shared_ptr<Cascade> cascade_data = casc_shad_map_->GetCascade(cascade);
				shadow_constants_->GetData().shad_view[cascade] = cascade_data->GetView();
				shadow_constants_->GetData().shad_proj[cascade] = cascade_data->GetProj();
				switch (cascade) {
				case 0:
					shadow_constants_->GetData().split_depths.x = cascade_data->GetSplitDepth();
					break;
				case 1:
					shadow_constants_->GetData().split_depths.y = cascade_data->GetSplitDepth();
					break;
				case 2:
					shadow_constants_->GetData().split_depths.z = cascade_data->GetSplitDepth();
					break;
				case 3:
					shadow_constants_->GetData().split_depths.w = cascade_data->GetSplitDepth();
					break;
				}
			}
			OutputDebugStringA(("split depths: "+std::to_string(shadow_constants_->GetData().split_depths.x) + " " + std::to_string(shadow_constants_->GetData().split_depths.y) + " " + std::to_string(shadow_constants_->GetData().split_depths.z) + " " + std::to_string(shadow_constants_->GetData().split_depths.w) + '\n').c_str());
			shadow_constants_->Save_changes();
		}
	}
}