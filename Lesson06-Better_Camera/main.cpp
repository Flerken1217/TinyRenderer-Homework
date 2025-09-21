#include <limits>
#include <algorithm>
#include "geometry.h"
#include "modelLoader.h"
#include "tgaimage.h"

// 全局矩阵：用于变换顶点到屏幕
mat<4, 4> ModelView, Viewport, Perspective;

// 构建摄像机视图矩阵（类似 gluLookAt）
void lookat(const vec3 eye, const vec3 center, const vec3 up) {
    vec3 n = normalized(eye - center);          // 摄像机方向向量（从中心指向眼睛）
    vec3 l = normalized(cross(up, n));          // 摄像机右方向（x 轴）
    vec3 m = normalized(cross(n, l));           // 摄像机上方向（y 轴）

    // 构建旋转矩阵（将世界坐标转换到摄像机坐标）
    ModelView = mat<4, 4>{
        {{l.x,l.y,l.z,0},
         {m.x,m.y,m.z,0},
         {n.x,n.y,n.z,0},
         {0,0,0,1}}
    } *
        // 构建平移矩阵（将摄像机中心移到原点）
        mat<4, 4>{
            {{1, 0, 0, -center.x},
                { 0,1,0,-center.y },
                { 0,0,1,-center.z },
                { 0,0,0,1 }}
    };
}

// 构建透视投影矩阵
void perspective(const double f) {
    // 简化透视矩阵，f 为焦距
    Perspective = {
        {{1,0,0,0},
         {0,1,0,0},
         {0,0,1,0},
         {0,0,-1 / f,1}}
    };
}

// 构建屏幕视口变换矩阵
void viewport(const int x, const int y, const int w, const int h) {
    // 将归一化设备坐标（[-1,1]）映射到屏幕像素坐标
    Viewport = {
        {{w / 2., 0, 0, x + w / 2.},
         {0, h / 2., 0, y + h / 2.},
         {0, 0, 1, 0},
         {0, 0, 0, 1}}
    };
}

// 光栅化三角形（clip-space 顶点 -> 屏幕像素）
void rasterize(const vec4 clip[3], std::vector<double>& zbuffer, TGAImage& framebuffer, const TGAColor color) {
    // 齐次除法，将 clip-space 顶点转换为 NDC（[-1,1]）
    vec4 ndc[3] = { clip[0] / clip[0].w, clip[1] / clip[1].w, clip[2] / clip[2].w };

    // NDC -> 屏幕坐标
    vec2 screen[3] = {
        (Viewport * ndc[0]).xy(),
        (Viewport * ndc[1]).xy(),
        (Viewport * ndc[2]).xy()
    };

    // 构建矩阵 ABC，用于计算重心坐标
    mat<3, 3> ABC = {
        {{screen[0].x, screen[0].y, 1.},
         {screen[1].x, screen[1].y, 1.},
         {screen[2].x, screen[2].y, 1.}}
    };

    // 背面剔除 & 面积小于 1 的三角形直接丢弃
    if (ABC.det() < 1) return;

    // 计算三角形的包围盒（最小、最大 x/y）
    auto [bbminx, bbmaxx] = std::minmax({ screen[0].x, screen[1].x, screen[2].x });
    auto [bbminy, bbmaxy] = std::minmax({ screen[0].y, screen[1].y, screen[2].y });

#pragma omp parallel for
    for (int x = std::max<int>(bbminx, 0); x <= std::min<int>(bbmaxx, framebuffer.width() - 1); x++) {
        for (int y = std::max<int>(bbminy, 0); y <= std::min<int>(bbmaxy, framebuffer.height() - 1); y++) {
            // 计算重心坐标
            vec3 bc = ABC.invert_transpose() * vec3 { static_cast<double>(x), static_cast<double>(y), 1. };

            // 如果重心坐标任意分量为负，则像素在三角形外
            if (bc.x < 0 || bc.y < 0 || bc.z < 0) continue;

            // 插值计算深度 z
            double z = bc * vec3{ ndc[0].z, ndc[1].z, ndc[2].z };

            // 深度测试，如果被遮挡则跳过
            if (z <= zbuffer[x + y * framebuffer.width()]) continue;

            zbuffer[x + y * framebuffer.width()] = z; // 更新 Z-buffer
            framebuffer.set(x, y, color);           // 设置像素颜色
        }
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " obj/model.obj" << std::endl;
        return 1;
    }

    // 输出图像尺寸
    constexpr int width = 800;
    constexpr int height = 800;

    // 摄像机参数
    constexpr vec3 eye{ -1,0,2 };   // 摄像机位置
    constexpr vec3 center{ 0,0,0 }; // 观察目标
    constexpr vec3 up{ 0,1,0 };     // 上方向

    // 构建变换矩阵
    lookat(eye, center, up);                              // ModelView
    perspective(norm(eye - center));                      // Perspective
    viewport(width / 16, height / 16, width * 7 / 8, height * 7 / 8);// Viewport

    // 帧缓冲和 Z-buffer（初始值为最小值）
    TGAImage framebuffer(width, height, TGAImage::RGB);
    std::vector<double> zbuffer(width * height, -std::numeric_limits<double>::max());

    // 遍历所有输入模型
    for (int m = 1; m < argc; m++) {
        Model model(argv[m]);

        // 遍历每个三角形
        for (int i = 0; i < model.nfaces(); i++) {
            vec4 clip[3];

            // 构建三角形的 clip-space 顶点
            for (int d : {0, 1, 2}) {
                vec3 v = model.vert(i, d);
                clip[d] = Perspective * ModelView * vec4{ v.x, v.y, v.z, 1. };
            }

            // 随机颜色
            TGAColor rnd;
            for (int c = 0; c < 3; c++) rnd[c] = std::rand() % 255;

            // 光栅化三角形
            rasterize(clip, zbuffer, framebuffer, rnd);
        }
    }

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}
