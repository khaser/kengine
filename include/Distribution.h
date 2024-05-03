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
    virtual double pdf(const Vec3<double> &pos, const Vec3<double> &n, const Vec3<double> &d) = 0;
};


struct UniformDistribution : public Distribution {
    UniformDistribution();
    ~UniformDistribution();
    Vec3<double> sample(const Vec3<double> &pos, const Vec3<double> &n);
    double pdf(const Vec3<double> &pos, const Vec3<double> &n, const Vec3<double> &d);
};


struct CosineDistribution : public Distribution {
    CosineDistribution();
    ~CosineDistribution();
    Vec3<double> sample(const Vec3<double> &pos, const Vec3<double> &n);
    double pdf(const Vec3<double> &pos, const Vec3<double> &n, const Vec3<double> &d);
};


template<typename T>
struct LightDistribution : public Distribution {
    std::shared_ptr<T> geom;
    LightDistribution(std::shared_ptr<T> geom) : geom(geom), Distribution() {}
    ~LightDistribution() {}

    Vec3<double> sample(const Vec3<double> &pos, const Vec3<double>&) {
        Vec3<double> x = geom->position + geom->rotation * sample_();
        return (x - pos).norm();
    }

    // angle pdf
    double pdf(const Vec3<double> &pos, const Vec3<double> &n, const Vec3<double> &d) {
        double res = 0;
        Ray r = {pos, d};
        for (Intersection &obj_inter : geom->get_intersect(r)) {
            double tmp = pdf_(-geom->rotation * (r.reveal(obj_inter.t) - geom->position));
            if (tmp == 0) {
                throw std::logic_error("zero probability density by point");
            }
            res += tmp * obj_inter.t * obj_inter.t / abs(obj_inter.normal % d);
        }
        return res;
    }

private:
    // return sample from geom in local cords
    virtual Vec3<double> sample_() = 0;
    // return geometry point pdf
    virtual double pdf_(const Vec3<double> &pos) = 0;
};


struct BoxDistribution : public LightDistribution<Box> {
    Vec3<double> faces;
    double faces_square;

    BoxDistribution(std::shared_ptr<Box> b);
    ~BoxDistribution();
    Vec3<double> sample_();
    double pdf_(const Vec3<double> &);
};


struct EllipsoidDistribution : public LightDistribution<Ellipsoid> {
    EllipsoidDistribution(std::shared_ptr<Ellipsoid> b);
    ~EllipsoidDistribution();
    Vec3<double> sample_();
    double pdf_(const Vec3<double> &pos);
};

struct TriangleDistribution : public LightDistribution<Triangle> {
    TriangleDistribution(std::shared_ptr<Triangle> b);
    ~TriangleDistribution();
    Vec3<double> sample_();
    double pdf_(const Vec3<double> &pos);
};

struct MixedDistribution : public Distribution {
    std::vector<std::unique_ptr<Distribution>> dists;

    MixedDistribution(std::vector<std::unique_ptr<Distribution>> &&dists);
    ~MixedDistribution();
    Vec3<double> sample(const Vec3<double> &pos, const Vec3<double> &n);
    double pdf(const Vec3<double> &pos, const Vec3<double> &n, const Vec3<double> &d);
};
