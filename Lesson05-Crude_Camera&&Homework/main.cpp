#include <cmath>
#include <tuple>
#include <vector>
#include <algorithm>
#include <iostream>
#include "geometry.h"
#include "modelLoader.h"
#include "tgaimage.h"

constexpr int width = 800;
constexpr int height = 800;
constexpr double PI = 3.14159265358979323846;

// 计算三角形的有向面积
double signed_triangle_area(int ax, int ay, int bx, int by, int cx, int cy) {
    return .5 * ((by - ay) * (bx + ax) +
        (cy - by) * (cx + bx) +
        (ay - cy) * (ax + cx));
}

// triangle 函数：使用浮点 Z-buffer
void triangle(int ax, int ay, float az,
    int bx, int by, float bz,
    int cx, int cy, float cz,
    const TGAColor& color,
    std::vector<std::vector<float>>& zbuffer,
    TGAImage& framebuffer) {

    int bbminx = std::max(0, std::min({ ax, bx, cx }));
    int bbminy = std::max(0, std::min({ ay, by, cy }));
    int bbmaxx = std::min(framebuffer.width() - 1, std::max({ ax, bx, cx }));
    int bbmaxy = std::min(framebuffer.height() - 1, std::max({ ay, by, cy }));

    double total_area = signed_triangle_area(ax, ay, bx, by, cx, cy);
    if (total_area <= 0) return;

    for (int x = bbminx; x <= bbmaxx; ++x) {
        for (int y = bbminy; y <= bbmaxy; ++y) {
            double alpha = signed_triangle_area(x, y, bx, by, cx, cy) / total_area;
            double beta = signed_triangle_area(x, y, cx, cy, ax, ay) / total_area;
            double gamma = signed_triangle_area(x, y, ax, ay, bx, by) / total_area;

            if (alpha < 0 || beta < 0 || gamma < 0) continue;

            float z = alpha * az + beta * bz + gamma * cz;

            if (zbuffer[x][y] < z) {
                zbuffer[x][y] = z;
                framebuffer.set(x, y, color);
            }
        }
    }
}

// 绕 Y 轴旋转向量 v
vec3 rot(vec3 v) {
    double a = PI / 6; // 旋转角度 30 度
    mat<3, 3> Ry = {
        {{std::cos(a), 0, std::sin(a)},
         {0, 1, 0},
         {-std::sin(a), 0, std::cos(a)}}
    };
    return Ry * v;
}

// 简单透视投影
vec3 persp(vec3 v) {
    double c = 3.;
    return v / (1 - v.z / c);
}

// 将 3D 世界坐标投影到屏幕坐标
std::tuple<int, int, float> project(vec3 v) {
    return { static_cast<int>((v.x + 1.) * width / 2),
             static_cast<int>((v.y + 1.) * height / 2),
             v.z };
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " obj/model.obj" << std::endl;
        return 1;
    }

    Model model(argv[1]);
    TGAImage framebuffer(width, height, TGAImage::RGB);

    // 浮点 Z-buffer
    std::vector<std::vector<float>> zbuffer(width, std::vector<float>(height, -1e30f));

    for (int i = 0; i < model.nfaces(); i++) {
        auto [ax, ay, az] = project(persp(rot(model.vert(i, 0))));
        auto [bx, by, bz] = project(persp(rot(model.vert(i, 1))));
        auto [cx, cy, cz] = project(persp(rot(model.vert(i, 2))));

        TGAColor rnd;
        for (int c = 0; c < 3; c++) rnd[c] = std::rand() % 255;

        triangle(ax, ay, az, bx, by, bz, cx, cy, cz, rnd, zbuffer, framebuffer);
    }

    // 输出 framebuffer
    framebuffer.write_tga_file("framebuffer.tga");

    // 输出 Z-buffer 可视化
    TGAImage zimg(width, height, TGAImage::GRAYSCALE);
    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
            float z = zbuffer[x][y];
            unsigned char value = static_cast<unsigned char>(255.f * (z + 1.f) / 2.f);
            zimg.set(x, y, { value });
        }
    }
    zimg.write_tga_file("zbuffer.tga");

    return 0;
}
