#include <cmath>
#include "tgaimage.h"

// ===================== 计算有向三角形面积 =====================
// 输入三个点 (ax, ay), (bx, by), (cx, cy)
// 输出带符号的面积（正负号表示点的顺序方向）
double signed_triangle_area(int ax, int ay, int bx, int by, int cx, int cy) {
    return .5 * ((by - ay) * (bx + ax) +
        (cy - by) * (cx + bx) +
        (ay - cy) * (ax + cx));
}


// ===================== 绘制三角形 =====================
// 输入：三角形三个顶点 (ax,ay,az), (bx,by,bz), (cx,cy,cz)
// ax,ay,bx,by,cx,cy 是屏幕坐标 (2D)，az,bz,cz 是深度值
// framebuffer 是画布，用灰度图保存
void triangle(int ax, int ay, int az,
    int bx, int by, int bz,
    int cx, int cy, int cz,
    TGAImage& framebuffer) {

    // 计算三角形的包围盒（bounding box），减少扫描范围
    int bbminx = std::min(std::min(ax, bx), cx); // 左边界
    int bbminy = std::min(std::min(ay, by), cy); // 上边界
    int bbmaxx = std::max(std::max(ax, bx), cx); // 右边界
    int bbmaxy = std::max(std::max(ay, by), cy); // 下边界

    // 整个三角形的有向面积
    double total_area = signed_triangle_area(ax, ay, bx, by, cx, cy);

    // 如果面积过小 (<1)，说明是背面或几乎不可见，直接丢弃
    if (total_area < 1) return;


#pragma omp parallel for
    for (int x = bbminx; x <= bbmaxx; x++) {
        for (int y = bbminy; y <= bbmaxy; y++) {
            // 计算像素 (x,y) 相对三角形的重心坐标 (alpha, beta, gamma)
            double alpha = signed_triangle_area(x, y, bx, by, cx, cy) / total_area;
            double beta = signed_triangle_area(x, y, cx, cy, ax, ay) / total_area;
            double gamma = signed_triangle_area(x, y, ax, ay, bx, by) / total_area;

            // 如果有负数，说明点在三角形外，跳过
            if (alpha < 0 || beta < 0 || gamma < 0) continue;

            // 使用重心坐标插值深度值 (az, bz, cz)
            unsigned char z = static_cast<unsigned char>(
                alpha * az + beta * bz + gamma * cz
                );

            // 在帧缓冲里设置像素颜色（灰度）
            framebuffer.set(x, y, { z });
        }
    }
}


int main(int argc, char** argv) {
    constexpr int width = 64;
    constexpr int height = 64;

    TGAImage framebuffer(width, height, TGAImage::GRAYSCALE);

    int ax = 17, ay = 4, az = 13;
    int bx = 55, by = 39, bz = 128;
    int cx = 23, cy = 59, cz = 255;

    triangle(ax, ay, az, bx, by, bz, cx, cy, cz, framebuffer);

    framebuffer.write_tga_file("framebuffer_z.tga");
    return 0;
}
