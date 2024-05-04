#pragma once

#include <vector>
#include <optional>
#include <type_traits>

#include "Camera.h"
#include "Object.h"
#include "Primitives.h"
#include "BVH.h"

struct Scene {
    std::pair<uint16_t, uint16_t> dimensions;
    Vec3<double> bg_color;
    Vec3<double> ambient_light;
    Camera camera;
    BVH geometry;

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
