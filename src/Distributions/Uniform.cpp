#include "Primitives/Vec3.h"
#include "Rnd.h"

#include "Distribution.h"

typedef Vec3<float> vec3;

UniformDistribution::UniformDistribution() : Distribution() {}

UniformDistribution::~UniformDistribution() {};

vec3 UniformDistribution::sample(const vec3 &pos, const vec3 &n) const {
    vec3 res = rnd->in_sphere();
    if (res % n < 0) {
        res = -res;
    }
    return res;
}

float UniformDistribution::pdf(const vec3 &pos, const vec3 &n, const vec3 &d) const {
    return (d % n > 0 ? 1 / (M_PI * 2) : 0);
}
