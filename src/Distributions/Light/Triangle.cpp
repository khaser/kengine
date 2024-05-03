#include "Primitives/Vec3.h"
#include "Rnd.h"

#include "Distribution.h"

typedef Vec3<double> vec3;

TriangleDistribution::TriangleDistribution(std::shared_ptr<Triangle> geom)
    : LightDistribution(geom) {};

TriangleDistribution::~TriangleDistribution() {};

vec3 TriangleDistribution::sample_() {
    auto rnd = Rnd::getRnd();
    auto [x, y] = std::make_tuple(rnd->uniform(0, 1), rnd->uniform(0, 1));
    if (x + y > 1) {
        x = 1 - x;
        y = 1 - y;
    }
    vec3 bari = vec3 {1 - x - y, x, y};
    return geom->vert * bari;
}

double TriangleDistribution::pdf_(const Vec3<double>&) {
    double square = 0.5 * (geom->u ^ geom->v).len();
    return 1 / square;
}
