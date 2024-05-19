#pragma once

#include "Vec3.h"

struct Ray {
    Vec3<float> start;
    Vec3<float> v; // normalized ray direction vector

    Vec3<float> reveal(float t) const {
        return start + v * t;
    }

    void bump() {
        start = start + v * 1e-4;
    }
};

