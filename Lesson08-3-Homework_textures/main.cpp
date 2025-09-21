#include "MyGL.h"
#include "modelLoader.h"
#include <algorithm>

extern mat<4, 4> ModelView, Perspective; // "OpenGL" ״̬����
extern std::vector<double> zbuffer;     // ��Ȼ�����

struct SimpleShader : IShader {
    const Model& model;
    vec3 tri[3];        // ����λ�� (eye space)
    vec3 nrm[3];        // ���㷨�� (eye space)
    vec2 uv[3];         // ���� UV

    vec3 light_dir;     // ��Դ�����ۿռ䣩

    SimpleShader(const Model& m, const vec3& light_world) : model(m) {
        // ��Դ�任���ۿռ�
        light_dir = normalized((ModelView * vec4(light_world.x, light_world.y, light_world.z, 0.0)).xyz());
    }

    virtual vec4 vertex(const int face, const int vert) {
        vec4 v_eye = ModelView * model.vert(face, vert);
        tri[vert] = v_eye.xyz();

        // ���߱任���ۿռ�
        nrm[vert] = normalized((ModelView.invert_transpose() * model.normal(face, vert)).xyz());

        uv[vert] = model.uv(face, vert);
        return Perspective * v_eye;
    }

    virtual std::pair<bool, TGAColor> fragment(const vec3 bar) const {
        // ��ֵ UV
        vec2 uvP = uv[0] * bar[0] + uv[1] * bar[1] + uv[2] * bar[2];

        // ��ֵ���ߺ�λ��
        vec3 N = normalized(nrm[0] * bar[0] + nrm[1] * bar[1] + nrm[2] * bar[2]);
        vec3 P = tri[0] * bar[0] + tri[1] * bar[1] + tri[2] * bar[2];

        // ���߷���
        vec3 V = normalized(-P);
        // ���䷽��
        vec3 R = normalized(N * 2.f * (N * light_dir) - light_dir);

        // ���� diffuse ��ɫ
        int u = std::min(std::max(int(uvP.x * model.diffuse().width()), 0), model.diffuse().width() - 1);
        int v = std::min(std::max(int(uvP.y * model.diffuse().height()), 0), model.diffuse().height() - 1);
        TGAColor diff_c = model.diffuse().get(u, v);

        // ���� specular ǿ��
        u = std::min(std::max(int(uvP.x * model.specular().width()), 0), model.specular().width() - 1);
        v = std::min(std::max(int(uvP.y * model.specular().height()), 0), model.specular().height() - 1);
        TGAColor spec_c = model.specular().get(u, v);
        float spec_intensity = spec_c[0] / 255.f;

        // ����ϵ��
        float ka = 0.3f;                    // ������
        float kd = std::max(0.f, float(N * light_dir)); // ������
        float ks = powf(std::max(0.f, float(R * V)), 32.f) * spec_intensity * 0.5f; // �߹�

        // �ϳ���ɫ
        vec3 base = vec3{ diff_c[2] / 255.f, diff_c[1] / 255.f, diff_c[0] / 255.f };
        vec3 color = base * (ka + kd) + vec3{ ks, ks, ks };

        // clamp
        color.x = std::min(1.f, float(color.x));
        color.y = std::min(1.f, float(color.y));
        color.z = std::min(1.f, float(color.z));

        // ��� TGAColor
        TGAColor c;
        c[0] = (unsigned char)(color.z * 255);
        c[1] = (unsigned char)(color.y * 255);
        c[2] = (unsigned char)(color.x * 255);
        c[3] = 255;
        return { false, c };
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
        SimpleShader shader(model, light);

        for (int f = 0; f < model.nfaces(); f++) {
            Triangle clip = {
                shader.vertex(f, 0),
                shader.vertex(f, 1),
                shader.vertex(f, 2)
            };
            rasterize(clip, shader, framebuffer);
        }
    }

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}
