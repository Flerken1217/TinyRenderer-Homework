#include <cmath>
#include <tuple>
#include "geometry.h"
#include "modelLoader.h"
#include "tgaimage.h"

constexpr int width = 800;
constexpr int height = 800;

constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};

//以两点画直线
void DrawLine(int x0,int y0,int x1,int y1, TGAImage& image, TGAColor color) {
    
	bool steep = false; //标记当前斜率的绝对值是否大于1
	if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
		//斜率绝对值>1了，此时将线段端点各自的x,y坐标对调。
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

int main(int argc, char** argv) {
	if (argc != 2) {
		std::cerr << "Usage: " << argv[0] << " obj/model.obj" << std::endl;
		return 1;
	}

	Model model(argv[1]);
	TGAImage framebuffer(width, height, TGAImage::RGB);

	// 遍历三角形
	for (int i = 0; i < model.nfaces(); i++) {
		Vec2i a = project(model.vert(i, 0), width, height);
		Vec2i b = project(model.vert(i, 1), width, height);
		Vec2i c = project(model.vert(i, 2), width, height);

		DrawLine(a.x, a.y, b.x, b.y, framebuffer, red);
		DrawLine(b.x, b.y, c.x, c.y, framebuffer, red);
		DrawLine(c.x, c.y, a.x, a.y, framebuffer, red);
	}

	// 遍历所有顶点并标记
	for (int i = 0; i < model.nverts(); i++) {
		vec3 v = model.vert(i);
		Vec2i p = project(v, width, height); // 投影到屏幕
		framebuffer.set(p.x, p.y, white);
	}

	framebuffer.write_tga_file("framebuffer.tga");
	return 0;
}


