#include "Model.h"
#include <fstream>
#include <sstream>
Model::Model(const std::string* filename, const std::string* texture_filename) : verts_(), faces_(), textures_(), normals_(), diffuse_texture_() {
	std::ifstream in(*filename);
	if (!in.is_open()) {
		std::cerr << "Couldn't open file: " << *filename << '\n';
		return;
	}
	std::string line;
	while (std::getline(in, line)) {
		char trash;
		std::istringstream iss(line);
		if (!line.compare(0, 2, "v ")) {
			Vec3f v;
			iss >> trash >> v.x >> v.y >> v.z;
			verts_.push_back(v);
		}else if (!line.compare(0, 2, "vt")){
			Vec2f vt;
			iss >> trash >> trash >> vt.x >> vt.y;
			textures_.push_back(vt);

		}
		else if (!line.compare(0, 2, "vn")) {
			Vec3f vn;
			iss >> trash >> trash >> vn.x >> vn.y >> vn.z;
			normals_.push_back(vn);

		}
		else if (!line.compare(0, 2, "f ")) {
			std::vector<int> f = { 0,0,0,0,0,0,0,0,0 };
			int itrash;
			int v;
			int vt;
			int vn;
			iss >> trash;
			for (int i = 0; i < 3; i++) {
				iss >>v>>trash>> vt >>trash>> vn;
				f[3*i] = v - 1;
				f[3 * i+1] = vt - 1;
				f[3 * i + 2] = vn - 1;
			}
			
			faces_.push_back(f);
		}
	}
	std::cout << "verts amount: " << verts_.size() << "faces amount: " << faces_.size() << '\n';
	in.close();
	in.open(*texture_filename);
	if (!in.is_open()) {
		std::cerr << "Couldn't open file: " << *texture_filename << '\n';
		return;
	}
	diffuse_texture_.read_tga_file( (*texture_filename).c_str() );

}
Model::Model(const std::string* filename) : verts_(), faces_(), textures_(), normals_(), diffuse_texture_() {
	std::ifstream in(*filename);
	if (!in.is_open()) {
		std::cerr << "Couldn't open file: " << *filename << '\n';
		return;
	}
	std::string line;
	while (std::getline(in, line)) {
		char trash;
		std::istringstream iss(line);
		if (!line.compare(0, 2, "v ")) {
			Vec3f v;
			iss >> trash >> v.x >> v.y >> v.z;
			verts_.push_back(v);
		}
		else if (!line.compare(0, 2, "vt")) {
			Vec2f vt;
			iss >> trash >> trash >> vt.x >> vt.y;
			textures_.push_back(vt);

		}
		else if (!line.compare(0, 2, "vn")) {
			Vec3f vn;
			iss >> trash >> trash >> vn.x >> vn.y >> vn.z;
			normals_.push_back(vn);

		}
		else if (!line.compare(0, 2, "f ")) {
			std::vector<int> f = { 0,0,0,0,0,0,0,0,0 };
			int itrash;
			int v;
			int vt;
			int vn;
			iss >> trash;
			for (int i = 0; i < 3; i++) {
				iss >> v >> trash >> vt >> trash >> vn;
				f[3 * i] = v - 1;
				f[3 * i + 1] = vt - 1;
				f[3 * i + 2] = vn - 1;
			}

			faces_.push_back(f);
		}
	}
	std::cout << "verts amount: " << verts_.size() << "faces amount: " << faces_.size() << '\n';
}
int Model::Get_verts_amount(){
	return verts_.size();
}
int Model::Get_faces_amount() {
	return faces_.size();
}
int Model::Get_textures_amount() {
	return textures_.size();
}
int Model::Get_normals_amount() {
	return normals_.size();
}
Vec3f Model::Get_vert(int i) {
	return verts_[i];
}
Vec2f Model::Get_vertex_texture(int i) {
	return textures_[i];
}
TGAImage& Model::Get_texture() {
	return diffuse_texture_;
}
Vec3f Model::Get_normal(int i) {
	return normals_[i];
}
std::vector<int> Model::Get_face(int i) {
	return faces_[i];
};