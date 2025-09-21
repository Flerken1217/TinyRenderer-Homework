#pragma once
#include <cstdint>
#include <fstream>
#include <vector>

#pragma pack(push,1)
struct TGAHeader {
    std::uint8_t  idlength = 0;
    std::uint8_t  colormaptype = 0;
    std::uint8_t  datatypecode = 0;
    std::uint16_t colormaporigin = 0;
    std::uint16_t colormaplength = 0;
    std::uint8_t  colormapdepth = 0;
    std::uint16_t x_origin = 0;
    std::uint16_t y_origin = 0;
    std::uint16_t width = 0;
    std::uint16_t height = 0;
    std::uint8_t  bitsperpixel = 0;
    std::uint8_t  imagedescriptor = 0;
};
#pragma pack(pop)

struct TGAColor {
    std::uint8_t bgra[4] = { 0,0,0,0 };
    std::uint8_t bytespp = 4;

    std::uint8_t& operator[](const int i) { return bgra[i]; }
    const std::uint8_t& operator[](const int i) const { return bgra[i]; }
};


//struct TGAColor {
//    union {
//        struct {
//            std::uint8_t b, g, r, a;
//        };
//        std::uint8_t raw[4];   // ���ֽ�ֱ�ӷ���
//        std::uint32_t val;     // ���� 32bit �洢
//    };
//    std::uint8_t bytespp = 1;  // ÿ�����ֽ��� (1=�Ҷ�, 3=RGB, 4=RGBA)
//
//    // Ĭ�Ϲ��죺��ɫ��1�ֽ� (�Ҷ�)
//    TGAColor() : val(0), bytespp(1) {}
//
//    // ָ�� RGBA (Ĭ�� alpha=255)��bpp=4
//    TGAColor(std::uint8_t R, std::uint8_t G, std::uint8_t B, std::uint8_t A = 255) {
//        b = B; g = G; r = R; a = A;
//        bytespp = 4;
//    }
//
//    // ������ֱ�ӱ�ʾ����ֵ�������ֽ���
//    TGAColor(std::uint32_t v, std::uint8_t bpp) : val(v), bytespp(bpp) {}
//
//    // ����һ����ɫ����
//    TGAColor(const TGAColor& c) : val(c.val), bytespp(c.bytespp) {}
//
//    // ���ֽ����鿽����ָ�� bpp
//    TGAColor(const std::uint8_t* p, std::uint8_t bpp) : val(0), bytespp(bpp) {
//        std::memcpy(raw, p, bpp);
//    }
//
//    // ��ֵ�����
//    TGAColor& operator=(const TGAColor& c) {
//        if (this != &c) {
//            val = c.val;
//            bytespp = c.bytespp;
//        }
//        return *this;
//    }
//
//    // �±����
//    std::uint8_t& operator[](int i) { return raw[i]; }
//    const std::uint8_t& operator[](int i) const { return raw[i]; }
//};


struct TGAImage {
    enum Format { GRAYSCALE = 1, RGB = 3, RGBA = 4 };
    TGAImage() = default;
    TGAImage(const int w, const int h, const int bpp, TGAColor c = {});
    bool  read_tga_file(const std::string filename);
    bool write_tga_file(const std::string filename, const bool vflip = true, const bool rle = true) const;
    void flip_horizontally();
    void flip_vertically();
    TGAColor get(const int x, const int y) const;
    void set(const int x, const int y, const TGAColor& c);
    int width()  const;
    int height() const;
private:
    bool   load_rle_data(std::ifstream& in);
    bool unload_rle_data(std::ofstream& out) const;
    int w = 0, h = 0;
    std::uint8_t bpp = 0;
    std::vector<std::uint8_t> data = {};
};