#pragma once

#include <vector>
#include <optional>
#include <type_traits>

#include "Camera.h"
#include "Object.h"
#include "Primitives.h"
#include "BVH.h"

namespace {
using T = Object;
using F = std::optional<std::pair<Object, Intersection>>;

struct Map {
    Map (const Ray &ray) : ray(ray) {};
    F operator() (const T& obj) const {
        std::vector<Intersection> inters = obj.geometry->get_intersect(ray);
        auto it = std::min_element(inters.begin(), inters.end(),
            [] (const auto &a, const auto &b) {
                return a.t < b.t;
            });
        if (it == inters.end()) {
            return std::nullopt;
        } else {
            return std::make_pair(obj, *it);
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

using BVH_bounds = BVH<T, F, Map, Merge>;

};

struct Scene {
    std::pair<uint16_t, uint16_t> dimensions;
    Vec3<double> bg_color;
    Vec3<double> ambient_light;
    Camera camera;
    std::unique_ptr<BVH_bounds> geometry;

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
