#pragma once
#include <cmath>
#include <cassert>
#include <iostream>

// 通用 vec<n> 模板
template<int n> struct vec {
    double data[n] = { 0 };

    vec() = default;
    vec(const std::initializer_list<double>& l) {
        assert(l.size() == n);
        int i = 0;
        for (auto v : l) data[i++] = v;
    }

    double& operator[](int i) { assert(i >= 0 && i < n); return data[i]; }
    double  operator[](int i) const { assert(i >= 0 && i < n); return data[i]; }

    vec<n> operator+(const vec<n>& o) const {
        vec<n> res;
        for (int i = 0; i < n; i++) res[i] = data[i] + o[i];
        return res;
    }

    vec<n> operator-(const vec<n>& o) const {
        vec<n> res;
        for (int i = 0; i < n; i++) res[i] = data[i] - o[i];
        return res;
    }

    vec<n> operator*(double f) const {
        vec<n> res;
        for (int i = 0; i < n; i++) res[i] = data[i] * f;
        return res;
    }

    vec<n> operator/(double f) const {
        assert(f != 0);
        vec<n> res;
        for (int i = 0; i < n; i++) res[i] = data[i] / f;
        return res;
    }

    double dot(const vec<n>& o) const {
        double res = 0;
        for (int i = 0; i < n; i++) res += data[i] * o[i];
        return res;
    }

    double norm() const { return std::sqrt(this->dot(*this)); }
    vec<n> normalize() const { return *this / norm(); }
};

template<> struct vec<3> {
    double x = 0, y = 0, z = 0;

    vec<3>() = default;
    vec<3>(double X, double Y, double Z) : x(X), y(Y), z(Z) {}

    double& operator[](int i) { assert(i >= 0 && i < 3); return i == 0 ? x : (i == 1 ? y : z); }
    double  operator[](int i) const { assert(i >= 0 && i < 3); return i == 0 ? x : (i == 1 ? y : z); }

    vec<3> operator+(const vec<3>& o) const { return { x + o.x, y + o.y, z + o.z }; }
    vec<3> operator-(const vec<3>& o) const { return { x - o.x, y - o.y, z - o.z }; }
    vec<3> operator*(double f) const { return { x * f, y * f, z * f }; }
    vec<3> operator/(double f) const { assert(f != 0); return { x / f, y / f, z / f }; }

    double dot(const vec<3>& o) const { return x * o.x + y * o.y + z * o.z; }
    vec<3> cross(const vec<3>& o) const { return { y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x }; }
    double norm() const { return std::sqrt(dot(*this)); }
    vec<3> normalize() const { return *this / norm(); }
};

typedef vec<2> vec2;
typedef vec<3> vec3;
typedef vec<4> vec4;

template<int n>
std::ostream& operator<<(std::ostream& out, const vec<n>& v) {
    for (int i = 0; i < n; i++) out << v[i] << " ";
    return out;
}

template<int n> struct Mat {
    double data[n][n] = {};

    Mat() = default;

    double* operator[](int i) { return data[i]; }
    const double* operator[](int i) const { return data[i]; }

    Mat<n> operator*(const Mat<n>& o) const {
        Mat<n> res;
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                for (int k = 0; k < n; k++)
                    res[i][j] += data[i][k] * o[k][j];
        return res;
    }

    Mat<n> transpose() const {
        Mat<n> res;
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                res[i][j] = data[j][i];
        return res;
    }
};

// 3x3 求逆
Mat<3> inverse(const Mat<3>& m) {
    Mat<3> res;
    double det = m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
        - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])
        + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
    assert(det != 0);
    double invdet = 1. / det;

    res[0][0] = (m[1][1] * m[2][2] - m[1][2] * m[2][1]) * invdet;
    res[0][1] = -(m[0][1] * m[2][2] - m[0][2] * m[2][1]) * invdet;
    res[0][2] = (m[0][1] * m[1][2] - m[0][2] * m[1][1]) * invdet;

    res[1][0] = -(m[1][0] * m[2][2] - m[1][2] * m[2][0]) * invdet;
    res[1][1] = (m[0][0] * m[2][2] - m[0][2] * m[2][0]) * invdet;
    res[1][2] = -(m[0][0] * m[1][2] - m[0][2] * m[1][0]) * invdet;

    res[2][0] = (m[1][0] * m[2][1] - m[1][1] * m[2][0]) * invdet;
    res[2][1] = -(m[0][0] * m[2][1] - m[0][1] * m[2][0]) * invdet;
    res[2][2] = (m[0][0] * m[1][1] - m[0][1] * m[1][0]) * invdet;

    return res;
}
