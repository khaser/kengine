#include "Primitives/Vec3.h"
#include "Rnd.h"

#include "Distribution.h"

typedef Vec3<double> vec3;

MixedDistribution::MixedDistribution(std::vector<std::unique_ptr<Distribution>> &&dists) : dists(std::move(dists)) {}
MixedDistribution::~MixedDistribution() {};

vec3 MixedDistribution::sample(const vec3 &pos, const vec3 &n) {
    Rnd *rnd = Rnd::getRnd();
    return dists[rnd->uniform_int(0, dists.size())]->sample(pos, n);
}

double MixedDistribution::pdf(const vec3 &pos, const vec3 &n, const vec3 &d) {
    double res =
        std::accumulate(dists.begin(), dists.end(), 0.0, [&] (double acc, const std::unique_ptr<Distribution> &dist) {
            return acc + dist->pdf(pos, n, d);
        });
    if (res == 0) throw std::logic_error("zero probability density on sample");

    return res / dists.size();
}
