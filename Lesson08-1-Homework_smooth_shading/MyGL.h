#include "tgaimage.h" 
#include "geometry.h"   

// ����������ӽǵĺ���
void lookat(const vec3 eye, const vec3 center, const vec3 up);

// ��ʼ��͸��ͶӰ����
void init_perspective(const double f);

// ��ʼ���ӿھ�����Ļ����ӳ�䣩
void init_viewport(const int x, const int y, const int w, const int h);

// ��ʼ����Ȼ��棨Z-buffer��
void init_zbuffer(const int width, const int height);

// �������ɫ���ӿڣ�����ƬԪ��ɫ����
struct IShader {
    virtual std::pair<bool, TGAColor> fragment(const vec3 bar) const = 0;
    // ����ֵ first ��ʾ�Ƿ�������ƬԪ��second ��ʾ��ɫ
};

// �������������ͣ��������������ά�����
typedef vec4 Triangle[3];

// ���Ĺ�դ����������������ƬԪ���Ƶ�֡����
void rasterize(const Triangle& clip, const IShader& shader, TGAImage& framebuffer);