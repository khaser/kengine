#include "Primitives/Vec3.h"
#include "Object/Geometry.h"

Box::Box(const Vec3<double> &v) : size(v) {}

Vec3<double> Box::normal(const Vec3<double>& p) const {
    auto v = p / size;
    Vec3<double> av = {fabs(v.x), fabs(v.y), fabs(v.z)};
    if (av.x > av.y && av.x > av.z) {
        return {v.x, 0, 0};
    } else if (av.y > av.x && av.y > av.z) {
        return {0, v.y, 0};
    } else {
        return {0, 0, v.z};
    }
}

std::optional<double> Box::get_intersect_(const Ray& ray) const {
    Vec3<double> t1v = (size - ray.start) / ray.v;
    Vec3<double> t2v = (-size - ray.start) / ray.v;
    Vec3<double> tmin = min(t1v, t2v);
    Vec3<double> tmax = max(t1v, t2v);
    auto t1 = std::max({tmin.x, tmin.y, tmin.z});
    auto t2 = std::min({tmax.x, tmax.y, tmax.z});
    if (t1 > t2 || t2 < 0) {
        return {};
    } else if (t1 > 0) {
        return t1;
    } else if (t2 > 0) {
        return t2;
    } else return {};
};

