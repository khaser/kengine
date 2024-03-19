#pragma once

#include <optional>
#include <cmath>
#include <algorithm>

#include "Primitives.h"

struct Geometry {
    Quaternion rotation;
    Vec3<double> position;

    virtual ~Geometry() {};

    std::optional<Intersection> get_intersect(Ray ray) const;
    virtual Vec3<double> normal(const Vec3<double>& p) const = 0;
private:
    virtual std::optional<double> get_intersect_(const Ray&) const = 0;
    bool is_inside(const Ray&, double) const;
};

struct Plane : public Geometry {
    Vec3<double> norm;
    Plane(const Vec3<double>&);
    virtual ~Plane() {};
    std::optional<double> get_intersect_(const Ray&) const;
    Vec3<double> normal(const Vec3<double>&) const;
};

struct Ellipsoid : public Geometry {
    Vec3<double> r;
    Ellipsoid(const Vec3<double>&);
    virtual ~Ellipsoid() {};
    std::optional<double> get_intersect_(const Ray&) const;
    // TODO should implement
    // std::optional<std::pair<double, double>> get_intersect2(const Ray&) const;
    Vec3<double> normal(const Vec3<double>&) const;
    Vec3<double> gen_sample() const;
};

struct Box : public Geometry {
    Vec3<double> size;
    Box(const Vec3<double>&);
    virtual ~Box() {};
    std::optional<double> get_intersect_(const Ray&) const;
    Vec3<double> normal(const Vec3<double>&) const;
    Vec3<double> gen_sample() const;
};
