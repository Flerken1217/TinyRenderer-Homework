#include "tgaimage.h" 
#include "geometry.h"   

// 设置摄像机视角的函数
void lookat(const vec3 eye, const vec3 center, const vec3 up);

// 初始化透视投影矩阵
void init_perspective(const double f);

// 初始化视口矩阵（屏幕坐标映射）
void init_viewport(const int x, const int y, const int w, const int h);

// 初始化深度缓存（Z-buffer）
void init_zbuffer(const int width, const int height);

// 抽象的着色器接口，定义片元着色函数
struct IShader {
    virtual std::pair<bool, TGAColor> fragment(const vec3 bar) const = 0;
    // 返回值 first 表示是否舍弃该片元，second 表示颜色
};

// 定义三角形类型：由三个有序的四维点组成
typedef vec4 Triangle[3];

// 核心光栅化函数，将三角形片元绘制到帧缓冲
void rasterize(const Triangle& clip, const IShader& shader, TGAImage& framebuffer);