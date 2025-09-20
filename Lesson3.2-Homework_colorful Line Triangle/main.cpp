#include <cmath>
#include "tgaimage.h"


// 顶点颜色插值画线
void line_color(int x0, int y0, TGAColor c0,
    int x1, int y1, TGAColor c1,
    TGAImage& image)
{
    int dx = x1 - x0;
    int dy = y1 - y0;
    int steps = std::max(std::abs(dx), std::abs(dy));
    if (steps == 0) { image.set(x0, y0, c0); return; }

    float x_inc = dx / float(steps);
    float y_inc = dy / float(steps);

    // 分别计算每个颜色通道的增量
    float r_inc = (c1[2] - c0[2]) / float(steps); // R 通道在 bgra[2]
    float g_inc = (c1[1] - c0[1]) / float(steps); // G 通道在 bgra[1]
    float b_inc = (c1[0] - c0[0]) / float(steps); // B 通道在 bgra[0]

    float x = x0;
    float y = y0;
    float r = c0[2];
    float g = c0[1];
    float b = c0[0];

    for (int i = 0; i <= steps; i++) {
        image.set(int(x + 0.5f), int(y + 0.5f), TGAColor((uint8_t)r, (uint8_t)g, (uint8_t)b));
        x += x_inc;
        y += y_inc;
        r += r_inc;
        g += g_inc;
        b += b_inc;
    }
}

// 彩色线框三角形
void wireframe_triangle_color(int ax, int ay, TGAColor ca,
    int bx, int by, TGAColor cb,
    int cx, int cy, TGAColor cc,
    TGAImage& image)
{
    line_color(ax, ay, ca, bx, by, cb, image);
    line_color(bx, by, cb, cx, cy, cc, image);
    line_color(cx, cy, cc, ax, ay, ca, image);
}

// 示例
int main() {
    TGAImage image(128, 128, TGAImage::RGB);

    int ax = 20, ay = 10;
    int bx = 100, by = 30;
    int cx = 50, cy = 100;

    TGAColor colorA(255, 0, 0);   // 红
    TGAColor colorB(0, 255, 0);   // 绿
    TGAColor colorC(0, 0, 255);   // 蓝

    wireframe_triangle_color(ax, ay, colorA,
        bx, by, colorB,
        cx, cy, colorC,
        image);

    image.write_tga_file("wireframe_color.tga");
    return 0;
}

