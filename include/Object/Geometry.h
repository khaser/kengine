#pragma once

#include <cmath>
#include <algorithm>
#include <vector>

#include "Primitives.h"

struct Box;

struct Geometry {
    Vec3<float> position;
    // used for correct BVH work on ill formatted scenes,
    // where all triangles defined in local coords
    Vec3<float> shift;
    Quaternion rotation;
    Geometry() {};
    Geometry(const Vec3<float> &pos, const Quaternion &r) : position(pos), rotation(r) {};
    virtual ~Geometry() {};

    Vec3<float> mid() const { return position + shift; };
    std::vector<Intersection> get_intersect(Ray ray) const;
    virtual Vec3<float> normal(const Vec3<float>& p) const = 0;
    virtual Box AABB() const = 0;
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
    virtual Box AABB() const;
private:
    std::vector<float> get_intersect_(const Ray&) const;
};

struct Ellipsoid : public Geometry {
    Vec3<float> r;
    Ellipsoid(const Vec3<float>&);
    virtual ~Ellipsoid() {};
    Vec3<float> normal(const Vec3<float>&) const;
    virtual Box AABB() const;
private:
    std::vector<float> get_intersect_(const Ray&) const;
};

struct Box : public Geometry {
    Vec3<float> size;
    Vec3<float> div_size;
    Box(const Vec3<float>&);
    Box(const Vec3<float> &size, const Vec3<float> &position, const Quaternion &rotation);
    Box(const Vec3<float> &aa, const Vec3<float> &bb);
    Vec3<float> Min() const;
    Vec3<float> Max() const;
    void bump();
    virtual ~Box() {};
    Vec3<float> normal(const Vec3<float>&) const;
    Box operator|(const Geometry& oth) const;
    virtual Box AABB() const;
private:
    std::vector<float> get_intersect_(const Ray&) const;
};

struct Triangle : public Geometry {
    Mat3<float> vert;
    Vec3<float> u, v;
    Triangle(const Mat3<float>&);
    virtual ~Triangle() {};
    Vec3<float> normal(const Vec3<float>&) const;
    virtual Box AABB() const;
private:
    std::vector<float> get_intersect_(const Ray&) const;
};
