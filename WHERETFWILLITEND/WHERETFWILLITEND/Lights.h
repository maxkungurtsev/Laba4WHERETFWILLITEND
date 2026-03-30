#pragma once
#include "Graphics\StructBuffer.h"
#include "Graphics\Constants.h"
#include "Graphics\CBuffer.h"
class Lights {
	std::shared_ptr<StructBuffer<LightData>> lights_;
	std::shared_ptr <Cbuffer<XMFLOAT4>> max_lights_;
public:
	Lights(std::shared_ptr<Gdevice> device);
	void Addlight(XMFLOAT3 strength,float falloff_start, XMFLOAT4 direction, XMFLOAT4 position, float falloff_end, float spot_power, int type, float velocity=0);
	void RemoveLastLight();
	void AddAmbientlight(XMFLOAT3 strength);
	std::shared_ptr<StructBuffer<LightData>> GetBuffer();
	std::shared_ptr <Cbuffer<XMFLOAT4>> GetMaxLights();
};