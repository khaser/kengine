#include "Primitives/Vec3.h"
#include "Primitives/AABB.h"

#include "Object/Geometry.h"

#include <vector>
#include <iostream>

Box::Box(const Vec3<float> &v) : size(v), div_size(Vec3<float>(1) / size) {}

Box::Box(const Vec3<float> &size, const Vec3<float> &position, const Quaternion &rotation) :
    Geometry(position, rotation),
    size(size),
    div_size(Vec3<float>(1) / size) {}
Box::Box(const Vec3<float> &aa, const Vec3<float> &bb) :
    Geometry((bb + aa) / 2, {}),
    size((bb - aa) / 2),
    div_size(Vec3<float>(1) / size) {}

Vec3<float> Box::normal(const Vec3<float>& p) const {
    Vec3<float> v = p * div_size;
    Vec3<float> av = {fabsf(v.x), fabsf(v.y), fabsf(v.z)};
    return av.maj() * v;
}

std::vector<float> Box::get_intersect_(const Ray& ray) const {
    Vec3<float> t1v = (size - ray.start) / ray.v;
    Vec3<float> t2v = (-size - ray.start) / ray.v;
    float t1 = min(t1v, t2v).max();
    float t2 = max(t1v, t2v).min();
    if (t1 > t2) {
        return {};
    } else {
        std::vector<float> res;
        if (t1 > 0) res.push_back(t1);
        if (t2 > 0) res.push_back(t2);
        return res;
    }
};

struct AABB Box::get_aabb() const {
    struct AABB res;
    for (float i = -1; i <= 1; i += 2) {
        for (float j = -1; j <= 1; j += 2) {
            for (float k = -1; k <= 1; k += 2) {
                res.extend(position + rotation * (size * Vec3<float>{i, j, k}));
            }
        }
    }
    return res;
}
