#include <cmath>
#include <tuple>
#include <iostream>
#include "geometry.h"
#include "modelLoader.h"
#include "tgaimage.h"

constexpr int width = 800;
constexpr int height = 800;

// 计算有符号三角形面积，用于重心坐标
double signed_triangle_area(int ax, int ay, int bx, int by, int cx, int cy) {
    return .5 * ((by - ay) * (bx + ax) + (cy - by) * (cx + bx) + (ay - cy) * (ax + cx));
}

// 绘制三角形（深度灰度，带 z-buffer）
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

            // 插值 z
            float z = alpha * az + beta * bz + gamma * cz;

            // z-buffer 判断
            if (z <= zbuffer.get(x, y)[0]) continue;

            TGAColor depth_color((uint8_t)z); // 灰度颜色
            zbuffer.set(x, y, depth_color);   // 更新 z-buffer
            framebuffer.set(x, y, depth_color); // 绘制深度灰度
        }
    }
}

// 投影函数：将 [-1,1]^3 映射到屏幕坐标
std::tuple<int, int, int> project(vec3 v) {
    int x = (v.x + 1.) * width / 2;
    int y = (1. - (v.y + 1.) / 2.) * height; // 翻转 y
    int z = (v.z + 1.) * 255. / 2;           // 映射到 0-255 灰度
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
