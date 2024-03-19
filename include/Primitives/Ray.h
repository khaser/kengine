#pragma once

#include "Vec3.h"

struct Ray {
    Vec3<double> start;
    Vec3<double> v; // normalized ray direction vector

    Vec3<double> reveal(double t) const {
        return start + v * t;
    }

    void bump() {
        start = start + v * 1e-9;
    }
};

