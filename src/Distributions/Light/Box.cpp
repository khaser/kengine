#include "Primitives/Vec3.h"
#include "Rnd.h"

#include "Distribution.h"
#include <memory>

typedef Vec3<double> vec3;

BoxDistribution::BoxDistribution(std::shared_ptr<Box> b) : LightDistribution(b), box(b) {
    auto &[x, y, z] = box->size;
    faces = {y * z, x * z, x * y};
    faces = faces * 4;
    faces_square = 2 * (faces.x + faces.y + faces.z);
}

BoxDistribution::~BoxDistribution() {};

vec3 BoxDistribution::sample_() {
    Rnd* rnd = Rnd::getRnd();
    auto &[x, y, z] = box->size;

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

double BoxDistribution::pdf_(const vec3 &) const {
    return 1 / faces_square;
}
