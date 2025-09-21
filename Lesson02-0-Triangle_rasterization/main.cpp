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



//�����λ��ƺ���
void triangle(int ax, int ay, int bx, int by, int cx, int cy,
    TGAImage& framebuffer, TGAColor color) {
    // �������ΰ� y �����С��������ay <= by <= cy
    // �����α��ֳɡ��°벿�������Ρ��͡��ϰ벿�������Ρ�
    if (ay > by) { std::swap(ax, bx); std::swap(ay, by); }
    if (ay > cy) { std::swap(ax, cx); std::swap(ay, cy); }
    if (by > cy) { std::swap(bx, cx); std::swap(by, cy); }

    // �����ε��ܸ߶ȣ� a �� c �� y ���룩
    int total_height = cy - ay;

    // -------- �����°벿�������� (a �� b) --------
    if (ay != by) { // ��� a �� b ����ͬһ��ˮƽ����
        int segment_height = by - ay; // �°벿�ָ߶�
        // ������ a.y �� b.y ��ÿһ��ɨ����
        for (int y = ay; y <= by; y++) {
            // �ڵ�ǰ y �ϣ����� a->c �߶�Ӧ�� x ����
            int x1 = ax + ((cx - ax) * (y - ay)) / total_height;
            // �ڵ�ǰ y �ϣ����� a->b �߶�Ӧ�� x ����
            int x2 = ax + ((bx - ax) * (y - ay)) / segment_height;
            // ������ɨ�����ϣ�����߽续���ұ߽�
            for (int x = std::min(x1, x2); x < std::max(x1, x2); x++)
                framebuffer.set(x, y, color); // ����������ɫ
        }
    }

    // -------- �����ϰ벿�������� (b �� c) --------
    if (by != cy) { // ��� b �� c ����ͬһ��ˮƽ����
        int segment_height = cy - by; // �ϰ벿�ָ߶�
        // ������ b.y �� c.y ��ÿһ��ɨ����
        for (int y = by; y <= cy; y++) {
            // �ڵ�ǰ y �ϣ����� a->c �߶�Ӧ�� x ����
            int x1 = ax + ((cx - ax) * (y - ay)) / total_height;
            // �ڵ�ǰ y �ϣ����� b->c �߶�Ӧ�� x ����
            int x2 = bx + ((cx - bx) * (y - by)) / segment_height;
            // ������ɨ�����ϣ�����߽续���ұ߽�
            for (int x = std::min(x1, x2); x < std::max(x1, x2); x++)
                framebuffer.set(x, y, color); // ����������ɫ
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