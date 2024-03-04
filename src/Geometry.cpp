#include "Vec3.h"
#include "Geometry.h"
#include "Intersection.h"

std::optional<Intersection> Geometry::get_intersect(const Ray& ray) const {
    auto t = get_intersect_(ray);
    if (t.has_value()) {
        return {{t.value(), normal(ray.reveal(t.value())), is_inside(ray, t.value())}};
    } else {
        return std::nullopt;
    }
}


bool Geometry::is_inside(const Ray &ray, double t) const {
    return ray.v % normal(ray.reveal(t)) > 0;
}

