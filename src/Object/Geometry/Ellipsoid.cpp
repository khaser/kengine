#include "Primitives/Vec3.h"
#include "Object/Geometry.h"

Ellipsoid::Ellipsoid(const Vec3<double> &v) : r(v) {}

Vec3<double> Ellipsoid::normal(const Vec3<double>& p) const {
    return p.norm() / (r * r);
}

std::optional<double> Ellipsoid::get_intersect_(const Ray& ray) const {
    // start % start + 2 * t * (start % v) + t * t * (v % v) = R * R
    double a = (ray.v / r) % (ray.v / r);
    double b = ((ray.start / r) % (ray.v / r)) * 2;
    double c = (ray.start / r) % (ray.start / r) - 1;
    double D = b * b - 4 * a * c;
    if (D < 0) return {};
    double t1 = (-b - sqrt(D)) / (2 * a);
    double t2 = (-b + sqrt(D)) / (2 * a);

    if (t1 > 0) {
        return t1;
    } else if (t2 > 0) {
        return t2;
    } else return {};
};
