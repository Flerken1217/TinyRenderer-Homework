#include <algorithm>
#include "MyGL.h"

mat<4, 4> ModelView, Viewport, Perspective; // 全局矩阵：模型视图、视口、透视
std::vector<double> zbuffer;               // 全局 Z-buffer，用于深度测试

// ------------------- 摄像机函数 -------------------
void lookat(const vec3 eye, const vec3 center, const vec3 up) {
    vec3 n = normalized(eye - center);          // 摄像机朝向（视线反方向）
    vec3 l = normalized(cross(up, n));          // 右方向向量
    vec3 m = normalized(cross(n, l));           // 真正的上方向向量
    // 构建 ModelView 矩阵：旋转 * 平移
    ModelView = mat<4, 4>{ {{l.x,l.y,l.z,0}, {m.x,m.y,m.z,0}, {n.x,n.y,n.z,0}, {0,0,0,1}} } *
        mat<4, 4>{{{1, 0, 0, -center.x}, { 0,1,0,-center.y }, { 0,0,1,-center.z }, { 0,0,0,1 }}};
}

// ------------------- 透视投影矩阵 -------------------
void init_perspective(const double f) {
    // 简单透视矩阵，将 z 映射到 [-1,1] 范围
    Perspective = { {{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,-1 / f,1}} };
}

// ------------------- 视口矩阵 -------------------
void init_viewport(const int x, const int y, const int w, const int h) {
    // 将标准化设备坐标（[-1,1]）映射到屏幕坐标
    Viewport = { {{w / 2., 0, 0, x + w / 2.},
                  {0, h / 2., 0, y + h / 2.},
                  {0,0,1,0},
                  {0,0,0,1}} };
}

// ------------------- Z-buffer 初始化 -------------------
void init_zbuffer(const int width, const int height) {
    // 初始化为一个很小的值，表示最远深度
    zbuffer = std::vector(width * height, -1000.);
}

// ------------------- 光栅化函数 -------------------
void rasterize(const Triangle& clip, const IShader& shader, TGAImage& framebuffer) {
    // 将三角形顶点从裁剪空间归一化到 NDC 空间
    vec4 ndc[3] = { clip[0] / clip[0].w, clip[1] / clip[1].w, clip[2] / clip[2].w };
    // 将 NDC 坐标映射到屏幕坐标
    vec2 screen[3] = { (Viewport * ndc[0]).xy(), (Viewport * ndc[1]).xy(), (Viewport * ndc[2]).xy() };

    // 构建 3x3 矩阵 ABC，用于求重心坐标
    mat<3, 3> ABC = { { {screen[0].x, screen[0].y, 1.},
                        {screen[1].x, screen[1].y, 1.},
                        {screen[2].x, screen[2].y, 1.} } };
    if (ABC.det() < 1) return; // 面剔除 + 丢弃面积小于一个像素的三角形

    // 计算三角形的边界框
    auto [bbminx, bbmaxx] = std::minmax({ screen[0].x, screen[1].x, screen[2].x });
    auto [bbminy, bbmaxy] = std::minmax({ screen[0].y, screen[1].y, screen[2].y });

    // 遍历边界框内所有像素
#pragma omp parallel for
    for (int x = std::max<int>(bbminx, 0); x <= std::min<int>(bbmaxx, framebuffer.width() - 1); x++) {
        for (int y = std::max<int>(bbminy, 0); y <= std::min<int>(bbmaxy, framebuffer.height() - 1); y++) {
            // 计算屏幕重心坐标
            vec3 bc_screen = ABC.invert_transpose() * vec3 { static_cast<double>(x), static_cast<double>(y), 1. };
            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue; // 不在三角形内

            // 透视修正：从屏幕重心坐标到裁剪空间重心坐标
            vec3 bc_clip = { bc_screen.x / clip[0].w, bc_screen.y / clip[1].w, bc_screen.z / clip[2].w };
            bc_clip = bc_clip / (bc_clip.x + bc_clip.y + bc_clip.z); // 归一化

            // 插值深度值（线性插值 NDC.z）
            double z = bc_screen * vec3{ ndc[0].z, ndc[1].z, ndc[2].z };
            if (z <= zbuffer[x + y * framebuffer.width()]) continue; // 深度测试

            // 调用片元着色器获取颜色
            auto [discard, color] = shader.fragment(bc_clip);
            if (discard) continue; // 如果着色器舍弃，跳过

            // 更新 Z-buffer 和帧缓冲
            zbuffer[x + y * framebuffer.width()] = z;
            framebuffer.set(x, y, color);
        }
    }
}
