#include <vector>       
#include "geometry.h" 

// Model 类：用来加载和存储一个 3D 模型（三角形网格）
class Model {
    // 模型的顶点列表，每个顶点是一个 vec3 (x, y, z)
    std::vector<vec3> verts = {};

    // 每个三角形的顶点索引列表
    // 例如，一个三角形由 verts[1], verts[2], verts[5] 组成，
    // 那么 facet_vrt 中对应位置存的就是 [1, 2, 5]
    // 所以 facet_vrt.size() / 3 = 三角形数量
    std::vector<int> facet_vrt = {};

public:
    // 构造函数：根据文件名加载模型,通常是 .obj 等格式，里面存着顶点和三角形信息
    Model(const std::string filename);

    // 返回顶点总数
    int nverts() const;

    // 返回三角形总数
    int nfaces() const;

    // 返回第 i 个顶点的坐标，索引 i 必须满足 0 <= i < nverts()
    vec3 vert(const int i) const;

    // 返回第 iface 个三角形的第 nthvert 个顶点坐标
    // iface 必须满足 0 <= iface < nfaces()
    // nthvert 必须是 0, 1 或 2，对应三角形的三个顶点
    vec3 vert(const int iface, const int nthvert) const;
};
