#pragma once

#include <vector>
#include <optional>
#include <type_traits>

#include "Camera.h"
#include "Object.h"
#include "Primitives.h"
#include "BVH.h"

namespace BVH_bounds {
using T = Object;
using F = std::optional<std::pair<Object, Intersection>>;

std::optional<Intersection> best_inter(std::shared_ptr<Geometry> geom, const Ray &r);

struct Map {
    Map (const Ray &ray) : ray(ray) {};
    F operator() (const T& obj) const {
        auto res = best_inter(obj.geometry, ray);
        if (res) {
            return std::make_pair(obj, *res);
        } else {
            return std::nullopt;
        }
    }
    const Ray ray;
};

struct Merge {
    F operator() (const F &a, const F &b) const {
        if (!a.has_value()) return b;
        if (!b.has_value()) return a;
        return (a->second.t < b->second.t ? a : b);
    }
};

struct Geom {
    std::shared_ptr<Geometry> operator() (const T &a) const {
        return a.geometry;
    }
};

struct EarlyOut {
    bool operator() (const Ray& r, const F &res, const Node &node) const {
        // TODO: remove
        /* return false; */
        if (!res) return false;
        auto inter = best_inter(std::make_shared<Box>(node.aabb), r);
        if (!inter) return true;
        return res->second.t < inter->t;
    }
};

using BVH = RawBVH::BVH<T, F, Map, Merge, Geom, EarlyOut>;

}; // namespace BVH_bounds

struct Scene {
    std::pair<uint16_t, uint16_t> dimensions;
    Vec3<double> bg_color;
    Vec3<double> ambient_light;
    Camera camera;

    BVH_bounds::BVH bvh;
    std::vector<Object> non_bvh_objs;

    std::unique_ptr<MixedDistribution> light_pdf;

    int ray_depth;
    uint16_t samples;

    Scene(std::ifstream is);

    std::vector<std::vector<Vec3<double>>> render_scene();

private:
    // x, y in [-1, 1]
    Vec3<double> postprocess(Vec3<double> in_color);

    Vec3<double> aces_tonemap(const Vec3<double> &x);

    Vec3<double> gamma_correction(const Vec3<double> &x);

    Vec3<double> saturate(const Vec3<double> &color);

    Vec3<double> raycast(const Ray& ray, int ttl) const;

    std::optional<std::pair<Object, Intersection>> get_intersect(const Ray& ray) const;
};
