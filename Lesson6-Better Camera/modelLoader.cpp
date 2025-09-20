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

        // 顶点行：v x y z
        if (line.substr(0, 2) == "v ") {
            iss >> trash; // 读掉 'v'
            float x, y, z;
            iss >> x >> y >> z;
            verts.push_back(vec3(x, y, z));
        }
        // 面行：f i j k (注意 OBJ 索引从 1 开始)
        else if (line.substr(0, 2) == "f ") {
            iss >> trash; // 读掉 'f'
            int idx[3];
            for (int i = 0; i < 3; i++) {
                std::string vertex_str;
                iss >> vertex_str;
                // 如果是 i/j/k 形式，只取 i
                idx[i] = std::stoi(vertex_str) - 1; // 转为 0 基索引
            }
            facet_vrt.push_back(idx[0]);
            facet_vrt.push_back(idx[1]);
            facet_vrt.push_back(idx[2]);
        }
        // 其他行忽略
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
