#pragma once

#include <memory>
#include <optional>

#include "Geometry.h"
#include "Quaternion.h"
#include "Vec3.h"

enum Material {
    DIFFUSE = 0,
    METALLIC = 1,
    DIELECTRIC = 2
};

struct Primitive {
    std::unique_ptr<Geometry> geom;

    // TODO: refactor into Material class
    Material material = DIFFUSE;
    double ior;

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
