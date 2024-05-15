#include "Primitives.h"

#include "Object/Geometry.h"

std::vector<Intersection> Geometry::get_intersect(Ray ray) const {
    ray.start = -rotation * (ray.start - position);
    ray.v = -rotation * ray.v;
    std::vector<Intersection> res;
    for (float &t : get_intersect_(ray)) {
        Vec3<float> n = normal(ray.reveal(t));
        if (fabs(n.len() - 1) > 1e-5) {
            throw std::logic_error("wrong normal length");
        }
        bool is_ins = is_inside(ray, t);
        if (is_ins) {
            n = -n;
        }
        n = (rotation * n).norm();
        res.push_back({t, n, is_ins});
    }
    return res;
}

bool Geometry::is_inside(const Ray &ray, float t) const {
    return ray.v % normal(ray.reveal(t)) >= 0;
}
