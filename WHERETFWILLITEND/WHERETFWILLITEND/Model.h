#pragma once
#include <vector>
#include <string>
#include <iostream>
#include "tgaimage.h"
#include <DirectXMath.h>
using namespace DirectX;
struct Vertex
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT2 uv;
};
class Model {
	private:
		std::vector<XMFLOAT3> positions_;
		std::vector<XMFLOAT2> texcoords_;
		std::vector<XMFLOAT3> normals_;
		std::vector<Vertex> vertices_;
		TGAImage diffuse_texture_;
	public:
		Model(const std::string& filename, const std::string& texture_filename);
		Model(const std::string& filename);
		const std::vector<Vertex>& GetVertices() const { return vertices_; }
		TGAImage& GetTexture() { return diffuse_texture_; }
};