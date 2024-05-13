#include "Primitives/Vec3.h"

#include "Object/Geometry.h"

#include <vector>

Ellipsoid::Ellipsoid(const Vec3<float> &v) : r(v) {}

Vec3<float> Ellipsoid::normal(const Vec3<float>& p) const {
    return p.norm() / (r * r);
}

std::vector<float> Ellipsoid::get_intersect_(const Ray& ray) const {
    // start % start + 2 * t * (start % v) + t * t * (v % v) = R * R
    float a = (ray.v / r) % (ray.v / r);
    float b = ((ray.start / r) % (ray.v / r)) * 2;
    float c = (ray.start / r) % (ray.start / r) - 1;
    float D = b * b - 4 * a * c;
    if (D < 0) return {};
    float t1 = (-b - sqrt(D)) / (2 * a);
    float t2 = (-b + sqrt(D)) / (2 * a);

    std::vector<float> res;

    if (t1 > 0) res.push_back(t1);
    if (t2 > 0) res.push_back(t2);
    return res;
};

Box Ellipsoid::AABB() const {
    return Box(r, position, rotation);
}
