#include "Geometry.h"
#include "Quaternion.h"
#include "Vec3.h"
#include "Intersection.h"

std::optional<Intersection> Geometry::get_intersect(Ray ray) const {
    ray.start = -rotation * (ray.start - position);
    ray.v = -rotation * ray.v;
    std::optional<double> intersect = get_intersect_(ray);
    if (intersect) {
        double t = intersect.value();
        Vec3<double> n = normal(ray.reveal(intersect.value()));
        bool is_ins = is_inside(ray, t);
        if (is_ins) {
            n = -n;
        }
        n = (rotation * n).norm();
        return {{t, n, is_ins}};
    } else {
        return std::nullopt;
    }
}

bool Geometry::is_inside(const Ray &ray, double t) const {
    return ray.v % normal(ray.reveal(t)) >= 0;
}

