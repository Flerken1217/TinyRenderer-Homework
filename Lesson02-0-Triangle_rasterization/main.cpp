#include <cmath>
#include <tuple>
#include "geometry.h"
#include "modelLoader.h"
#include "tgaimage.h"

constexpr int width = 128;
constexpr int height = 128;

constexpr TGAColor white = { 255, 255, 255, 255 }; // attention, BGRA order
constexpr TGAColor green = { 0, 255,   0, 255 };
constexpr TGAColor red = { 0,   0, 255, 255 };
constexpr TGAColor blue = { 255, 128,  64, 255 };
constexpr TGAColor yellow = { 0, 200, 255, 255 };


//以两点画直线
void DrawLine(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {

	bool steep = false; //标记当前斜率的绝对值是否大于1
	if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
		//斜率绝对值>1,将线段端点各自的x,y坐标对调
		std::swap(x0, y0);
		std::swap(x1, y1);
		steep = true;
	}
	if (x0 > x1) {  //x0>x1时，对线段端点坐标进行对调
		std::swap(x0, x1);
		std::swap(y0, y1);
	}
	for (int x = x0; x <= x1; x++) {
		float t = (x - x0) / (float)(x1 - x0);
		int y = y0 * (1. - t) + y1 * t;
		if (steep) {
			//如果线段是斜率大于1的，线段上的点原本坐标应该是(y,x)
			image.set(y, x, color);
		}
		else {
			image.set(x, y, color);
		}
	}
}

//忽略z，直接用3D的xy投影到2D平面
Vec2i project(const vec3& v, int width, int height) {
	// 正交投影，把 [-1,1]^2 映射到屏幕 [0,width] x [0,height]
	int x_screen = static_cast<int>((v.x + 1.0) * width / 2.0);
	int y_screen = static_cast<int>((v.y + 1.0) * height / 2.0);
	return Vec2i(x_screen, y_screen);
}



//三角形绘制函数
void triangle(int ax, int ay, int bx, int by, int cx, int cy,
    TGAImage& framebuffer, TGAColor color) {
    // 将三角形按 y 坐标从小到大排序，ay <= by <= cy
    // 三角形被分成“下半部分三角形”和“上半部分三角形”
    if (ay > by) { std::swap(ax, bx); std::swap(ay, by); }
    if (ay > cy) { std::swap(ax, cx); std::swap(ay, cy); }
    if (by > cy) { std::swap(bx, cx); std::swap(by, cy); }

    // 三角形的总高度（ a 到 c 的 y 距离）
    int total_height = cy - ay;

    // -------- 处理下半部分三角形 (a 到 b) --------
    if (ay != by) { // 如果 a 和 b 不在同一条水平线上
        int segment_height = by - ay; // 下半部分高度
        // 遍历从 a.y 到 b.y 的每一条扫描线
        for (int y = ay; y <= by; y++) {
            // 在当前 y 上，计算 a->c 边对应的 x 坐标
            int x1 = ax + ((cx - ax) * (y - ay)) / total_height;
            // 在当前 y 上，计算 a->b 边对应的 x 坐标
            int x2 = ax + ((bx - ax) * (y - ay)) / segment_height;
            // 在这条扫描线上，从左边界画到右边界
            for (int x = std::min(x1, x2); x < std::max(x1, x2); x++)
                framebuffer.set(x, y, color); // 设置像素颜色
        }
    }

    // -------- 处理上半部分三角形 (b 到 c) --------
    if (by != cy) { // 如果 b 和 c 不在同一条水平线上
        int segment_height = cy - by; // 上半部分高度
        // 遍历从 b.y 到 c.y 的每一条扫描线
        for (int y = by; y <= cy; y++) {
            // 在当前 y 上，计算 a->c 边对应的 x 坐标
            int x1 = ax + ((cx - ax) * (y - ay)) / total_height;
            // 在当前 y 上，计算 b->c 边对应的 x 坐标
            int x2 = bx + ((cx - bx) * (y - by)) / segment_height;
            // 在这条扫描线上，从左边界画到右边界
            for (int x = std::min(x1, x2); x < std::max(x1, x2); x++)
                framebuffer.set(x, y, color); // 设置像素颜色
        }
    }
}


int main(int argc, char** argv) {
    TGAImage framebuffer(width, height, TGAImage::RGB);
    triangle(7, 45, 35, 100, 45, 60, framebuffer, red);
    triangle(120, 35, 90, 5, 45, 110, framebuffer, white);
    triangle(115, 83, 80, 90, 85, 120, framebuffer, green);
    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}