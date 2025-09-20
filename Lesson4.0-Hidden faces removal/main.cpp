#include <cmath>
#include <tuple>
#include "geometry.h"     
#include "modelLoader.h" 
#include "tgaimage.h"   

constexpr int width = 800;  
constexpr int height = 800; 

// ��������ŵ��������������������������㣩
double signed_triangle_area(int ax, int ay, int bx, int by, int cx, int cy) {
    return .5 * ((by - ay) * (bx + ax) + (cy - by) * (cx + bx) + (ay - cy) * (ax + cx));
}

// ���������Σ�ʹ���������� + Z-buffer��
void triangle(
    int ax, int ay, int az,  // �����ζ��� A
    int bx, int by, int bz,  // ���� B
    int cx, int cy, int cz,  // ���� C
    TGAImage& zbuffer,       // ��Ȼ���
    TGAImage& framebuffer,   // ������ɫ����
    TGAColor color           // ��������ɫ
) {
    // ���������εı߽��bounding box��
    int bbminx = std::min({ ax, bx, cx });
    int bbminy = std::min({ ay, by, cy });
    int bbmaxx = std::max({ ax, bx, cx });
    int bbmaxy = std::max({ ay, by, cy });

    // �������������ε����
    double total_area = signed_triangle_area(ax, ay, bx, by, cx, cy);

    // ���̫С����������
    if (total_area < 1) return;

    // �����߽���ڵ���������
#pragma omp parallel for
    for (int x = bbminx; x <= bbmaxx; x++) {
        for (int y = bbminy; y <= bbmaxy; y++) {
            // ������������
            double alpha = signed_triangle_area(x, y, bx, by, cx, cy) / total_area;
            double beta = signed_triangle_area(x, y, cx, cy, ax, ay) / total_area;
            double gamma = signed_triangle_area(x, y, ax, ay, bx, by) / total_area;

            // ��������Ϊ����ʾ�õ�����������
            if (alpha < 0 || beta < 0 || gamma < 0) continue;

            // ͨ�����������ֵ�������ֵ z
            unsigned char z = static_cast<unsigned char>(alpha * az + beta * bz + gamma * cz);

            // Z-buffer ��Ȳ��ԣ������ǰ���ظ�Զ������
            if (z <= zbuffer.get(x, y)[0]) continue;

            // ������Ȼ���
            zbuffer.set(x, y, TGAColor(z));

            // ������ɫ
            framebuffer.set(x, y, color);
        }
    }
}

// ͶӰ�������� vec3 ��ά����ͶӰ����Ļ��ά����
std::tuple<int, int, int> project(vec3 v) {
    return {
        (v.x + 1.) * width / 2,        // x ��ӳ�䵽��Ļ
        (v.y + 1.) * height / 2,       // y ��ӳ�䵽��Ļ
        (v.z + 1.) * 255. / 2          // z ��ӳ�䵽 0~255 ��ȻҶ�
    };
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " obj/model.obj" << std::endl;
        return 1;
    }

    Model model(argv[1]);

    TGAImage framebuffer(width, height, TGAImage::RGB);       // ��ɫͼ��
    TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);     // �Ҷ����ͼ

    for (int i = 0; i < model.nfaces(); i++) {
        // ��ȡ���������������ͶӰ����
        auto [ax, ay, az] = project(model.vert(i, 0));
        auto [bx, by, bz] = project(model.vert(i, 1));
        auto [cx, cy, cz] = project(model.vert(i, 2));

        // ���������������ɫ
        TGAColor rnd;
        for (int c = 0; c < 3; c++) rnd[c] = std::rand() % 255;

        // ����������
        triangle(ax, ay, az, bx, by, bz, cx, cy, cz, zbuffer, framebuffer, rnd);
    }

    // ��תͼ��
    framebuffer.flip_vertically();
    zbuffer.flip_vertically();

    // ����ͼ��
    framebuffer.write_tga_file("framebuffer.tga");
    zbuffer.write_tga_file("zbuffer.tga");

    return 0;
}
