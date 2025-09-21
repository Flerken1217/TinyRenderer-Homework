#include "MyGL.h"
#include "modelLoader.h"
#include <algorithm>
#include <vector>
#include <iostream>

extern mat<4, 4> Viewport, ModelView, Perspective;
extern std::vector<double> zbuffer;

// ----------------- Phong Shader -----------------
struct PhongShader : IShader {
    const Model& model;
    vec4 l;              // 光源方向（眼坐标系）
    vec2 varying_uv[3];  // 顶点 UV
    vec4 varying_nrm[3]; // 顶点法线
    vec4 tri[3];         // 三角形顶点（眼坐标系）

    PhongShader(const vec3 light, const Model& m) : model(m) {
        l = normalized(ModelView * vec4{ light.x, light.y, light.z, 0.0 });
    }

    virtual vec4 vertex(const int face, const int vert) {
        varying_uv[vert] = model.uv(face, vert);
        varying_nrm[vert] = ModelView.invert_transpose() * model.normal(face, vert);
        vec4 gl_Position = ModelView * model.vert(face, vert);
        tri[vert] = gl_Position;
        return Perspective * gl_Position;
    }

    virtual std::pair<bool, TGAColor> fragment(const vec3 bar) const {
        // 构建切线空间 Darboux frame
        mat<2, 4> E = { tri[1] - tri[0], tri[2] - tri[0] };
        mat<2, 2> U = { varying_uv[1] - varying_uv[0], varying_uv[2] - varying_uv[0] };
        mat<2, 4> T = U.invert() * E;
        mat<4, 4> D = { normalized(T[0]),
                       normalized(T[1]),
                       normalized(varying_nrm[0] * bar[0] + varying_nrm[1] * bar[1] + varying_nrm[2] * bar[2]),
                       {0,0,0,1} };

        vec2 uv = varying_uv[0] * bar[0] + varying_uv[1] * bar[1] + varying_uv[2] * bar[2];
        vec4 n = normalized(D.transpose() * model.normal(uv));
        vec4 r = normalized(n * (n * l) * 2 - l); // 反射光

        double ambient = 0.4;
        double diffuse = std::max(0.0, n * l);
        double specular = (3.0 * sample2D(model.specular(), uv)[0] / 255.0) * std::pow(std::max(r.z, 0.0), 35.0);

        TGAColor color = sample2D(model.diffuse(), uv);
        for (int i = 0; i < 3; i++)
            color[i] = static_cast<unsigned char>(std::min(255.0, color[i] * (ambient + diffuse + specular)));

        return { false, color };
    }
};

// ----------------- main -----------------
int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " obj/model.obj" << std::endl;
        return 1;
    }

    constexpr int width = 800;
    constexpr int height = 800;
    constexpr vec3  light{ 1,1,1 };
    constexpr vec3    eye{ -1,0,2 };
    constexpr vec3 center{ 0,0,0 };
    constexpr vec3     up{ 0,1,0 };

    lookat(eye, center, up);
    init_perspective(norm(eye - center));
    init_viewport(width / 16, height / 16, width * 7 / 8, height * 7 / 8);
    init_zbuffer(width, height);

    // 黑色背景 framebuffer
    TGAImage framebuffer(width, height, TGAImage::RGB, { 0,0,0,255 });

    for (int m = 1; m < argc; m++) {
        Model model(argv[m]);
        PhongShader shader(light, model);
        for (int f = 0; f < model.nfaces(); f++) {
            Triangle clip = { shader.vertex(f,0), shader.vertex(f,1), shader.vertex(f,2) };
            rasterize(clip, shader, framebuffer);
        }
    }

    framebuffer.flip_vertically();
    framebuffer.write_tga_file("framebuffer.tga");

    return 0;
}
