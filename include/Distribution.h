#pragma once

#include "Primitives/Vec3.h"
#include "Object/Geometry.h"
#include "Rnd.h"
#include <math.h>
#include <algorithm>
#include <vector>

typedef Vec3<double> vec3;

// Distribution on hemisphere class
struct Distribution {
    Distribution() : rnd(Rnd::getRnd()) {}
    virtual vec3 sample(const vec3 &pos, const vec3 &n) = 0;
    virtual double pdf(const vec3 &pos, const vec3 &n, const vec3 &d) = 0;
    Rnd *rnd;
};

struct UniformDistribution : public Distribution {

    UniformDistribution() : Distribution() {}

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

    vec3 sample(const vec3 &pos, const vec3 &n) {
        vec3 res = rnd->in_sphere();
        res = res + n;
        return res.norm();
    }

    double pdf(const vec3 &pos, const vec3 &n, const vec3 &d) {
        return std::max(0.0, d % n / M_PI);
    }
};


struct BoxDistribution : public Distribution {

    Box box;
    vec3 faces;
    double faces_square;

    BoxDistribution(const Box &b) : Distribution(), box(b) {
        auto &[x, y, z] = box.size;
        faces = {y * z, x * z, x * y};
        faces = faces * 4;
        faces_square = faces.x + faces.y + faces.z;
    }

    vec3 box_sample(const vec3 &pos, const vec3 &n) {
        Rnd* rnd = Rnd::getRnd();
        auto &[x, y, z] = box.size;

        Vec3<double> u = {rnd->uniform(-x, x), rnd->uniform(-y, y), rnd->uniform(-z, z)};
        double pick_dim = rnd->uniform(0.0, faces_square);
        double pick_front_back = rnd->bernoulli() ? 1 : -1;

        if (pick_dim < faces.x) {
            return {x * pick_front_back, u.y, u.z};
        } else if (pick_dim < faces.x + faces.y) {
            return {u.x, y * pick_front_back, u.z};
        } else {
            return {u.x, u.y, z * pick_front_back};
        }
    }

    vec3 sample(const vec3 &pos, const vec3 &n) {
        return (box_sample(pos, n) - pos).norm();
    }

    double pdf(const vec3 &pos, const vec3 &n, const vec3 &d) {
        Ray r = {pos, d};
        return (box.get_intersect(r) ? 1 / faces_square : 0);
    }

};


struct MixedDistribution : public Distribution {
    std::vector<Distribution*> dists;

    MixedDistribution(const std::vector<Distribution*> &dists) : dists(dists) {}

    vec3 sample(const vec3 &pos, const vec3 &n) {
        Rnd *rnd = Rnd::getRnd();
        return dists[rnd->uniform_int(0, dists.size())]->sample(pos, n);
    }


    double pdf(const vec3 &pos, const vec3 &n, const vec3 &d) {
        return
            std::accumulate(dists.begin(), dists.end(), 0.0, [&] (double acc, Distribution* dist) {
                return acc + dist->pdf(pos, n, d);
            }) / dists.size();
    }

};
