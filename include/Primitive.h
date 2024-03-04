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
    std::unique_ptr<Geometry> geom;

    // TODO: refactor into Material class
    Material material = DIFFUSE;
    double ior;

    Quaternion rotation;
    Vec3<double> position;
    Vec3<double> color;

    std::optional<Intersection> get_intersect(Ray ray) const {
        ray.start = -rotation * (ray.start - position);
        ray.v = -rotation * ray.v;
        return geom->get_intersect(ray);
        /* if (t) { */
        /*     return std::optional<Intersection>(Intersection{t.value(), {0, 0, 0}, false}); */
        /* } else { */
        /*     return {}; */
        /* } */
    }
};

