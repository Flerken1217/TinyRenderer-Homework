#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <cmath>
#include <iostream>

//2D点类
struct Vec2i {
    int x;
    int y;

    Vec2i() : x(0), y(0) {}
    Vec2i(int x_, int y_) : x(x_), y(y_) {}
};

// vec3 类：表示 3D 向量或点
class vec3 {
public:
    float x, y, z; // 三个坐标分量

    // 默认构造函数，初始化为 (0, 0, 0)
    vec3() : x(0), y(0), z(0) {}

    // 构造函数，初始化为 (x_, y_, z_)
    vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

    // 向量加法
    vec3 operator+(const vec3& other) const {
        return vec3(x + other.x, y + other.y, z + other.z);
    }

    // 向量减法
    vec3 operator-(const vec3& other) const {
        return vec3(x - other.x, y - other.y, z - other.z);
    }

    // 向量数乘
    vec3 operator*(float scalar) const {
        return vec3(x * scalar, y * scalar, z * scalar);
    }

    // 向量点积
    float dot(const vec3& other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    // 向量叉积
    vec3 cross(const vec3& other) const {
        return vec3(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        );
    }

    // 向量长度
    float length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    // 输出向量（调试用）
    void print() const {
        std::cout << "(" << x << ", " << y << ", " << z << ")" << std::endl;
    }
};

#endif // GEOMETRY_H
