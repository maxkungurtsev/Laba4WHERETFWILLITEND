#pragma once
#include "Graphics\StructBuffer.h"
#include "Graphics\Constants.h"
class Lights {
	std::shared_ptr<StructBuffer<LightData>> lights_;
public:
	Lights(std::shared_ptr<Gdevice> device);
	void Addlight(XMFLOAT3 strength,float falloff_start, XMFLOAT4 direction, XMFLOAT4 position, float falloff_end, float spot_power, int type, float velocity=0);
	void RemoveLastLight();
	void AddAmbientlight(XMFLOAT3 strength);
	std::shared_ptr<StructBuffer<LightData>> GetBuffer();
};