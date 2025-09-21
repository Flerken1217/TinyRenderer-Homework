#include <cmath>
#include "tgaimage.h"

// ===================== ����������������� =====================
// ���������� (ax, ay), (bx, by), (cx, cy)
// ��������ŵ�����������ű�ʾ���˳����
double signed_triangle_area(int ax, int ay, int bx, int by, int cx, int cy) {
    return .5 * ((by - ay) * (bx + ax) +
        (cy - by) * (cx + bx) +
        (ay - cy) * (ax + cx));
}


// ===================== ���������� =====================
// ���룺�������������� (ax,ay,az), (bx,by,bz), (cx,cy,cz)
// ax,ay,bx,by,cx,cy ����Ļ���� (2D)��az,bz,cz �����ֵ
// framebuffer �ǻ������ûҶ�ͼ����
void triangle(int ax, int ay, int az,
    int bx, int by, int bz,
    int cx, int cy, int cz,
    TGAImage& framebuffer) {

    // ���������εİ�Χ�У�bounding box��������ɨ�跶Χ
    int bbminx = std::min(std::min(ax, bx), cx); // ��߽�
    int bbminy = std::min(std::min(ay, by), cy); // �ϱ߽�
    int bbmaxx = std::max(std::max(ax, bx), cx); // �ұ߽�
    int bbmaxy = std::max(std::max(ay, by), cy); // �±߽�

    // ���������ε��������
    double total_area = signed_triangle_area(ax, ay, bx, by, cx, cy);

    // ��������С (<1)��˵���Ǳ���򼸺����ɼ���ֱ�Ӷ���
    if (total_area < 1) return;


#pragma omp parallel for
    for (int x = bbminx; x <= bbmaxx; x++) {
        for (int y = bbminy; y <= bbmaxy; y++) {
            // �������� (x,y) ��������ε��������� (alpha, beta, gamma)
            double alpha = signed_triangle_area(x, y, bx, by, cx, cy) / total_area;
            double beta = signed_triangle_area(x, y, cx, cy, ax, ay) / total_area;
            double gamma = signed_triangle_area(x, y, ax, ay, bx, by) / total_area;

            // ����и�����˵�������������⣬����
            if (alpha < 0 || beta < 0 || gamma < 0) continue;

            // ʹ�����������ֵ���ֵ (az, bz, cz)
            unsigned char z = static_cast<unsigned char>(
                alpha * az + beta * bz + gamma * cz
                );

            // ��֡����������������ɫ���Ҷȣ�
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
