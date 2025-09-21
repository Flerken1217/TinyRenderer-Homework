#include <vector>       
#include "geometry.h" 

// Model �ࣺ�������غʹ洢һ�� 3D ģ�ͣ�����������
class Model {
    // ģ�͵Ķ����б�ÿ��������һ�� vec3 (x, y, z)
    std::vector<vec3> verts = {};

    // ÿ�������εĶ��������б�
    // ���磬һ���������� verts[1], verts[2], verts[5] ��ɣ�
    // ��ô facet_vrt �ж�Ӧλ�ô�ľ��� [1, 2, 5]
    // ���� facet_vrt.size() / 3 = ����������
    std::vector<int> facet_vrt = {};

public:
    // ���캯���������ļ�������ģ��,ͨ���� .obj �ȸ�ʽ��������Ŷ������������Ϣ
    Model(const std::string filename);

    // ���ض�������
    int nverts() const;

    // ��������������
    int nfaces() const;

    // ���ص� i ����������꣬���� i �������� 0 <= i < nverts()
    vec3 vert(const int i) const;

    // ���ص� iface �������εĵ� nthvert ����������
    // iface �������� 0 <= iface < nfaces()
    // nthvert ������ 0, 1 �� 2����Ӧ�����ε���������
    vec3 vert(const int iface, const int nthvert) const;
};
