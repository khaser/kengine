#pragma once

#include "Primitives/Vec3.h"
#include "Primitives/Ray.h"
#include "Primitives/Intersection.h"
#include "Object/Geometry.h"
#include "Rnd.h"

#include <math.h>
#include <algorithm>
#include <vector>
#include <memory>
#include <iostream>

struct Distribution {
    Rnd *rnd;

    Distribution() : rnd(Rnd::getRnd()) {}
    virtual ~Distribution() {}
    virtual Vec3<float> sample(const Vec3<float> &pos, const Vec3<float> &n) = 0;
    virtual float pdf(const Vec3<float> &pos, const Vec3<float> &n, const Vec3<float> &d) const = 0;
};

#include "BVH.h"

// Not used now
struct UniformDistribution : public Distribution {
    UniformDistribution();
    ~UniformDistribution();
    Vec3<float> sample(const Vec3<float> &pos, const Vec3<float> &n);
    float pdf(const Vec3<float> &pos, const Vec3<float> &n, const Vec3<float> &d) const;
};


struct CosineDistribution : public Distribution {
    CosineDistribution();
    ~CosineDistribution();
    Vec3<float> sample(const Vec3<float> &pos, const Vec3<float> &n);
    float pdf(const Vec3<float> &pos, const Vec3<float> &n, const Vec3<float> &d) const;
};


struct LightDistribution : public Distribution {
public:
    std::shared_ptr<Geometry> geometry;
    LightDistribution(std::shared_ptr<Geometry> geom) : geometry(geom), Distribution() {}
    ~LightDistribution() {}

    Vec3<float> sample(const Vec3<float> &pos, const Vec3<float> &n) {
        Vec3<float> x = geometry->position + geometry->rotation * sample_();
        return (x - pos).norm();
    }

    // angle pdf
    float pdf(const Ray &r) const {
        float res = 0;
        for (Intersection &obj_inter : geometry->get_intersect(r)) {
            float tmp = pdf_(-geometry->rotation * (r.reveal(obj_inter.t) - geometry->position));
            if (tmp <= 1e-5) {
                throw std::logic_error("zero probability density by point");
            }
            float angle_k = abs(obj_inter.normal % r.v);
            if (angle_k <= 1e-4) {
                angle_k = 1e-4;
            }
            res += tmp * obj_inter.t * obj_inter.t / angle_k;
        }
        return res;
    }

    float pdf(const Vec3<float> &pos, const Vec3<float> &n, const Vec3<float> &d) const {
        return pdf({pos, d});
    }

private:
    // return sample from geom in local cords
    virtual Vec3<float> sample_() = 0;
    // return geometry point pdf
    virtual float pdf_(const Vec3<float> &pos) const = 0;
};


struct BoxDistribution : public LightDistribution {
    std::shared_ptr<Box> box;
    Vec3<float> faces;
    float faces_square;

    BoxDistribution(std::shared_ptr<Box> b);
    ~BoxDistribution();
    Vec3<float> sample_();
    float pdf_(const Vec3<float> &) const;
};


struct EllipsoidDistribution : public LightDistribution {
    std::shared_ptr<Ellipsoid> ellips;
    EllipsoidDistribution(std::shared_ptr<Ellipsoid> e);
    ~EllipsoidDistribution();
    Vec3<float> sample_();
    float pdf_(const Vec3<float> &pos) const;
};

struct TriangleDistribution : public LightDistribution {
    std::shared_ptr<Triangle> tr;
    TriangleDistribution(std::shared_ptr<Triangle> tr);
    ~TriangleDistribution();
    Vec3<float> sample_();
    float pdf_(const Vec3<float> &pos) const;
};

namespace BVH_light {
using T = std::shared_ptr<LightDistribution>;
struct Map {
    Map (const Ray &r) : r(r) {};
    float operator() (const T& dist) const {
        return dist->pdf(r);
    }
    const Ray r;
};

struct Geom {
    std::shared_ptr<Geometry> operator() (const T &a) const {
        return a->geometry;
    }
};

struct EarlyOut {
    bool operator() (const float &res, float inter_t) const {
        return false;
    }
};

using BVH = RawBVH::BVH<T, float, Map, std::plus<float>, Geom, EarlyOut>;

} // namespace BVH_light

struct MixedDistribution : public Distribution {
    BVH_light::BVH bvh;
    std::vector<std::shared_ptr<LightDistribution>> dists;
    CosineDistribution cosine;

    MixedDistribution(std::vector<std::shared_ptr<LightDistribution>> &&dists);
    ~MixedDistribution();
    Vec3<float> sample(const Vec3<float> &pos, const Vec3<float> &n);
    float pdf(const Vec3<float> &pos, const Vec3<float> &n, const Vec3<float> &d) const;
};
