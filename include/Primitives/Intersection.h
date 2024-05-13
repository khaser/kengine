#pragma once

#include "Vec3.h"

struct Intersection {
    float t;
    Vec3<float> normal;
    bool is_inside;
};

