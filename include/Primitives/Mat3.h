#pragma once

#include "Vec3.h"

#include <optional>

template<typename T>
struct Mat3 {
    Vec3<T> x, y, z;

    explicit Mat3() : x(0), y(0), z(0) {}
    explicit Mat3(const Vec3<T>& v) : x(v), y(v), z(v) {}
    Mat3(const Vec3<T>& x, const Vec3<T>& y, const Vec3<T>& z) : x(x), y(y), z(z) { }

    Vec3<T> operator*(const Vec3<T> &v) {
        return x * v.x + y * v.y + z * v.z;
    }

    Mat3<T> operator+(const Mat3<T> &v) {
        return {x + v.x, y + v.y, z + v.z};
    }

    T det() {
        return x.x * y.y * z.z
              -x.x * y.z * z.y
              -x.y * y.x * z.z
              -x.z * y.y * z.x
              +x.z * y.x * z.y
              +x.y * y.z * z.x;
    }

    std::optional<Vec3<T>> solve(const Vec3<T> &b) {
        T d = det();
        if (d <= 1e-9) {
            return std::nullopt;
        }
        T dx = (Mat3 {b, y, z}).det();
        T dy = (Mat3 {x, b, z}).det();
        T dz = (Mat3 {x, y, b}).det();
        return {{dx / d, dy / d, dz / d}};
    }
};
