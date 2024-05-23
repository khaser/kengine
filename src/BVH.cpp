#include "Primitives/Ray.h"
#include "Object.h"

namespace RawBVH {

std::optional<Intersection> best_inter(std::shared_ptr<Geometry> geom, const Ray &r) {
    Intersection inter = geom->get_intersect(r);
    if (inter.t < 0) { return std::nullopt; }
    return inter;
}

}
