#include "modelLoader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>

Model::Model(const std::string filename) {
    std::ifstream in(filename);
    if (!in) {
        std::cerr << "Error: cannot open " << filename << std::endl;
        return;
    }

    std::string line;
    while (std::getline(in, line)) {
        std::istringstream iss(line);
        char trash;

        // �����У�v x y z
        if (line.substr(0, 2) == "v ") {
            iss >> trash; // ���� 'v'
            float x, y, z;
            iss >> x >> y >> z;
            verts.push_back(vec3(x, y, z));
        }
        // ���У�f i j k (ע�� OBJ ������ 1 ��ʼ)
        else if (line.substr(0, 2) == "f ") {
            iss >> trash; // ���� 'f'
            int idx[3];
            for (int i = 0; i < 3; i++) {
                std::string vertex_str;
                iss >> vertex_str;
                // ����� i/j/k ��ʽ��ֻȡ i
                idx[i] = std::stoi(vertex_str) - 1; // תΪ 0 ������
            }
            facet_vrt.push_back(idx[0]);
            facet_vrt.push_back(idx[1]);
            facet_vrt.push_back(idx[2]);
        }
        // �����к���
    }

    std::cout << "Loaded " << nverts() << " vertices and "
        << nfaces() << " faces from " << filename << std::endl;
}

int Model::nverts() const {
    return (int)verts.size();
}

int Model::nfaces() const {
    return (int)(facet_vrt.size() / 3);
}

vec3 Model::vert(const int i) const {
    return verts[i];
}

vec3 Model::vert(const int iface, const int nthvert) const {
    return verts[facet_vrt[iface * 3 + nthvert]];
}
