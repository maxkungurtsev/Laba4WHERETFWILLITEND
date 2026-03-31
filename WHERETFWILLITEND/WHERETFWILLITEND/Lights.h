#pragma once
#include "Graphics\StructBuffer.h"
#include "Graphics\Constants.h"
#include "Graphics\CBuffer.h"
class Lights {
	std::shared_ptr<StructBuffer<LightData>> lights_;
	std::shared_ptr <Cbuffer<XMFLOAT4>> max_lights_;
public:
	Lights(std::shared_ptr<Gdevice> device);
	void AddSpotlight(XMFLOAT3 strength,float falloff_start, XMFLOAT4 direction, XMFLOAT4 position, float falloff_end, float spot_power, bool in_render_frame, float velocity, float spawn_time, XMFLOAT4 movement_direction);
	void RemoveLastLight(bool in_render_frame);
	void AddAmbientlight(XMFLOAT3 strength, bool in_render_frame);
	void AddDirlight(XMFLOAT3 strength, XMFLOAT4 direction, bool in_render_frame);
	void AddPointlight(XMFLOAT3 strength, XMFLOAT4 position, float falloff_start, float falloff_end, bool in_render_frame, float velocity, float spawn_time, XMFLOAT4 movement_direction);
	std::shared_ptr<StructBuffer<LightData>> GetBuffer();
	std::shared_ptr <Cbuffer<XMFLOAT4>> GetMaxLights();
};