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

//�����㻭ֱ��
void DrawLine(int x0,int y0,int x1,int y1, TGAImage& image, TGAColor color) {
    
	bool steep = false; //��ǵ�ǰб�ʵľ���ֵ�Ƿ����1
	if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
		//б�ʾ���ֵ>1�ˣ���ʱ���߶ζ˵���Ե�x,y����Ե���
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

int main(int argc, char** argv) {
	if (argc != 2) {
		std::cerr << "Usage: " << argv[0] << " obj/model.obj" << std::endl;
		return 1;
	}

	Model model(argv[1]);
	TGAImage framebuffer(width, height, TGAImage::RGB);

	// ����������
	for (int i = 0; i < model.nfaces(); i++) {
		Vec2i a = project(model.vert(i, 0), width, height);
		Vec2i b = project(model.vert(i, 1), width, height);
		Vec2i c = project(model.vert(i, 2), width, height);

		DrawLine(a.x, a.y, b.x, b.y, framebuffer, red);
		DrawLine(b.x, b.y, c.x, c.y, framebuffer, red);
		DrawLine(c.x, c.y, a.x, a.y, framebuffer, red);
	}

	// �������ж��㲢���
	for (int i = 0; i < model.nverts(); i++) {
		vec3 v = model.vert(i);
		Vec2i p = project(v, width, height); // ͶӰ����Ļ
		framebuffer.set(p.x, p.y, white);
	}

	framebuffer.write_tga_file("framebuffer.tga");
	return 0;
}


