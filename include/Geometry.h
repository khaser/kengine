#pragma once

#include <optional>
#include <cmath>
#include <algorithm>

#include "Vec3.h"
#include "Ray.h"
#include "Intersection.h"

struct Geometry {
    std::optional<Intersection> get_intersect(const Ray&) const;
    bool is_inside(const Ray&, double) const;
    virtual std::optional<double> get_intersect_(const Ray& ray) const = 0;
    virtual Vec3<double> normal(const Vec3<double>& p) const = 0;
};

struct Plane : public Geometry {
    Vec3<double> norm;
    Plane(const Vec3<double>&);
    std::optional<double> get_intersect_(const Ray&) const;
    Vec3<double> normal(const Vec3<double>&) const;
};

struct Ellipsoid : public Geometry {
    Vec3<double> r;
    Ellipsoid(const Vec3<double>&);
    std::optional<double> get_intersect_(const Ray&) const;
    Vec3<double> normal(const Vec3<double>&) const;
};

struct Box : public Geometry {
    Vec3<double> size;
    Box(const Vec3<double>&);
    std::optional<double> get_intersect_(const Ray&) const;
    Vec3<double> normal(const Vec3<double>&) const;
};
