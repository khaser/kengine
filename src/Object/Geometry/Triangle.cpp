#include "Primitives/Vec3.h"
#include "Primitives/Mat3.h"
#include "Primitives/AABB.h"

#include "Object/Geometry.h"

#include <vector>
#include <iostream>
#include <utility>

Triangle:: Triangle(const Mat3<float> &cords)
    : vert(cords), v(cords.y - cords.x), u(cords.z - cords.x), norm((v ^ u).norm()) {
}

Vec3<float> Triangle::normal(const Vec3<float>& p) const {
    return norm;
}

float Triangle::get_intersect_(const Ray& ray) const {
    Mat3<float> mat;
    if ((u ^ v) % -ray.v > 0) {
        mat = {u, v, -ray.v};
    } else {
        mat = {v, u, -ray.v};
    }
    auto solution = mat.solve(ray.start - vert.x);
    if (solution.has_value()) {
        Vec3<float> inter = solution.value();
        if (inter.x >= 0 && inter.y >= 0 && inter.x + inter.y <= 1 && inter.z > 0) {
            return inter.z;
        } else {
            return -1e30;
        }
    } else {
        return -1e30;
    }
};

AABB Triangle::get_aabb() const {
    auto gvert = Mat3<float>(position) + rotation * vert;
    AABB res;
    res.extend(gvert.x);
    res.extend(gvert.y);
    res.extend(gvert.z);
    return res;
}
