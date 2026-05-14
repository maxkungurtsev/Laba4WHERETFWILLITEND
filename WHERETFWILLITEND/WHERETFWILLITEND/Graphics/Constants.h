#pragma once
#include <DirectXMath.h>
using namespace DirectX;
struct LightData {
	XMFLOAT3 strength;
	float falloff_start;
	XMFLOAT4 direction;
	XMFLOAT4 position;
	float falloff_end;
	float spot_power;
	int type; //0-dir, 1-point, 2-spot
	float velocity= 0;
	float spawn_time;
	float shad_map_index;
	float pad[2];
	XMFLOAT4 movement_direction;
};
struct shaderMaterialData {
	XMFLOAT3 ambient_;
	float shiny_;
	XMFLOAT3 diffuse_;
	// if has normal texture - 1; heigth -2; or none of those-0;
	float NormalType = 0;
	XMFLOAT3 spec_;
	float pad1 = 0;
};
struct PassConstants {
	XMFLOAT4X4 model;
	XMFLOAT4X4 inv_model;
	XMFLOAT4X4 view;
	XMFLOAT4X4 inv_view;
	XMFLOAT4X4 projection;
	XMFLOAT4X4 inv_projection;
	XMFLOAT4 cam_pos;
	XMFLOAT4 cam_forward;
	shaderMaterialData mats[300];
	float time;
	int current_mat;
	float pad2[2];
};