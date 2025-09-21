#include <cmath>
#include <tuple>
#include "geometry.h"     
#include "modelLoader.h" 
#include "tgaimage.h"   

constexpr int width = 800;  
constexpr int height = 800; 

// 计算带符号的三角形面积（用于重心坐标计算）
double signed_triangle_area(int ax, int ay, int bx, int by, int cx, int cy) {
    return .5 * ((by - ay) * (bx + ax) + (cy - by) * (cx + bx) + (ay - cy) * (ax + cx));
}

// 绘制三角形（使用重心坐标 + Z-buffer）
void triangle(
    int ax, int ay, int az,  // 三角形顶点 A
    int bx, int by, int bz,  // 顶点 B
    int cx, int cy, int cz,  // 顶点 C
    TGAImage& zbuffer,       // 深度缓存
    TGAImage& framebuffer,   // 最终颜色缓存
    TGAColor color           // 三角形颜色
) {
    // 计算三角形的边界框（bounding box）
    int bbminx = std::min({ ax, bx, cx });
    int bbminy = std::min({ ay, by, cy });
    int bbmaxx = std::max({ ax, bx, cx });
    int bbmaxy = std::max({ ay, by, cy });

    // 计算整个三角形的面积
    double total_area = signed_triangle_area(ax, ay, bx, by, cx, cy);

    // 面积太小或朝向背面则丢弃
    if (total_area < 1) return;

    // 遍历边界框内的所有像素
#pragma omp parallel for
    for (int x = bbminx; x <= bbmaxx; x++) {
        for (int y = bbminy; y <= bbmaxy; y++) {
            // 计算重心坐标
            double alpha = signed_triangle_area(x, y, bx, by, cx, cy) / total_area;
            double beta = signed_triangle_area(x, y, cx, cy, ax, ay) / total_area;
            double gamma = signed_triangle_area(x, y, ax, ay, bx, by) / total_area;

            // 重心坐标为负表示该点在三角形外
            if (alpha < 0 || beta < 0 || gamma < 0) continue;

            // 通过重心坐标插值计算深度值 z
            unsigned char z = static_cast<unsigned char>(alpha * az + beta * bz + gamma * cz);

            // Z-buffer 深度测试，如果当前像素更远则跳过
            if (z <= zbuffer.get(x, y)[0]) continue;

            // 更新深度缓存
            zbuffer.set(x, y, TGAColor(z));

            // 设置颜色
            framebuffer.set(x, y, color);
        }
    }
}

// 投影函数：将 vec3 三维坐标投影到屏幕二维坐标
std::tuple<int, int, int> project(vec3 v) {
    return {
        (v.x + 1.) * width / 2,        // x 轴映射到屏幕
        (v.y + 1.) * height / 2,       // y 轴映射到屏幕
        (v.z + 1.) * 255. / 2          // z 轴映射到 0~255 深度灰度
    };
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " obj/model.obj" << std::endl;
        return 1;
    }

    Model model(argv[1]);

    TGAImage framebuffer(width, height, TGAImage::RGB);       // 彩色图像
    TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);     // 灰度深度图

    for (int i = 0; i < model.nfaces(); i++) {
        // 获取三角形三个顶点的投影坐标
        auto [ax, ay, az] = project(model.vert(i, 0));
        auto [bx, by, bz] = project(model.vert(i, 1));
        auto [cx, cy, cz] = project(model.vert(i, 2));

        // 随机生成三角形颜色
        TGAColor rnd;
        for (int c = 0; c < 3; c++) rnd[c] = std::rand() % 255;

        // 绘制三角形
        triangle(ax, ay, az, bx, by, bz, cx, cy, cz, zbuffer, framebuffer, rnd);
    }

    // 翻转图像
    framebuffer.flip_vertically();
    zbuffer.flip_vertically();

    // 保存图像
    framebuffer.write_tga_file("framebuffer.tga");
    zbuffer.write_tga_file("zbuffer.tga");

    return 0;
}
