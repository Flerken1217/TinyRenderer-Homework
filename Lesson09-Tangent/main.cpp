#include "MyGL.h"
#include "modelLoader.h"
#include <algorithm>

extern mat<4, 4> ModelView, Perspective; // "OpenGL" ״̬����
extern std::vector<double> zbuffer;     // ���ӽ���Ȼ�����

struct TangentShader : IShader {
    const Model& model;
    vec4 tri[3];           // ����λ�ã�eye space, vec4��
    vec4 varying_nrm[3];   // ���㷨�ߣ�eye space, vec4��
    vec2 varying_uv[3];    // ���� UV
    vec4 l;                // ��Դ����eye space��

    TangentShader(const Model& m, const vec3& light) : model(m) {
        // ����Դ����ת���� eye space
        l = normalized(ModelView * vec4{ light.x, light.y, light.z, 0.0 });
    }

    // ������ɫ��
    virtual vec4 vertex(const int face, const int vert) {
        varying_uv[vert] = model.uv(face, vert);
        varying_nrm[vert] = ModelView.invert_transpose() * model.normal(face, vert); // ���߱任
        vec4 v_eye = ModelView * model.vert(face, vert);
        tri[vert] = v_eye;
        return Perspective * v_eye; // ��� clip space
    }

    // Ƭ����ɫ��
    virtual std::pair<bool, TGAColor> fragment(const vec3 bar) const {
        // ��ֵ UV
        vec2 uv = varying_uv[0] * bar[0] + varying_uv[1] * bar[1] + varying_uv[2] * bar[2];

        // �������߿ռ� TBN
        mat<2, 4> E = { tri[1] - tri[0], tri[2] - tri[0] };
        mat<2, 2> U = { varying_uv[1] - varying_uv[0], varying_uv[2] - varying_uv[0] };
        mat<2, 4> T = U.invert() * E;
        mat<4, 4> D = {
            normalized(T[0]), // ����
            normalized(T[1]), // ������
            normalized(varying_nrm[0] * bar[0] + varying_nrm[1] * bar[1] + varying_nrm[2] * bar[2]), // ��ֵ����
            {0,0,0,1}
        };

        // �ӷ�����ͼ��������
        vec4 n = normalized(D.transpose() * model.normal(uv));

        // ��������
        vec4 r = normalized(n * (n * l) * 2.0 - l);

        // ������ + ������ + �߹⣨PhongShader �䷽��
        double ambient = 0.4;
        double diffuse = std::max(0.0, n * l);
        double specular = (3.0 * model.specular().get(
            std::min(std::max(int(uv.x * model.specular().width()), 0), model.specular().width() - 1),
            std::min(std::max(int(uv.y * model.specular().height()), 0), model.specular().height() - 1)
        )[0] / 255.0) * std::pow(std::max(r.z, 0.0), 35);

        // ���� diffuse ��ɫ
        TGAColor fragColor = model.diffuse().get(
            std::min(std::max(int(uv.x * model.diffuse().width()), 0), model.diffuse().width() - 1),
            std::min(std::max(int(uv.y * model.diffuse().height()), 0), model.diffuse().height() - 1)
        );

        // ������ɫ�ϳ�
        for (int i : {0, 1, 2})
            fragColor[i] = std::min<int>(255, fragColor[i] * (ambient + diffuse + specular));

        return { false, fragColor }; // ����������
    }
};


// ------------------------
// main
// ------------------------
int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " obj/model.obj" << std::endl;
        return 1;
    }

    constexpr int width = 800;
    constexpr int height = 800;
    constexpr vec3 eye{ -1, 0, 2 };
    constexpr vec3 center{ 0, 0, 0 };
    constexpr vec3 up{ 0, 1, 0 };
    constexpr vec3 light{ 1, 1, 1 }; // ���������Դ

    lookat(eye, center, up);
    init_perspective(norm(eye - center));
    init_viewport(width / 16, height / 16, width * 7 / 8, height * 7 / 8);
    init_zbuffer(width, height);

    TGAImage framebuffer(width, height, TGAImage::RGB);

    for (int m = 1; m < argc; m++) {
        Model model(argv[m]);
        TangentShader shader(model, light);

        for (int f = 0; f < model.nfaces(); f++) {
            Triangle clip = {
                shader.vertex(f, 0),
                shader.vertex(f, 1),
                shader.vertex(f, 2)
            };
            rasterize(clip, shader, framebuffer);
        }
    }
    framebuffer.flip_vertically();
    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}