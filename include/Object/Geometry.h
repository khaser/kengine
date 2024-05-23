#pragma once

#include <cmath>
#include <algorithm>
#include <vector>

#include "Primitives/AABB.h"
#include "Primitives.h"

struct Box;

struct Geometry {
    Vec3<float> position;
    // used for correct BVH work on ill formatted scenes,
    // where all triangles defined in local coords
    Quaternion rotation;
    Geometry() {};
    Geometry(const Vec3<float> &pos, const Quaternion &r) : position(pos), rotation(r) {};
    virtual ~Geometry() {};

    Intersection get_intersect(Ray ray) const;
    virtual Vec3<float> normal(const Vec3<float>& p) const = 0;
    virtual AABB get_aabb() const = 0;
private:
    // get sorted vector of lengths on which ray intersect geometry
    // Use ray.reveal, to get intersection cords
    virtual float get_intersect_(const Ray&) const = 0;
};

struct Triangle : public Geometry {
    Mat3<float> vert;
    Vec3<float> u, v;
    Vec3<float> norm;
    Triangle(const Mat3<float>&);
    virtual ~Triangle() {};
    Vec3<float> normal(const Vec3<float>&) const;
    virtual AABB get_aabb() const;
private:
    float get_intersect_(const Ray&) const;
};
