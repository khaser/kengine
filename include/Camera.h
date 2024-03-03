#pragma once

#include <cmath>

#include "Vec3.h"
#include "Ray.h"

struct Camera {
    Vec3<double> position;
    Vec3<double> right;
    Vec3<double> up;
    Vec3<double> forward;
    double fov_x;
    double fov_y;
    void calc_fov_y(int w, int h) {
        // w/h = tan(x/2)/tan(y/2)
        fov_y = 2 * atan(h * tan(fov_x/2) / w);
    }

    // x, y in [-1, 1]
    Ray raycast(double x, double y) {
        Vec3<double> v = {x * tan(fov_x / 2), y * tan(fov_y / 2), 1};
        Vec3<double> v_rotated = forward * v.z + up * v.y + right * v.x;
        return Ray {position, v_rotated.norm()};
    }
};
