#include <string>
#include "geometry.h"
#include "tgaimage.h"

class Model {
    // 顶点坐标 (v)
    std::vector<vec4> verts = {};

    // 顶点法线 (vn)
    std::vector<vec4> norms = {};

    // 顶点纹理坐标 (vt)
    std::vector<vec2> tex = {};

    // 三角形索引：顶点、法线、纹理坐标
    std::vector<int> facet_vrt = {}; // 每个三角形的顶点索引 (3 * nfaces)
    std::vector<int> facet_nrm = {}; // 每个三角形的法线索引 (3 * nfaces)
    std::vector<int> facet_tex = {}; // 每个三角形的纹理索引 (3 * nfaces)

    // 贴图
    TGAImage diffusemap = {};  // 漫反射贴图
    TGAImage normalmap = {};  // 法线贴图
    TGAImage specularmap = {};  // 高光贴图

public:
    // 构造函数：读取 .obj 模型文件
    Model(const std::string filename);

    // 模型统计
    int nverts() const; // 顶点数
    int nfaces() const; // 三角形数

    // 顶点访问
    vec4 vert(const int i) const;                        // 返回第 i 个顶点
    vec4 vert(const int iface, const int nthvert) const; // 返回第 iface 个三角形的第 nthvert 个顶点

    // 法线访问
    vec4 normal(const int iface, const int nthvert) const; // 从 .obj 文件 vn 获取
    vec4 normal(const vec2& uv) const;                     // 从 normal map 贴图获取

    // 纹理坐标访问
    vec2 uv(const int iface, const int nthvert) const;

    // 贴图访问
    const TGAImage& diffuse() const;
    const TGAImage& specular() const;
};
