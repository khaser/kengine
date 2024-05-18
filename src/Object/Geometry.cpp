#include "Primitives.h"

#include "Object/Geometry.h"

#include <iostream>

std::vector<Intersection> Geometry::get_intersect(Ray ray) const {
    ray.start = -rotation * (ray.start - position);
    ray.v = -rotation * ray.v;
    std::vector<Intersection> res;
    for (float &t : get_intersect_(ray)) {
        Vec3<float> n = normal(ray.reveal(t));
        bool is_ins = is_inside(ray, t);
        if (is_ins) {
            n = -n;
        }
        res.push_back({t, rotation * n, is_ins});
    }
    return res;
}

bool Geometry::is_inside(const Ray &ray, float t) const {
    return ray.v % normal(ray.reveal(t)) >= 0;
}
