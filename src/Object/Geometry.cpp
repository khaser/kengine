#include "Primitives.h"

#include "Object/Geometry.h"

#include <iostream>

std::vector<Intersection> Geometry::get_intersect(Ray ray) const {
    ray.start = -rotation * (ray.start - position);
    ray.v = -rotation * ray.v;
    std::vector<Intersection> res;
    for (float &t : get_intersect_(ray)) {
        Vec3<float> n = normal(ray.reveal(t));
        bool is_ins = ray.v % n >= 0;
        if (is_ins) {
            n = -n;
        }
        res.push_back({t, rotation * n, is_ins});
    }
    return res;
}
