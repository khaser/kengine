#pragma once

#include "Vec3.h"
#include "Mat3.h"

struct Quaternion {
    Vec3<double> v = {0, 0, 0};
    double w = 1;
    Quaternion operator*(const Quaternion& oth) const {
        return {v * oth.w + oth.v * w + (v ^ oth.v), w * oth.w - v % oth.v};
    }
    Quaternion operator-() const {
        return {-v, w};
    }
};

template<typename T>
Vec3<T> operator*(const Quaternion &q, const Vec3<T> &v) {
    Quaternion res = q * (Quaternion {v, 0}) * -q;
    return res.v;
}

template<typename T>
Mat3<T> operator*(const Quaternion &q, const Mat3<T> &m) {
    return {q * m.x, q * m.y, q * m.z};
}

std::istream& operator>>(std::istream &, Quaternion &);
