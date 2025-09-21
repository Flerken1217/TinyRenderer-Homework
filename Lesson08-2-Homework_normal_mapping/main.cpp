#include "MyGL.h"
#include "modelLoader.h"

extern mat<4, 4> ModelView, Perspective; // "OpenGL" ״̬����
extern std::vector<double> zbuffer;     // ��Ȼ�����

// ------------------------
// Normal Map Shader
// ------------------------
struct NormalMapShader : IShader {
    const Model& model;
    vec3 tri_eye[3];   // �����ζ��� (eye space)
    vec3 nrm[3];       // ���㷨�� (eye space)
    vec2 uv_coords[3]; // ���� UV
    int face_idx;

    NormalMapShader(const Model& m) : model(m) {}

    // ������ɫ��
    virtual vec4 vertex(const int face, const int vert) {
        face_idx = face;

        vec4 v_obj = model.vert(face, vert);       // ģ�Ϳռ�
        vec4 v_eye = ModelView * v_obj;            // eye space
        tri_eye[vert] = v_eye.xyz();

        vec4 n_obj = model.normal(face, vert);     // ģ�Ϳռ䷨��
        nrm[vert] = (ModelView * n_obj).xyz();     // eye space
        uv_coords[vert] = model.uv(face, vert);    // ���� UV

        return Perspective * v_eye;
    }

    // Ƭ����ɫ��
    virtual std::pair<bool, TGAColor> fragment(const vec3 bar) const {
        // ------------------------
        // 1. ��ֵ UV
        // ------------------------
        vec2 uv = uv_coords[0] * bar.x + uv_coords[1] * bar.y + uv_coords[2] * bar.z;

        // ���� y ����ת
        uv.y = 1.0f - uv.y;

        // ------------------------
        // 2. �� normal map ��ȡ����
        // ------------------------
        vec3 N = model.normal(uv).xyz();
        N = normalized(N);

        // ------------------------
        // 3. Phong ����
        // ------------------------
        vec3 L = normalized(vec3{ 1, 1, 1 });       // ��Դ����
        vec3 P = tri_eye[0] * bar.x + tri_eye[1] * bar.y + tri_eye[2] * bar.z; // ����λ��
        vec3 V = normalized(-P);                  // ���߷���
        vec3 R = normalized(N * (N * L) * 2.f - L); // ��������

        float ka = 0.1f;
        float kd = std::max(0.f, float(N * L));
        float ks = powf(std::max(0.f, float(R * V)), 32.f);

        vec3 base_color{ 1.0f, 0.8f, 0.7f }; // ��������ɫ
        vec3 intensity = base_color * (ka + kd + ks);

        // ���Ʒ�Χ
        intensity.x = std::min(1.f, float(intensity.x));
        intensity.y = std::min(1.f, float(intensity.y));
        intensity.z = std::min(1.f, float(intensity.z));

        TGAColor out{
            static_cast<unsigned char>(intensity.x * 255),
            static_cast<unsigned char>(intensity.y * 255),
            static_cast<unsigned char>(intensity.z * 255),
            255
        };
        return { false, out };
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

    lookat(eye, center, up);
    init_perspective(norm(eye - center));
    init_viewport(width / 16, height / 16, width * 7 / 8, height * 7 / 8);
    init_zbuffer(width, height);

    TGAImage framebuffer(width, height, TGAImage::RGB);

    for (int m = 1; m < argc; m++) {
        Model model(argv[m]);
        NormalMapShader shader(model);

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
