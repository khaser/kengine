#include "Primitives/Vec3.h"
#include "Rnd.h"

#include "Distribution.h"
#include <memory>

typedef Vec3<float> vec3;

MixedDistribution::MixedDistribution(std::vector<std::shared_ptr<LightDistribution>> &&dists_) {
    bvh = BVH_light::BVH(0, dists_.cbegin(), dists_.cend());
    for (auto &dist : dists_) {
        dists.push_back(std::static_pointer_cast<Distribution>(dist));
    }
    bvh_end = dists.end();
    dists.emplace_back(static_cast<Distribution*>(new CosineDistribution()));
}

MixedDistribution::~MixedDistribution() {};

vec3 MixedDistribution::sample(const vec3 &pos, const vec3 &n) {
    Rnd *rnd = Rnd::getRnd();
    return dists[rnd->uniform_int(0, dists.size())]->sample(pos, n);
}

float MixedDistribution::pdf(const vec3 &pos, const vec3 &n, const vec3 &d) const {
    float res = dists.back()->pdf(pos, n, d) + bvh.get_intersect({pos, d}, false);
    if (res == 0) throw std::logic_error("zero probability density by direction");

    return res / dists.size();
}
