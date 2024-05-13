#include "Primitives/Vec3.h"

#include "Object/Geometry.h"

#include <vector>

Plane::Plane(const Vec3<float> &v) : norm(v.norm()) {}

Vec3<float> Plane::normal(const Vec3<float> &p) const {
    return norm;
}

std::vector<float> Plane::get_intersect_(const Ray& ray) const {
    float t = -(ray.start % norm) / (ray.v % norm);
    std::vector<float> res;
    if (t > 0) res.push_back(t);
    return res;
};

Box Plane::AABB() const {
    return Box({0, 0, 0}, position, rotation);
}
