#include "Primitives/Vec3.h"

#include "Object/Geometry.h"

#include <vector>

Plane::Plane(const Vec3<double> &v) : norm(v.norm()) {}

Vec3<double> Plane::normal(const Vec3<double> &p) const {
    return norm;
}

std::vector<double> Plane::get_intersect_(const Ray& ray) const {
    double t = -(ray.start % norm) / (ray.v % norm);
    std::vector<double> res;
    if (t > 0) res.push_back(t);
    return res;
};

