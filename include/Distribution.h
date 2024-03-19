#pragma once

#include "Primitives/Vec3.h"
#include "Object/Geometry.h"
#include "Rnd.h"

#include <math.h>
#include <algorithm>
#include <vector>
#include <memory>
#include <iostream>

typedef Vec3<double> vec3;

struct Distribution {
    Distribution() : rnd(Rnd::getRnd()) {}
    virtual ~Distribution() {}
    virtual vec3 sample(const vec3 &pos, const vec3 &n) = 0;
    virtual double pdf(const vec3 &pos, const vec3 &n, const vec3 &d) = 0;
    Rnd *rnd;
};


struct UniformDistribution : public Distribution {

    UniformDistribution() : Distribution() {}
    ~UniformDistribution() {};

    vec3 sample(const vec3 &pos, const vec3 &n) {
        vec3 res = rnd->in_sphere();
        if (res % n < 0) {
            res = -res;
        }
        return res;
    }

    double pdf(const vec3 &pos, const vec3 &n, const vec3 &d) {
        return (d % n > 0 ? 1 / (M_PI * 2) : 0);
    }

};


struct CosineDistribution : public Distribution {
    CosineDistribution() : Distribution() {}
    ~CosineDistribution() {};

    vec3 sample(const vec3 &pos, const vec3 &n) {
        vec3 res = rnd->in_sphere();
        res = res + n;
        return res.norm();
    }

    double pdf(const vec3 &pos, const vec3 &n, const vec3 &d) {
        return std::max(0.0, d % n / M_PI);
    }
};


template<typename T>
struct LightDistribution : public Distribution {
    std::shared_ptr<T> geom;
    LightDistribution(std::shared_ptr<T> geom) : geom(geom), Distribution() {}
    ~LightDistribution() {}

    vec3 sample(const vec3 &pos, const vec3&) {
        vec3 x = geom->position + geom->rotation * sample_();
        return (x - pos).norm();
    }

    // angle pdf
    double pdf(const vec3 &pos, const vec3 &n, const vec3 &d) {
        double res = 0;
        Ray r = {pos, d};
        for (auto &obj_inter : geom->get_intersect(r)) {
            res += pdf_(pos) * obj_inter.t * obj_inter.t / abs(obj_inter.normal % d);
        }
        return res / M_PI;
    }

private:
    // return sample from geom in local cords
    virtual vec3 sample_() = 0;
    // return geometry point pdf
    virtual double pdf_(const vec3 &pos) = 0;
};


struct BoxDistribution : public LightDistribution<Box> {

    vec3 faces;
    double faces_square;

    BoxDistribution(std::shared_ptr<Box> b) : LightDistribution(b) {
        auto &[x, y, z] = geom->size;
        faces = {y * z, x * z, x * y};
        faces = faces * 4;
        faces_square = 2 * (faces.x + faces.y + faces.z);
    }

    ~BoxDistribution() {};

    vec3 sample_() {
        Rnd* rnd = Rnd::getRnd();
        auto &[x, y, z] = geom->size;

        vec3 u = {rnd->uniform(-x, x), rnd->uniform(-y, y), rnd->uniform(-z, z)};
        double pick_dim = rnd->uniform(0.0, faces.x + faces.y + faces.z);
        double pick_front_back = rnd->bernoulli() ? 1 : -1;

        if (pick_dim < faces.x) {
            return {x * pick_front_back, u.y, u.z};
        } else if (pick_dim < faces.x + faces.y) {
            return {u.x, y * pick_front_back, u.z};
        } else {
            return {u.x, u.y, z * pick_front_back};
        }
    }

    double pdf_(const vec3 &) {
        return 1 / faces_square;
    }

};


struct EllipsoidDistribution : public LightDistribution<Ellipsoid> {

    EllipsoidDistribution(std::shared_ptr<Ellipsoid> b) : LightDistribution(b) { }

    ~EllipsoidDistribution() {};

    vec3 sample_() {
        return geom->r * rnd->in_sphere();
    }

    double pdf_(const vec3 &pos) {
        vec3 n = geom->normal(pos);
        vec3 n2 = n * n;
        vec3 r2 = geom->r * geom->r;
        double res = sqrt(n2.x * r2.y * r2.z + r2.x * n2.y * r2.z + r2.x * r2.y * n2.z);
        return 1.0 / (4 * res);
    }

};


struct MixedDistribution : public Distribution {
    std::vector<std::unique_ptr<Distribution>> dists;

    MixedDistribution(std::vector<std::unique_ptr<Distribution>> &&dists) : dists(std::move(dists)) {}
    ~MixedDistribution() {};

    vec3 sample(const vec3 &pos, const vec3 &n) {
        Rnd *rnd = Rnd::getRnd();
        return dists[rnd->uniform_int(0, dists.size())]->sample(pos, n);
    }

    double pdf(const vec3 &pos, const vec3 &n, const vec3 &d) {
        double res =
            std::accumulate(dists.begin(), dists.end(), 0.0, [&] (double acc, const std::unique_ptr<Distribution> &dist) {
                return acc + dist->pdf(pos, n, d);
            });
        if (res == 0) throw std::logic_error("zero probability density on sample");

        return res / dists.size();
    }

};
