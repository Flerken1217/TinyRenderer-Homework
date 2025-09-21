#include <cmath>
#include "tgaimage.h"

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
			//����߶���б�ʴ���1�ģ���ô�߶��ϵĵ�ԭ������Ӧ����(y,x)
			image.set(y, x, color);
		}
		else {
			image.set(x, y, color);
		}
	}
}

int main(int argc, char** argv) {
    constexpr int width  = 64;
    constexpr int height = 64;
    TGAImage framebuffer(width, height, TGAImage::RGB);

    //���������㣬�ֱ�Ϊ��ɫ
    int ax =  7, ay =  3;
    int bx = 12, by = 37;
    int cx = 62, cy = 53;

    DrawLine(ax, ay, bx, by, framebuffer, blue);
    DrawLine(cx, cy, bx, by, framebuffer, green);
    DrawLine(cx, cy, ax, ay, framebuffer, yellow);
    DrawLine(ax, ay, cx, cy, framebuffer, red);

    framebuffer.set(ax, ay, white);
    framebuffer.set(bx, by, white);
    framebuffer.set(cx, cy, white);

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}

