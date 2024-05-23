#pragma once

#include "Vec3.h"
#include "Quaternion.h"

#include <optional>

struct Mat4 {
    Quaternion x, y, z, w; // each of them is column

    explicit Mat4(const Quaternion& v) : x(v), y(v), z(v), w(v) {}
    Mat4(const Quaternion& x, const Quaternion& y, const Quaternion& z, const Quaternion& w) : x(x), y(y), z(z), w(w) { }

    Quaternion operator*(const Quaternion &v) {
        return {{
            v.v.x * x.v.x + v.v.y * y.v.x + v.v.z * z.v.x + v.w * w.v.x,
            v.v.x * x.v.y + v.v.y * y.v.y + v.v.z * z.v.y + v.w * w.v.y,
            v.v.x * x.v.z + v.v.y * y.v.z + v.v.z * z.v.z + v.w * w.v.z
            },
            v.v.x * x.w + v.v.y * y.w + v.v.z * z.w + v.w * w.w};
    }

};
