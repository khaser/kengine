#include "Primitives/Vec3.h"
#include "Primitives/Mat3.h"

#include "Object/Geometry.h"

#include <vector>
#include <iostream>
#include <utility>

Triangle::Triangle(const Mat3<double> &cords)
    : vert(cords), v(cords.y - cords.x), u(cords.z - cords.x) {
    shift = (cords.x + cords.y + cords.z) / 3;
}

Vec3<double> Triangle::normal(const Vec3<double>& p) const {
    return (v ^ u).norm();
}

std::vector<double> Triangle::get_intersect_(const Ray& ray) const {
    Mat3<double> mat;
    if ((u ^ v) % -ray.v > 0) {
        mat = {u, v, -ray.v};
    } else {
        mat = {v, u, -ray.v};
    }
    auto solution = mat.solve(ray.start - vert.x);
    if (solution.has_value()) {
        Vec3<double> inter = solution.value();
        if (inter.x >= 0 && inter.y >= 0 && inter.x + inter.y <= 1 && inter.z > 0) {
            return {inter.z};
        } else {
            return {};
        }
    } else {
        return {};
    }
};

Box Triangle::AABB() const {
    auto gvert = Mat3<double>(position) + rotation * vert;
    return Box(min(gvert.x, min(gvert.y, gvert.z)), max(max(gvert.x, gvert.y), gvert.z));
}
