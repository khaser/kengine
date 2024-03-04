#pragma once

#include <memory>
#include <optional>

#include "Geometry.h"
#include "Quaternion.h"
#include "Intersection.h"
#include "Vec3.h"

enum Material {
    DIFFUSE = 0,
    METALLIC = 1,
    DIELECTRIC = 2
};

struct Primitive {
    std::shared_ptr<Geometry> geom;

    // TODO: refactor into Material class
    Material material = DIFFUSE;
    double ior;

    Quaternion rotation;
    Vec3<double> position;
    Vec3<double> color;

    std::optional<Intersection> get_intersect(Ray ray) const {
        ray.start = -rotation * (ray.start - position);
        ray.v = -rotation * ray.v;
        auto intersect = geom->get_intersect(ray);
        if (intersect) {
            auto [t, normal, is_inside] = intersect.value();
            if (is_inside) {
                normal = -normal;
            }
            normal = (rotation * normal).norm();
            return {{t, normal, is_inside}};
        } else {
            return std::nullopt;
        }
    }
};

