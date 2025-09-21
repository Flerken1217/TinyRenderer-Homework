#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <cmath>
#include <iostream>

//2D����
struct Vec2i {
    int x;
    int y;

    Vec2i() : x(0), y(0) {}
    Vec2i(int x_, int y_) : x(x_), y(y_) {}
};

// vec3 �ࣺ��ʾ 3D �������
class vec3 {
public:
    float x, y, z; // �����������

    // Ĭ�Ϲ��캯������ʼ��Ϊ (0, 0, 0)
    vec3() : x(0), y(0), z(0) {}

    // ���캯������ʼ��Ϊ (x_, y_, z_)
    vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

    // �����ӷ�
    vec3 operator+(const vec3& other) const {
        return vec3(x + other.x, y + other.y, z + other.z);
    }

    // ��������
    vec3 operator-(const vec3& other) const {
        return vec3(x - other.x, y - other.y, z - other.z);
    }

    // ��������
    vec3 operator*(float scalar) const {
        return vec3(x * scalar, y * scalar, z * scalar);
    }

    // �������
    float dot(const vec3& other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    // �������
    vec3 cross(const vec3& other) const {
        return vec3(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        );
    }

    // ��������
    float length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    // ��������������ã�
    void print() const {
        std::cout << "(" << x << ", " << y << ", " << z << ")" << std::endl;
    }
};

#endif // GEOMETRY_H
