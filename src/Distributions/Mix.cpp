#include "Primitives/Vec3.h"
#include "Rnd.h"

#include "Distribution.h"
#include <memory>

typedef Vec3<float> vec3;

MixedDistribution::MixedDistribution(std::vector<std::shared_ptr<LightDistribution>> &&dists_) : dists(std::move(dists_)) {
    bvh = BVH_light::BVH(0, dists.cbegin(), dists.cend());
}

MixedDistribution::~MixedDistribution() {};

vec3 MixedDistribution::sample(const vec3 &pos, const vec3 &n) {
    Rnd *rnd = Rnd::getRnd();
    if (rnd->bernoulli() && !dists.empty()) {
        return dists[rnd->uniform_int(0, dists.size())]->sample(pos, n);
    } else {
        return cosine.sample(pos, n);
    }
}

float MixedDistribution::pdf(const vec3 &pos, const vec3 &n, const vec3 &d) const {
    if (dists.empty()) return cosine.pdf(pos, n, d);
    return (bvh.get_intersect({pos, d}, false) / dists.size() + cosine.pdf(pos, n, d)) / 2;
}
