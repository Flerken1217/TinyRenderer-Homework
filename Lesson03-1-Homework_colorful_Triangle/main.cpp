#include <cmath>
#include "tgaimage.h"

// ������������������������꣩
double signed_triangle_area(int ax, int ay, int bx, int by, int cx, int cy) {
    return .5 * ((by - ay) * (bx + ax) + (cy - by) * (cx + bx) + (ay - cy) * (ax + cx));
}

// ��Ⱦһ����ɫ������
void triangle(
    int ax, int ay, int az, TGAColor colorA,
    int bx, int by, int bz, TGAColor colorB,
    int cx, int cy, int cz, TGAColor colorC,
    TGAImage& framebuffer
) {
    // �����Χ��
    int bbminx = std::min({ ax, bx, cx });
    int bbminy = std::min({ ay, by, cy });
    int bbmaxx = std::max({ ax, bx, cx });
    int bbmaxy = std::max({ ay, by, cy });

    double total_area = signed_triangle_area(ax, ay, bx, by, cx, cy);
    if (total_area < 1) return; // �����޳���̫С�������ζ���

    // ɨ������
    for (int x = bbminx; x <= bbmaxx; x++) {
        for (int y = bbminy; y <= bbmaxy; y++) {
            // ������������
            double alpha = signed_triangle_area(x, y, bx, by, cx, cy) / total_area;
            double beta = signed_triangle_area(x, y, cx, cy, ax, ay) / total_area;
            double gamma = signed_triangle_area(x, y, ax, ay, bx, by) / total_area;

            if (alpha < 0 || beta < 0 || gamma < 0) continue; // ���������ⲿ

            // ��ֵ��ȣ������Ҫ�� z-buffer �����������
            /*double z = alpha * az + beta * bz + gamma * cz;*/

            // ��ֵ��ɫ
            unsigned char r = static_cast<unsigned char>(
                alpha * colorA[2] + beta * colorB[2] + gamma * colorC[2]   // R ������ bgra[2]
                );
            unsigned char g = static_cast<unsigned char>(
                alpha * colorA[1] + beta * colorB[1] + gamma * colorC[1]   // G ������ bgra[1]
                );
            unsigned char b = static_cast<unsigned char>(
                alpha * colorA[0] + beta * colorB[0] + gamma * colorC[0]   // B ������ bgra[0]
                );

            framebuffer.set(x, y, TGAColor(r, g, b, 255));
        }
    }
}


int main(int argc, char** argv) {
    constexpr int width = 64;
    constexpr int height = 64;
    TGAImage framebuffer(width, height, TGAImage::RGB);

    // �����ζ����������ɫ
    int ax = 10, ay = 10, az = 50;
    int bx = 50, by = 20, bz = 120;
    int cx = 25, cy = 55, cz = 200;

    TGAColor colorA = TGAColor(255, 0, 0, 255);   // ��
    TGAColor colorB = TGAColor(0, 255, 0, 255);   // ��
    TGAColor colorC = TGAColor(0, 0, 255, 255);   // ��

    //���Ҷ�������
    /*triangle(ax, ay, az, bx, by, bz, cx, cy, cz, framebuffer);*/

    // ����ɫ������
    triangle(ax, ay, az, colorA, bx, by, bz, colorB, cx, cy, cz, colorC, framebuffer);

    framebuffer.write_tga_file("color_triangle.tga");
    return 0;
}
