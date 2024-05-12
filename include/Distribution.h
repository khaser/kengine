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
    virtual Vec3<double> sample(const Vec3<double> &pos, const Vec3<double> &n) = 0;
    virtual double pdf(const Vec3<double> &pos, const Vec3<double> &n, const Vec3<double> &d) const = 0;
};

#include "BVH.h"

// Not used now
struct UniformDistribution : public Distribution {
    UniformDistribution();
    ~UniformDistribution();
    Vec3<double> sample(const Vec3<double> &pos, const Vec3<double> &n);
    double pdf(const Vec3<double> &pos, const Vec3<double> &n, const Vec3<double> &d) const;
};


struct CosineDistribution : public Distribution {
    CosineDistribution();
    ~CosineDistribution();
    Vec3<double> sample(const Vec3<double> &pos, const Vec3<double> &n);
    double pdf(const Vec3<double> &pos, const Vec3<double> &n, const Vec3<double> &d) const;
};


struct LightDistribution : public Distribution {
public:
    std::shared_ptr<Geometry> geometry;
    LightDistribution(std::shared_ptr<Geometry> geom) : geometry(geom), Distribution() {}
    ~LightDistribution() {}

    Vec3<double> sample(const Vec3<double> &pos, const Vec3<double>&) {
        Vec3<double> x = geometry->position + geometry->rotation * sample_();
        return (x - pos).norm();
    }

    // angle pdf
    double pdf(const Ray &r) const {
        double res = 0;
        for (Intersection &obj_inter : geometry->get_intersect(r)) {
            double tmp = pdf_(-geometry->rotation * (r.reveal(obj_inter.t) - geometry->position));
            if (tmp == 0) {
                throw std::logic_error("zero probability density by point");
            }
            res += tmp * obj_inter.t * obj_inter.t / abs(obj_inter.normal % r.v);
        }
        return res;
    }

    double pdf(const Vec3<double> &pos, const Vec3<double> &n, const Vec3<double> &d) const {
        return pdf({pos, d});
    }

private:
    // return sample from geom in local cords
    virtual Vec3<double> sample_() = 0;
    // return geometry point pdf
    virtual double pdf_(const Vec3<double> &pos) const = 0;
};


struct BoxDistribution : public LightDistribution {
    std::shared_ptr<Box> box;
    Vec3<double> faces;
    double faces_square;

    BoxDistribution(std::shared_ptr<Box> b);
    ~BoxDistribution();
    Vec3<double> sample_();
    double pdf_(const Vec3<double> &) const;
};


struct EllipsoidDistribution : public LightDistribution {
    std::shared_ptr<Ellipsoid> ellips;
    EllipsoidDistribution(std::shared_ptr<Ellipsoid> e);
    ~EllipsoidDistribution();
    Vec3<double> sample_();
    double pdf_(const Vec3<double> &pos) const;
};

struct TriangleDistribution : public LightDistribution {
    std::shared_ptr<Triangle> tr;
    TriangleDistribution(std::shared_ptr<Triangle> tr);
    ~TriangleDistribution();
    Vec3<double> sample_();
    double pdf_(const Vec3<double> &pos) const;
};

namespace BVH_light {
using T = std::shared_ptr<LightDistribution>;
struct Map {
    Map (const Ray &r) : r(r) {};
    double operator() (const T& dist) const {
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
    bool operator() (const Ray& r, const double &res, const Node *node) const {
        return false;
    }
};

struct Traverse {
    std::vector<Node*> operator() (const Ray& r, const Node* node) const {
        return {node->left, node->right};
    }
};

using BVH = RawBVH::BVH<T, double, Map, std::plus<double>, Geom, EarlyOut, Traverse>;

} // namespace BVH_light

struct MixedDistribution : public Distribution {
    BVH_light::BVH bvh;
    std::vector<std::shared_ptr<Distribution>> dists;
    std::vector<std::shared_ptr<Distribution>>::const_iterator bvh_end;

    MixedDistribution(std::vector<std::shared_ptr<LightDistribution>> &&dists);
    ~MixedDistribution();
    Vec3<double> sample(const Vec3<double> &pos, const Vec3<double> &n);
    double pdf(const Vec3<double> &pos, const Vec3<double> &n, const Vec3<double> &d) const;
};
