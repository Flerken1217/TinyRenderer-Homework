#include <string>
#include "geometry.h"
#include "tgaimage.h"

class Model {
    // �������� (v)
    std::vector<vec4> verts = {};

    // ���㷨�� (vn)
    std::vector<vec4> norms = {};

    // ������������ (vt)
    std::vector<vec2> tex = {};

    // ���������������㡢���ߡ���������
    std::vector<int> facet_vrt = {}; // ÿ�������εĶ������� (3 * nfaces)
    std::vector<int> facet_nrm = {}; // ÿ�������εķ������� (3 * nfaces)
    std::vector<int> facet_tex = {}; // ÿ�������ε��������� (3 * nfaces)

    // ��ͼ
    TGAImage diffusemap = {};  // ��������ͼ
    TGAImage normalmap = {};  // ������ͼ
    TGAImage specularmap = {};  // �߹���ͼ

public:
    // ���캯������ȡ .obj ģ���ļ�
    Model(const std::string filename);

    // ģ��ͳ��
    int nverts() const; // ������
    int nfaces() const; // ��������

    // �������
    vec4 vert(const int i) const;                        // ���ص� i ������
    vec4 vert(const int iface, const int nthvert) const; // ���ص� iface �������εĵ� nthvert ������

    // ���߷���
    vec4 normal(const int iface, const int nthvert) const; // �� .obj �ļ� vn ��ȡ
    vec4 normal(const vec2& uv) const;                     // �� normal map ��ͼ��ȡ

    // �����������
    vec2 uv(const int iface, const int nthvert) const;

    // ��ͼ����
    const TGAImage& diffuse() const;
    const TGAImage& specular() const;
};
