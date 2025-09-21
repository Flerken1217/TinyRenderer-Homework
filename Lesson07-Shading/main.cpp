#include "MyGL.h"
#include "modelLoader.h"

extern mat<4, 4> ModelView, Perspective; // "OpenGL" state matrices and
extern std::vector<double> zbuffer;     // the depth buffer

struct PhongShader : IShader {
    const Model& model;
    vec3 tri_eye[3];   // �����ζ��� (eye space)
    vec3 nrm[3];       // �����ζ��㷨�� (eye space)

    PhongShader(const Model& m) : model(m) {}

    // ������ɫ��
    virtual vec4 vertex(const int face, const int vert) {
        vec4 v_obj = model.vert(face, vert);
        vec4 v_eye = ModelView * v_obj;
        tri_eye[vert] = v_eye.xyz();

        vec4 n_obj = model.normal(face, vert);
        nrm[vert] = (ModelView * n_obj).xyz(); // ���߱任�� eye space

        return Perspective * v_eye; // ���زü�����
    }

    // Ƭ����ɫ��
    virtual std::pair<bool, TGAColor> fragment(const vec3 bar) const {
        // ����������ƽ�淨�ߣ����������£�
        vec3 N = normalized(cross(tri_eye[1] - tri_eye[0], tri_eye[2] - tri_eye[0]));

        // ��Դ����
        vec3 L = normalized(vec3{ 1, 1, 1 });

        // ����λ��
        vec3 P = tri_eye[0] * bar.x + tri_eye[1] * bar.y + tri_eye[2] * bar.z;

        // ���߷���
        vec3 V = normalized(-P);

        // ��������
        vec3 R = normalized(2.f * (N * L) * N - L);

        // ���ղ���
        float ka = 0.1f;                      // ������
        float kd = std::max(0.f, float(N * L));      // ������
        float ks = powf(std::max(0.f, float(R * V)), 32.f); // �߹�

        vec3 color(1.0f, 0.8f, 0.7f);        // ��������ɫ
        vec3 intensity = color * (ka + kd + ks);


        // ���Ʒ�Χ
        intensity.x = std::min(1.f, float(intensity.x));
        intensity.y = std::min(1.f, float(intensity.y));
        intensity.z = std::min(1.f, float(intensity.z));

        TGAColor out = {
            static_cast<unsigned char>(intensity.x * 255),
            static_cast<unsigned char>(intensity.y * 255),
            static_cast<unsigned char>(intensity.z * 255),
            255
        };

        return { false, out };
    }


};


int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " obj/model.obj" << std::endl;
        return 1;
    }

    constexpr int width = 800;      // output image size
    constexpr int height = 800;
    constexpr vec3  light{ 1, 1, 1 }; // light source
    constexpr vec3    eye{ -1, 0, 2 }; // camera position
    constexpr vec3 center{ 0, 0, 0 }; // camera direction
    constexpr vec3     up{ 0, 1, 0 }; // camera up vector

    lookat(eye, center, up);                                   // build the ModelView   matrix
    init_perspective(norm(eye - center));                        // build the Perspective matrix
    init_viewport(width / 16, height / 16, width * 7 / 8, height * 7 / 8); // build the Viewport    matrix
    init_zbuffer(width, height);
    TGAImage framebuffer(width, height, TGAImage::RGB);

    for (int m = 1; m < argc; m++) {
        Model model(argv[m]);
        PhongShader shader(model);
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
