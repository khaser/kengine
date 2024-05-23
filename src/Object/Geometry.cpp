#include "Primitives.h"

#include "Object/Geometry.h"

#include <iostream>

Intersection Geometry::get_intersect(Ray ray) const {
    ray.start = -rotation * (ray.start - position);
    ray.v = -rotation * ray.v;
    std::vector<Intersection> res;
    float t = get_intersect_(ray);
    if (t < 0) {
        return {t, Vec3<float>(), false};
    }
    Vec3<float> n = normal(ray.reveal(t));
    bool is_ins = ray.v % n >= 0;
    if (is_ins) {
        n = -n;
    }
    return {t, rotation * n, is_ins};
}
