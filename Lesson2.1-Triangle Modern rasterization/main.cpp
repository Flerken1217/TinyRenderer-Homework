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


//�����㻭ֱ��
void DrawLine(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {

	bool steep = false; //��ǵ�ǰб�ʵľ���ֵ�Ƿ����1
	if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
		//б�ʾ���ֵ>1,���߶ζ˵���Ե�x,y����Ե�
		std::swap(x0, y0);
		std::swap(x1, y1);
		steep = true;
	}
	if (x0 > x1) {  //x0>x1ʱ�����߶ζ˵�������жԵ�
		std::swap(x0, x1);
		std::swap(y0, y1);
	}
	for (int x = x0; x <= x1; x++) {
		float t = (x - x0) / (float)(x1 - x0);
		int y = y0 * (1. - t) + y1 * t;
		if (steep) {
			//����߶���б�ʴ���1�ģ��߶��ϵĵ�ԭ������Ӧ����(y,x)
			image.set(y, x, color);
		}
		else {
			image.set(x, y, color);
		}
	}
}

//����z��ֱ����3D��xyͶӰ��2Dƽ��
Vec2i project(const vec3& v, int width, int height) {
	// ����ͶӰ���� [-1,1]^2 ӳ�䵽��Ļ [0,width] x [0,height]
	int x_screen = static_cast<int>((v.x + 1.0) * width / 2.0);
	int y_screen = static_cast<int>((v.y + 1.0) * height / 2.0);
	return Vec2i(x_screen, y_screen);
}


double signed_triangle_area(int ax, int ay, int bx, int by, int cx, int cy) {
	return .5 * ((by - ay) * (bx + ax) + (cy - by) * (cx + bx) + (ay - cy) * (ax + cx));
}


//����������
void triangle(int ax, int ay, int bx, int by, int cx, int cy,
	TGAImage& framebuffer, TGAColor color) {
	// ���������εİ�Χ�� (bounding box)
	int bbminx = std::min({ ax, bx, cx });
	int bbminy = std::min({ ay, by, cy });
	int bbmaxx = std::max({ ax, bx, cx });
	int bbmaxy = std::max({ ay, by, cy });

	// ���������ε����
	double total_area = signed_triangle_area(ax, ay, bx, by, cx, cy);
	if (total_area < 1) return; // �����޳� + С��һ�����ؾͺ���

// ����ɨ����� (OpenMP)
#pragma omp parallel for
	for (int x = bbminx; x <= bbmaxx; x++) {
		for (int y = bbminy; y <= bbmaxy; y++) {
			// �������� (barycentric coordinates)
			double alpha = signed_triangle_area(x, y, bx, by, cx, cy) / total_area;
			double beta = signed_triangle_area(x, y, cx, cy, ax, ay) / total_area;
			double gamma = signed_triangle_area(x, y, ax, ay, bx, by) / total_area;

			// ����и�����˵������������
			if (alpha < 0 || beta < 0 || gamma < 0) continue;

			// �������
			framebuffer.set(x, y, color);
		}
	}
}

int main(int argc, char** argv) {
	if (argc != 2) {
		std::cerr << "Usage: " << argv[0] << " obj/model.obj" << std::endl;
		return 1;
	}

	// ����ģ��
	Model model(argv[1]);

	// ����һ���յ�֡���壨128x128, RGB��
	TGAImage framebuffer(width, height, TGAImage::RGB);

	// ��������������
	for (int i = 0; i < model.nfaces(); i++) {
		// ȡ���� i �������ε��������㣨3D ���꣩
		vec3 v0 = model.vert(i, 0);
		vec3 v1 = model.vert(i, 1);
		vec3 v2 = model.vert(i, 2);

		// ͶӰ����Ļ���� (2D)
		auto [ax, ay] = project(v0, width, height);
		auto [bx, by] = project(v1, width, height);
		auto [cx, cy] = project(v2, width, height);

		// ���������������ɫ
		TGAColor rnd;
		for (int c = 0; c < 3; c++) rnd[c] = std::rand() % 255;

		// ����������
		triangle(ax, ay, bx, by, cx, cy, framebuffer, rnd);
	}

	// ����� tga �ļ�
	framebuffer.write_tga_file("framebuffer.tga");
	return 0;
}
