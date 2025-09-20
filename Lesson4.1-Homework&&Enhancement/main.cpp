#include <cmath>
#include <tuple>
#include <iostream>
#include "geometry.h"
#include "modelLoader.h"
#include "tgaimage.h"

constexpr int width = 800;
constexpr int height = 800;

// �����з��������������������������
double signed_triangle_area(int ax, int ay, int bx, int by, int cx, int cy) {
    return .5 * ((by - ay) * (bx + ax) + (cy - by) * (cx + bx) + (ay - cy) * (ax + cx));
}

// ���������Σ���ȻҶȣ��� z-buffer��
void triangle(int ax, int ay, int az,
    int bx, int by, int bz,
    int cx, int cy, int cz,
    TGAImage& zbuffer,
    TGAImage& framebuffer)
{
    int bbminx = std::min({ ax, bx, cx });
    int bbminy = std::min({ ay, by, cy });
    int bbmaxx = std::max({ ax, bx, cx });
    int bbmaxy = std::max({ ay, by, cy });
    double total_area = signed_triangle_area(ax, ay, bx, by, cx, cy);
    if (total_area < 1) return;

#pragma omp parallel for
    for (int x = bbminx; x <= bbmaxx; x++) {
        for (int y = bbminy; y <= bbmaxy; y++) {
            double alpha = signed_triangle_area(x, y, bx, by, cx, cy) / total_area;
            double beta = signed_triangle_area(x, y, cx, cy, ax, ay) / total_area;
            double gamma = signed_triangle_area(x, y, ax, ay, bx, by) / total_area;
            if (alpha < 0 || beta < 0 || gamma < 0) continue;

            // ��ֵ z
            float z = alpha * az + beta * bz + gamma * cz;

            // z-buffer �ж�
            if (z <= zbuffer.get(x, y)[0]) continue;

            TGAColor depth_color((uint8_t)z); // �Ҷ���ɫ
            zbuffer.set(x, y, depth_color);   // ���� z-buffer
            framebuffer.set(x, y, depth_color); // ������ȻҶ�
        }
    }
}

// ͶӰ�������� [-1,1]^3 ӳ�䵽��Ļ����
std::tuple<int, int, int> project(vec3 v) {
    int x = (v.x + 1.) * width / 2;
    int y = (1. - (v.y + 1.) / 2.) * height; // ��ת y
    int z = (v.z + 1.) * 255. / 2;           // ӳ�䵽 0-255 �Ҷ�
    return { x, y, z };
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " obj/model.obj" << std::endl;
        return 1;
    }

    Model model(argv[1]);
    TGAImage framebuffer(width, height, TGAImage::RGB);
    TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);

    for (int i = 0; i < model.nfaces(); i++) {
        auto [ax, ay, az] = project(model.vert(i, 0));
        auto [bx, by, bz] = project(model.vert(i, 1));
        auto [cx, cy, cz] = project(model.vert(i, 2));
        triangle(ax, ay, az, bx, by, bz, cx, cy, cz, zbuffer, framebuffer);
    }

    framebuffer.write_tga_file("depth_gray.tga");
    zbuffer.write_tga_file("zbuffer.tga");

    return 0;
}
