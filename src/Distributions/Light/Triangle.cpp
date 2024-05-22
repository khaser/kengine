#include "Primitives/Vec3.h"
#include "Rnd.h"

#include "Distribution.h"

typedef Vec3<float> vec3;

TriangleDistribution::TriangleDistribution(std::shared_ptr<Triangle> geom)
    : LightDistribution(geom), tr(geom) {};

TriangleDistribution::~TriangleDistribution() {};

vec3 TriangleDistribution::sample_() const {
    auto rnd = Rnd::getRnd();
    auto [x, y] = std::make_tuple(rnd->uniform(0, 1), rnd->uniform(0, 1));
    if (x + y > 1) {
        x = 1 - x;
        y = 1 - y;
    }
    vec3 bari = vec3 {1 - x - y, x, y};
    return tr->vert * bari;
}

float TriangleDistribution::pdf_(const Vec3<float>&) const {
    float square = 0.5 * (tr->u ^ tr->v).len();
    return 1 / square;
}
