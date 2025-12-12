#pragma once
#include "vectors.h"
#include <vector>
class GourandShader {
private:
	Vec3f vertex_shader;
	Vec3f light_coords;
	float intensity;
public:
	GourandShader(Vec3f _light_coords, float _intensity);
	void Vertex(std::vector<Vec3f> positions, std::vector<Vec3f> normals, float ambient_k, float diffuse_k, float specular_k, float shiny_k);
	float Fragment(float w0, float w1, float w2);
};