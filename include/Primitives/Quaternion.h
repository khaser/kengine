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

/* template<typename T> */
/* Vec3<T> operator*(const Quaternion &q, const Vec3<T> &v) { */
/*     Quaternion res = q * (Quaternion {v, 0}) * -q; */
/*     return res.v; */
/* } */

// It seems faster
template<typename T>
Vec3<T> operator*(const Quaternion &q, const Vec3<T> &v) {
    float vx = q.v.y*v.z-q.v.z*v.y;
    float vy = q.v.z*v.x-q.v.x*v.z;
    float vz = q.v.x*v.y-q.v.y*v.x;
    vx += vx; vy += vy; vz += vz;
    return {
        v.x + q.w*vx + q.v.y*vz-q.v.z*vy,
        v.y + q.w*vy + q.v.z*vx-q.v.x*vz,
        v.z + q.w*vz + q.v.x*vy-q.v.y*vx
    };
}


template<typename T>
Mat3<T> operator*(const Quaternion &q, const Mat3<T> &m) {
    return {q * m.x, q * m.y, q * m.z};
}

std::istream& operator>>(std::istream &, Quaternion &);
