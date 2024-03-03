#pragma once

#include <memory>
#include <optional>

#include "Geometry.h"
#include "Quaternion.h"
#include "Vec3.h"

struct Primitive {
    std::unique_ptr<Geometry> geom;
    Quaternion rotation;
    Vec3<double> position;
    Vec3<double> color;

    std::optional<double> get_intersect(Ray ray) const {
        ray.start = -rotation * (ray.start - position);
        ray.v = -rotation * ray.v;
        std::optional<double> t = geom->get_intersect(ray);
        return t;
    }
};

