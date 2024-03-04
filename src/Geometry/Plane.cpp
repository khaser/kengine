#include "Geometry.h"
#include "Vec3.h"

Plane::Plane(const Vec3<double> &v) : norm(v.norm()) {}

std::optional<double> Plane::get_intersect_(const Ray& ray) const {
    double t = -(ray.start % norm) / (ray.v % norm);
    return (t > 0 ? std::optional<double>(t) : std::nullopt);
};

Vec3<double> Plane::normal(const Vec3<double> &p) const {
    return norm;
}
