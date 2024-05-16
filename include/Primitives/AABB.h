#pragma once

#include "Primitives/Vec3.h"
#include "Primitives/Intersection.h"
#include "Primitives/Ray.h"

#include <optional>

// Aligned box
struct AABB {
    Vec3<float> Min, Max;
    void extend(Vec3<float> p) {
        Min = min(Min, p);
        Max = max(Max, p);
    }

    AABB operator | (AABB oth) const {
        oth.extend(*this);
        return oth;
    }

    void extend(AABB b) {
        extend(b.Min);
        extend(b.Max);
    }

    Vec3<float> size() const {
        return Max - Min;
    }

    Vec3<float> position() const {
        return (Max + Min) / 2;
    }

    std::optional<Intersection> get_intersect(const Ray& ray) const {
        Vec3<float> tx1 = (Min - ray.start) / ray.v;
        Vec3<float> tx2 = (Max - ray.start) / ray.v;
        float tmin = min(tx1, tx2).max(), tmax = max(tx1, tx2).min();

        if (tmax >= tmin && tmax > 0)
            // Dirty hack: normal is not used, so zero it
            return {{tmin, Vec3<float>{0}, false}};
        else
            return std::nullopt;
    }
};
