#include "Primitives/Vec3.h"
#include "Rnd.h"

#include "Distribution.h"

typedef Vec3<double> vec3;

CosineDistribution::CosineDistribution() : Distribution() {}

CosineDistribution::~CosineDistribution() {};

vec3 CosineDistribution::sample(const vec3 &pos, const vec3 &n) {
    vec3 res = rnd->in_sphere();
    res = res + n;
    return res.norm();
}

double CosineDistribution::pdf(const vec3 &pos, const vec3 &n, const vec3 &d) const {
    return std::max(0.0, d % n / M_PI);
}
