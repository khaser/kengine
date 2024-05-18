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

    std::vector<Intersection> get_intersect(Ray ray) const;
    virtual Vec3<float> normal(const Vec3<float>& p) const = 0;
    virtual AABB get_aabb() const = 0;
private:
    // get sorted vector of lengths on which ray intersect geometry
    // Use ray.reveal, to get intersection cords
    virtual std::vector<float> get_intersect_(const Ray&) const = 0;
    bool is_inside(const Ray&, float) const;
};

struct Plane : public Geometry {
    Vec3<float> norm;
    Plane(const Vec3<float>&);
    virtual ~Plane() {};
    Vec3<float> normal(const Vec3<float>&) const;
    virtual AABB get_aabb() const;
private:
    std::vector<float> get_intersect_(const Ray&) const;
};

struct Ellipsoid : public Geometry {
    Vec3<float> r;
    Vec3<float> div_r2_cached;
    Ellipsoid(const Vec3<float>&);
    virtual ~Ellipsoid() {};
    Vec3<float> normal(const Vec3<float>&) const;
    virtual AABB get_aabb() const;
private:
    std::vector<float> get_intersect_(const Ray&) const;
};

struct Box : public Geometry {
    Vec3<float> size;
    Vec3<float> div_size;
    Box(const Vec3<float>&);
    Box(const Vec3<float> &size, const Vec3<float> &position, const Quaternion &rotation);
    Box(const Vec3<float> &aa, const Vec3<float> &bb);
    virtual AABB get_aabb() const;
    virtual ~Box() {};
    Vec3<float> normal(const Vec3<float>&) const;
private:
    std::vector<float> get_intersect_(const Ray&) const;
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
    std::vector<float> get_intersect_(const Ray&) const;
};
