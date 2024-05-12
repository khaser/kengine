#include "Primitives/Vec3.h"
#include "Rnd.h"

#include "Distribution.h"

typedef Vec3<double> vec3;

EllipsoidDistribution::EllipsoidDistribution(std::shared_ptr<Ellipsoid> e) : LightDistribution(e), ellips(e) { }

EllipsoidDistribution::~EllipsoidDistribution() {};

vec3 EllipsoidDistribution::sample_() {
    return ellips->r * rnd->in_sphere();
}

double EllipsoidDistribution::pdf_(const vec3 &pos) const {
    vec3 n = (pos / ellips->r).norm();
    vec3 n2 = n * n;
    vec3 r2 = ellips->r * ellips->r;
    double res = sqrt(n2.x * r2.y * r2.z + r2.x * n2.y * r2.z + r2.x * r2.y * n2.z);
    return 1.0 / (4 * M_PI * res);
}

