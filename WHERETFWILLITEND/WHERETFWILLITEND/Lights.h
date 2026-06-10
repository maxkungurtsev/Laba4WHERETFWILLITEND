#pragma once
#include "Graphics\StructBuffer.h"
#include "Graphics\Constants.h"
#include "Graphics\CBuffer.h"
#include "ShadowMap.h"
#include <array>
//#include "ShadowMap.h"
class Lights {
	std::shared_ptr<StructBuffer<LightData>> lights_;
	std::shared_ptr <Cbuffer<XMFLOAT4>> max_lights_;
	std::shared_ptr<ShadowMap> casc_shad_map_;
	std::shared_ptr <Cbuffer<ShadowConstants>> shadow_constants_;
	std::shared_ptr<Gdevice> device_;
	std::shared_ptr<GTexture> ambient_cubemap_;
	std::shared_ptr<GTexture> dummy_shad_map_;
	std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> casc_shad_map_handles;
	std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> dummy_shad_map_handles;
public:
	Lights(std::shared_ptr<Gdevice> device);
	void AddSpotlight(XMFLOAT3 strength,float falloff_start, XMFLOAT4 direction, XMFLOAT4 position, float falloff_end, float spot_power, bool in_render_frame, float velocity, float spawn_time, XMFLOAT4 movement_direction);
	void RemoveLastLight(bool in_render_frame);
	void AddAmbientlight(XMFLOAT3 strength, std::string cubemap_pass, bool in_render_frame);
	void AddAmbientlight(XMFLOAT3 strength, bool in_render_frame);
	void AddDirlight(XMFLOAT3 strength, XMFLOAT4 direction, bool in_render_frame);
	void AddPointlight(XMFLOAT3 strength, XMFLOAT4 position, float falloff_start, float falloff_end, bool in_render_frame, float velocity, float spawn_time, XMFLOAT4 movement_direction);
	std::shared_ptr<ShadowMap> GetShadowMap() { return casc_shad_map_; }
	std::vector<D3D12_GPU_DESCRIPTOR_HANDLE>& GetShadowMapHandles();
	std::shared_ptr<StructBuffer<LightData>> GetBuffer();
	std::shared_ptr <Cbuffer<XMFLOAT4>> GetMaxLights();
	std::shared_ptr <Cbuffer<ShadowConstants>> GetShadowConstants() { return shadow_constants_; }
	std::shared_ptr<GTexture> GetAmbCubeMap() { return ambient_cubemap_; }

	void UpdateShadowMatricies(XMMATRIX& cameraView, float cameraFovY, float cameraAspect);
};