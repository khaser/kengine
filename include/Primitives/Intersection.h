#pragma once

#include "Vec3.h"

struct Intersection {
    double t;
    Vec3<double> normal;
    bool is_inside;
};

