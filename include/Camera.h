#pragma once

#include <cmath>

#include "Primitives/Vec3.h"
#include "Primitives/Ray.h"

struct Camera {
    Vec3<float> position;
    Vec3<float> right;
    Vec3<float> up;
    Vec3<float> forward;
    float fov_x;
    float fov_y;
    void calc_fov_y(int w, int h) {
        // w/h = tan(x/2)/tan(y/2)
        fov_y = 2 * atanf(h * tanf(fov_x/2) / w);
    }

    void calc_fov_x(int w, int h) {
        fov_x = 2 * atanf(w * tanf(fov_y/2) / h);
    }

    // x, y in [-1, 1]
    Ray raycast(float x, float y) {
        Vec3<float> v = {x * tanf(fov_x / 2), y * tanf(fov_y / 2), 1};
        Vec3<float> v_rotated = forward * v.z + up * v.y + right * v.x;
        return Ray {position, v_rotated.norm()};
    }
};
