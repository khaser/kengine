#pragma once

#include "Primitives/Vec3.h"
#include "Rnd.h"
#include <math.h>
#include <algorithm>

typedef Vec3<double> vec3;

// Distribution on hemisphere class
struct Distribution {
    Distribution(vec3 x, vec3 n) : x(x), n(n), rnd(Rnd::getRnd()) {}
    virtual vec3 sample() = 0;
    virtual vec3 pdf(vec3 d) = 0;
    vec3 x;
    vec3 n;
    Rnd *rnd;

private:
};

struct Uniform : public Distribution {

    Uniform(vec3 x, vec3 n) : Distribution(x, n) {}

    vec3 sample() {
        Vec3<double> res = rnd->in_sphere();
        if (res % n < 0) {
            res = -res;
        }
        return res;
    }

    vec3 pdf(vec3 d) {
        return (d % n > 0 ? 1 / (M_PI * 2) : 0);
    }

};

struct Cosine : public Distribution {
    Cosine(vec3 x, vec3 n) : Distribution(x, n) {}

    vec3 sample() {
        Vec3<double> res = rnd->in_sphere();
        res = res + n;
        return res.norm();
    }

    vec3 pdf(vec3 d) {
        return std::max(0.0, d % n / M_PI);
    }
};
