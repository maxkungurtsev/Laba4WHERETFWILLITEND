#include "Model.h"
#include <fstream>
#include <sstream>

Model::Model(const std::string& filename, const std::string& texture_filename)
{
    std::ifstream in(filename);
    if (!in.is_open()) {
        std::cerr << "Couldn't open file: " << filename << '\n';
        return;
    }
    std::string line;
    while (std::getline(in, line)) {
        std::istringstream iss(line);
        if (line.rfind("v ", 0) == 0) {
            char trash;
            XMFLOAT3 pos;
            iss >> trash >> pos.x >> pos.y >> pos.z;
            positions_.push_back(pos);
        }
        else if (line.rfind("vt ", 0) == 0) {
            char trash1, trash2;
            XMFLOAT2 uv;
            iss >> trash1 >> trash2 >> uv.x >> uv.y;
            texcoords_.push_back(uv);
        }
        else if (line.rfind("vn ", 0) == 0) {
            char trash1, trash2;
            XMFLOAT3 n;
            iss >> trash1 >> trash2 >> n.x >> n.y >> n.z;
            normals_.push_back(n);
        }
        else if (line.rfind("f ", 0) == 0) {
            char trash;
            iss >> trash;

            for (int i = 0; i < 3; i++) {
                int v, vt, vn;
                char slash;

                iss >> v >> slash >> vt >> slash >> vn;

                Vertex vert;
                vert.position = positions_[v - 1];
                vert.uv = texcoords_[vt - 1];
                vert.normal = normals_[vn - 1];

                vertices_.push_back(vert);
            }
        }
    }

    in.close();
    std::cout << "Loaded model: "<< vertices_.size()<<" vertices\n";
    if (!diffuse_texture_.read_tga_file(texture_filename.c_str())) {
        std::cerr << "Couldn't load texture: " << texture_filename << '\n';
    }
}
Model::Model(const std::string& filename)
{
    std::ifstream in(filename);
    if (!in.is_open()) {
        std::cerr << "Couldn't open file: " << filename << '\n';
        return;
    }
    std::string line;
    while (std::getline(in, line)) {
        std::istringstream iss(line);
        if (line.rfind("v ", 0) == 0) {
            char trash;
            XMFLOAT3 pos;
            iss >> trash >> pos.x >> pos.y >> pos.z;
            positions_.push_back(pos);
        }
        else if (line.rfind("vt ", 0) == 0) {
            char trash1, trash2;
            XMFLOAT2 uv;
            iss >> trash1 >> trash2 >> uv.x >> uv.y;
            texcoords_.push_back(uv);
        }
        else if (line.rfind("vn ", 0) == 0) {
            char trash1, trash2;
            XMFLOAT3 n;
            iss >> trash1 >> trash2 >> n.x >> n.y >> n.z;
            normals_.push_back(n);
        }
        else if (line.rfind("f ", 0) == 0) {
            char trash;
            iss >> trash;

            for (int i = 0; i < 3; i++) {
                int v, vt, vn;
                char slash;

                iss >> v >> slash >> vt >> slash >> vn;

                Vertex vert;
                vert.position = positions_[v - 1];
                vert.uv = texcoords_[vt - 1];
                vert.normal = normals_[vn - 1];

                vertices_.push_back(vert);
            }
        }
    }

    in.close();
    std::cout << "Loaded model: " << vertices_.size() << " vertices\n";
}