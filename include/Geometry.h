#pragma once

#include <optional>
#include <cmath>
#include <algorithm>

#include "Vec3.h"
#include "Ray.h"

struct Geometry {
    virtual std::optional<double> get_intersect(const Ray& ray) = 0;
};

struct Plane : public Geometry {
    Vec3<double> norm;
    Plane(const Vec3<double> &v) : norm(v.norm()) {}
    std::optional<double> get_intersect(const Ray& ray) {
        double t = -(ray.start % norm) / (ray.v % norm);
        return (t > 0 ? std::optional<double>(t) : std::nullopt);
    };
};

struct Ellipsoid : public Geometry {
    Vec3<double> r;
    Ellipsoid(const Vec3<double> &v) : r(v) {}
    std::optional<double> get_intersect(const Ray& ray) {
        // start % start + 2 * t * (start % v) + t * t * (v % v) = R * R
        double a = (ray.v / r) % (ray.v / r);
        double b = ((ray.start / r) % (ray.v / r)) * 2;
        double c = (ray.start / r) % (ray.start / r) - 1;
        double D = b * b - 4 * a * c;
        if (D < 0) return {};
        double t1 = (-b - sqrt(D)) / (2 * a);
        double t2 = (-b + sqrt(D)) / (2 * a);
        if (t1 > 0) {
            return t1;
        } else if (t2 > 0) {
            return t2;
        } else return {};
    };
};

struct Box : public Geometry {
    Vec3<double> size;
    Box(const Vec3<double> &v) : size(v) {}
    std::optional<double> get_intersect(const Ray& ray) {
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
};
