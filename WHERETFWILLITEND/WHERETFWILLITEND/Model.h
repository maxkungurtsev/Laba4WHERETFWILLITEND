#pragma once
#include <vector>
#include <string>
#include "vectors.h"
#include "tgaimage.h"

class Model {
	private:
		std::vector<Vec3f> verts_;
		std::vector<Vec2f> textures_;
		std::vector<Vec3f> normals_;
		std::vector<std::vector<int>> faces_;
		TGAImage diffuse_texture_;
	public:
		Model(const std::string* obj_filename, const std::string* texture_filename);
		Model(const std::string* obj_filename);
		int Get_verts_amount();
		int Get_faces_amount();
		int Get_textures_amount();
		int Get_normals_amount();
		Vec3f Get_vert(int i);
		Vec2f Get_vertex_texture(int i);
		TGAImage& Get_texture();
		Vec3f Get_normal(int i);
		std::vector<int> Get_face(int i);
};