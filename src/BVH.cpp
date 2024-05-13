#include "Primitives/Ray.h"
#include "Object.h"

namespace RawBVH {

std::optional<Intersection> best_inter(std::shared_ptr<Geometry> geom, const Ray &r) {
    std::vector<Intersection> inters = geom->get_intersect(r);
    auto it = std::min_element(inters.begin(), inters.end(),
        [] (const auto &a, const auto &b) {
            return a.t < b.t;
        });
    if (it == inters.end()) {
        return std::nullopt;
    } else {
        return *it;
    }
}

}
