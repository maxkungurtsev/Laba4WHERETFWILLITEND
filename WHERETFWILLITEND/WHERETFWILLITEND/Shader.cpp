#include "Shader.h"
#include <iostream>
GourandShader::GourandShader(Vec3f _light_coords, float _intensity): light_coords(_light_coords), intensity(_intensity) {};
void GourandShader::Vertex(std::vector<Vec3f> positions, std::vector<Vec3f> normals, float ambient_k, float diffuse_k, float specular_k, float shiny_k) {
	for (int i = 0; i < 3; i++) {
		Vec3f pos = positions[i];
		Vec3f norm = (normals[i]).normalize();
		Vec3f v = (pos).normalize()*-1;
		Vec3f l = (light_coords - pos).normalize();
		Vec3f r =  norm*2* std::max(float(0), l*norm) - l;
		float Am = ambient_k * intensity;
		float D = intensity * diffuse_k * std::max(float(0), l * norm);
		float Spec = specular_k * intensity * std::pow(std::max(0.0f, std::max(float(0), v * r)), shiny_k);
		switch (i) {
			case 0:
				vertex_shader.x = Am + D + Spec;
				break;
			case 1:
				vertex_shader.y = Am + D + Spec;
				break;
			case 2:
				vertex_shader.z = Am + D + Spec;
				break;
		}
	}
}
float GourandShader::Fragment(float w0, float w1, float w2) {
	return vertex_shader.x * w0 + vertex_shader.y * w1 + vertex_shader.z * w2;
}