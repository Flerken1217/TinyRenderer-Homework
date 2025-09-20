#include <cmath>
#include <tuple>
#include "geometry.h"
#include "modelLoader.h"
#include "tgaimage.h"

constexpr int width = 800;
constexpr int height = 800;

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


double signed_triangle_area(int ax, int ay, int bx, int by, int cx, int cy) {
	return .5 * ((by - ay) * (bx + ax) + (cy - by) * (cx + bx) + (ay - cy) * (ax + cx));
}


//绘制三角形
void triangle(int ax, int ay, int bx, int by, int cx, int cy,
	TGAImage& framebuffer, TGAColor color) {
	// 计算三角形的包围盒 (bounding box)
	int bbminx = std::min({ ax, bx, cx });
	int bbminy = std::min({ ay, by, cy });
	int bbmaxx = std::max({ ax, bx, cx });
	int bbmaxy = std::max({ ay, by, cy });

	// 整个三角形的面积
	double total_area = signed_triangle_area(ax, ay, bx, by, cx, cy);
	if (total_area < 1) return; // 背面剔除 + 小于一个像素就忽略

// 并行扫描填充 (OpenMP)
#pragma omp parallel for
	for (int x = bbminx; x <= bbmaxx; x++) {
		for (int y = bbminy; y <= bbmaxy; y++) {
			// 重心坐标 (barycentric coordinates)
			double alpha = signed_triangle_area(x, y, bx, by, cx, cy) / total_area;
			double beta = signed_triangle_area(x, y, cx, cy, ax, ay) / total_area;
			double gamma = signed_triangle_area(x, y, ax, ay, bx, by) / total_area;

			// 如果有负数，说明在三角形外
			if (alpha < 0 || beta < 0 || gamma < 0) continue;

			// 填充像素
			framebuffer.set(x, y, color);
		}
	}
}

int main(int argc, char** argv) {
	if (argc != 2) {
		std::cerr << "Usage: " << argv[0] << " obj/model.obj" << std::endl;
		return 1;
	}

	// 载入模型
	Model model(argv[1]);

	// 创建一个空的帧缓冲（128x128, RGB）
	TGAImage framebuffer(width, height, TGAImage::RGB);

	// 遍历所有三角形
	for (int i = 0; i < model.nfaces(); i++) {
		// 取出第 i 个三角形的三个顶点（3D 坐标）
		vec3 v0 = model.vert(i, 0);
		vec3 v1 = model.vert(i, 1);
		vec3 v2 = model.vert(i, 2);

		// 投影到屏幕坐标 (2D)
		auto [ax, ay] = project(v0, width, height);
		auto [bx, by] = project(v1, width, height);
		auto [cx, cy] = project(v2, width, height);

		// 随机生成三角形颜色
		TGAColor rnd;
		for (int c = 0; c < 3; c++) rnd[c] = std::rand() % 255;

		// 绘制三角形
		triangle(ax, ay, bx, by, cx, cy, framebuffer, rnd);
	}

	// 输出到 tga 文件
	framebuffer.write_tga_file("framebuffer.tga");
	return 0;
}
