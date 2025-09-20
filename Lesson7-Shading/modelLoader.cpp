#include "modelLoader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <algorithm>

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
            iss >> trash; // 'v'
            float x, y, z;
            iss >> x >> y >> z;
            verts.push_back(vec4(x, y, z, 1.0f));
        }
        // 法线行：vn x y z
        else if (line.substr(0, 3) == "vn ") {
            iss >> trash >> trash; // "vn"
            float x, y, z;
            iss >> x >> y >> z;
            norms.push_back(vec4(x, y, z, 0.0f));
        }
        // 纹理坐标行：vt u v
        else if (line.substr(0, 3) == "vt ") {
            iss >> trash >> trash; // "vt"
            float u, v;
            iss >> u >> v;
            tex.push_back(vec2(u, v));
        }
        // 面行：f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3
        else if (line.substr(0, 2) == "f ") {
            iss >> trash; // 'f'
            for (int i = 0; i < 3; i++) {
                std::string vertex_str;
                iss >> vertex_str;

                // 解析 "v/vt/vn"
                int v_idx = -1, vt_idx = -1, vn_idx = -1;
                std::replace(vertex_str.begin(), vertex_str.end(), '/', ' ');
                std::istringstream viss(vertex_str);
                viss >> v_idx >> vt_idx >> vn_idx;

                // obj 索引是从 1 开始的，所以要减 1
                facet_vrt.push_back(v_idx - 1);
                facet_tex.push_back(vt_idx - 1);
                facet_nrm.push_back(vn_idx - 1);
            }
        }
        // 其他行忽略
    }

    std::cout << "Loaded " << nverts() << " vertices, "
        << norms.size() << " normals, "
        << tex.size() << " texcoords, "
        << nfaces() << " faces from "
        << filename << std::endl;
}

int Model::nverts() const {
    return (int)verts.size();
}

int Model::nfaces() const {
    return (int)(facet_vrt.size() / 3);
}

vec4 Model::vert(const int i) const {
    return verts[i];
}

vec4 Model::vert(const int iface, const int nthvert) const {
    return verts[facet_vrt[iface * 3 + nthvert]];
}

vec4 Model::normal(const int iface, const int nthvert) const {
    return norms[facet_nrm[iface * 3 + nthvert]];
}

vec2 Model::uv(const int iface, const int nthvert) const {
    return tex[facet_tex[iface * 3 + nthvert]];
}
