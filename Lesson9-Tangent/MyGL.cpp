#include <algorithm>
#include "MyGL.h"

mat<4, 4> ModelView, Viewport, Perspective; // ȫ�־���ģ����ͼ���ӿڡ�͸��
std::vector<double> zbuffer;               // ȫ�� Z-buffer��������Ȳ���

// ------------------- ��������� -------------------
void lookat(const vec3 eye, const vec3 center, const vec3 up) {
    vec3 n = normalized(eye - center);          // ������������߷�����
    vec3 l = normalized(cross(up, n));          // �ҷ�������
    vec3 m = normalized(cross(n, l));           // �������Ϸ�������
    // ���� ModelView ������ת * ƽ��
    ModelView = mat<4, 4>{ {{l.x,l.y,l.z,0}, {m.x,m.y,m.z,0}, {n.x,n.y,n.z,0}, {0,0,0,1}} } *
        mat<4, 4>{{{1, 0, 0, -center.x}, { 0,1,0,-center.y }, { 0,0,1,-center.z }, { 0,0,0,1 }}};
}

// ------------------- ͸��ͶӰ���� -------------------
void init_perspective(const double f) {
    // ��͸�Ӿ��󣬽� z ӳ�䵽 [-1,1] ��Χ
    Perspective = { {{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,-1 / f,1}} };
}

// ------------------- �ӿھ��� -------------------
void init_viewport(const int x, const int y, const int w, const int h) {
    // ����׼���豸���꣨[-1,1]��ӳ�䵽��Ļ����
    Viewport = { {{w / 2., 0, 0, x + w / 2.},
                  {0, h / 2., 0, y + h / 2.},
                  {0,0,1,0},
                  {0,0,0,1}} };
}

// ------------------- Z-buffer ��ʼ�� -------------------
void init_zbuffer(const int width, const int height) {
    // ��ʼ��Ϊһ����С��ֵ����ʾ��Զ���
    zbuffer = std::vector(width * height, -1000.);
}

// ------------------- ��դ������ -------------------
void rasterize(const Triangle& clip, const IShader& shader, TGAImage& framebuffer) {
    // �������ζ���Ӳü��ռ��һ���� NDC �ռ�
    vec4 ndc[3] = { clip[0] / clip[0].w, clip[1] / clip[1].w, clip[2] / clip[2].w };
    // �� NDC ����ӳ�䵽��Ļ����
    vec2 screen[3] = { (Viewport * ndc[0]).xy(), (Viewport * ndc[1]).xy(), (Viewport * ndc[2]).xy() };

    // ���� 3x3 ���� ABC����������������
    mat<3, 3> ABC = { { {screen[0].x, screen[0].y, 1.},
                        {screen[1].x, screen[1].y, 1.},
                        {screen[2].x, screen[2].y, 1.} } };
    if (ABC.det() < 1) return; // ���޳� + �������С��һ�����ص�������

    // ���������εı߽��
    auto [bbminx, bbmaxx] = std::minmax({ screen[0].x, screen[1].x, screen[2].x });
    auto [bbminy, bbmaxy] = std::minmax({ screen[0].y, screen[1].y, screen[2].y });

    // �����߽������������
#pragma omp parallel for
    for (int x = std::max<int>(bbminx, 0); x <= std::min<int>(bbmaxx, framebuffer.width() - 1); x++) {
        for (int y = std::max<int>(bbminy, 0); y <= std::min<int>(bbmaxy, framebuffer.height() - 1); y++) {
            // ������Ļ��������
            vec3 bc_screen = ABC.invert_transpose() * vec3 { static_cast<double>(x), static_cast<double>(y), 1. };
            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue; // ������������

            // ͸������������Ļ�������굽�ü��ռ���������
            vec3 bc_clip = { bc_screen.x / clip[0].w, bc_screen.y / clip[1].w, bc_screen.z / clip[2].w };
            bc_clip = bc_clip / (bc_clip.x + bc_clip.y + bc_clip.z); // ��һ��

            // ��ֵ���ֵ�����Բ�ֵ NDC.z��
            double z = bc_screen * vec3{ ndc[0].z, ndc[1].z, ndc[2].z };
            if (z <= zbuffer[x + y * framebuffer.width()]) continue; // ��Ȳ���

            // ����ƬԪ��ɫ����ȡ��ɫ
            auto [discard, color] = shader.fragment(bc_clip);
            if (discard) continue; // �����ɫ������������

            // ���� Z-buffer ��֡����
            zbuffer[x + y * framebuffer.width()] = z;
            framebuffer.set(x, y, color);
        }
    }
}
