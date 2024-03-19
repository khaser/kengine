#pragma once

#include <cmath>
#include <algorithm>
#include <vector>

#include "Primitives.h"

struct Geometry {
    Quaternion rotation;
    Vec3<double> position;
    virtual ~Geometry() {};

    std::vector<Intersection> get_intersect(Ray ray) const;
    virtual Vec3<double> normal(const Vec3<double>& p) const = 0;
private:
    // get sorted vector of lengths on which ray intersect geometry
    // Use ray.reveal, to get intersection cords
    virtual std::vector<double> get_intersect_(const Ray&) const = 0;
    bool is_inside(const Ray&, double) const;
};

struct Plane : public Geometry {
    Vec3<double> norm;
    Plane(const Vec3<double>&);
    virtual ~Plane() {};
    std::vector<double> get_intersect_(const Ray&) const;
    Vec3<double> normal(const Vec3<double>&) const;
};

struct Ellipsoid : public Geometry {
    Vec3<double> r;
    Ellipsoid(const Vec3<double>&);
    virtual ~Ellipsoid() {};
    std::vector<double> get_intersect_(const Ray&) const;
    Vec3<double> normal(const Vec3<double>&) const;
};

struct Box : public Geometry {
    Vec3<double> size;
    Box(const Vec3<double>&);
    virtual ~Box() {};
    std::vector<double> get_intersect_(const Ray&) const;
    Vec3<double> normal(const Vec3<double>&) const;
};
