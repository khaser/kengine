#include "Primitives/Vec3.h"

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
    if (av.x > av.y && av.x > av.z) {
        return {1, 0, 0};
    } else if (av.y > av.x && av.y > av.z) {
        return {0, 1, 0};
    } else {
        return {0, 0, 1};
    }
}

std::vector<float> Box::get_intersect_(const Ray& ray) const {
    Vec3<float> t1v = (size - ray.start) / ray.v;
    Vec3<float> t2v = (-size - ray.start) / ray.v;
    Vec3<float> tmin = min(t1v, t2v);
    Vec3<float> tmax = max(t1v, t2v);
    auto t1 = std::max({tmin.x, tmin.y, tmin.z});
    auto t2 = std::min({tmax.x, tmax.y, tmax.z});
    if (t1 > t2) {
        return {};
    } else {
        std::vector<float> res;
        if (t1 > 0) res.push_back(t1);
        if (t2 > 0) res.push_back(t2);
        return res;
    }
};

Box Box::AABB() const {
    Vec3<float> Min = position;
    Vec3<float> Max = position;
    for (float i = -1; i <= 1; i += 2) {
        for (float j = -1; j <= 1; j += 2) {
            for (float k = -1; k <= 1; k += 2) {
                Vec3<float> pnt = position + rotation * (size * Vec3<float>{i, j, k});
                Min = min(Min, pnt);
                Max = max(Max, pnt);
            }
        }
    }
    return Box(Min, Max);
}

Vec3<float> Box::Min() const {
    return position - size;
}

Vec3<float> Box::Max() const {
    return position + size;
}

Box Box::operator|(const Geometry& oth) const {
    auto a = AABB();
    auto b = oth.AABB();
    return Box(min(a.Min(), b.Min()), max(a.Max(), b.Max()));
}

void Box::bump() {
    size = size + 1e-4;
}
