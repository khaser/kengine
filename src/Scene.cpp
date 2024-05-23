#include "Camera.h"
#include "Object.h"
#include "Primitives.h"
#include "Rnd.h"
#include "Distribution.h"

#include "Scene.h"

#include <algorithm>
#include <optional>
#include <thread>
#include <utility>
#include <type_traits>
#include <string>
#include <memory>
#include <vector>
#include <atomic>

using namespace std::placeholders;

using namespace BVH_bounds;

Scene::Scene(SceneBuilder&& builder) : objs(std::move(builder.objs)), setup(std::move(builder.setup)), camera(std::move(builder.camera)) {
    std::vector<std::shared_ptr<LightDistribution>> dists;

    for (auto& i : objs) {
        if (i.material->emission.len() <= 1e-5) continue;
        if (auto t = std::dynamic_pointer_cast<Triangle>(i.geometry)) {
            dists.push_back(std::make_unique<TriangleDistribution>(t));
        }
    }

    light_pdf = std::make_unique<MixedDistribution>(std::move(dists));

    bvh = BVH(std::nullopt, objs.begin(), objs.end());
}


std::vector<std::vector<Vec3<float>>> Scene::render_scene() {
    std::vector<std::vector<Vec3<float>>> output(setup.dimensions.second, std::vector<Vec3<float>>(setup.dimensions.first));
    std::atomic_size_t samples_processed = 0;
    size_t samples_total = setup.dimensions.first * setup.dimensions.second * setup.samples;
    auto start_clock = clock();
    std::cerr << "Start rendering scene with following setup:\n";
    std::cerr << "Output resolution: " << setup.dimensions.first << 'x' << setup.dimensions.second << std::endl;
    std::cerr << "Samples per pixel: " << setup.samples << std::endl;
    std::cerr << "Object primitives in scene: " << objs.size() << std::endl;
    for (uint16_t x = 0; x < setup.dimensions.first; ++x) {
#pragma omp parallel for schedule(dynamic)
        for (uint16_t y = 0; y < setup.dimensions.second; ++y) {
            Vec3<float> pixel = {0, 0, 0};
            for (uint16_t sample = 0; sample < setup.samples; ++sample) {
                float x_01 = (x + Rnd::getRnd()->uniform(0, 1)) / setup.dimensions.first;
                float y_01 = (y + Rnd::getRnd()->uniform(0, 1)) / setup.dimensions.second;
                float x_11 = x_01 * 2 - 1;
                float y_11 = y_01 * 2 - 1;
                pixel = pixel + raycast(camera.raycast(x_11, -y_11), setup.ray_depth);
                samples_processed++;
            }
            output[y][x] = postprocess(pixel / setup.samples);
        }
        /* std::cerr << samples_processed << ' ' << samples_total << std::endl; */
        /* std::cerr << "Generated " << std::fixed << std::setprecision(4) << 100 * 1.0 * samples_processed / samples_total << "% of samples\n"; */
        /* std::cerr << "Spent time: " << 0.25 * (clock() - start_clock) / CLOCKS_PER_SEC << std::endl; */
    }
    return output;
}

// x, y in [-1, 1]
Vec3<float> Scene::postprocess(Vec3<float> in_color) {
    return gamma_correction(aces_tonemap(in_color));
}

Vec3<float> Scene::aces_tonemap(const Vec3<float> &x) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return (x*(x*a+b))/(x*(x*c+d)+e);
}

Vec3<float> Scene::gamma_correction(const Vec3<float> &x) {
    return saturate(pow(x, 1 / 2.2));
}

Vec3<float> Scene::saturate(const Vec3<float> &color) {
    return color.clamp(Vec3<float>(0), Vec3<float>(1));
}

Vec3<float> Scene::raycast(const Ray& ray, int ttl) {
    if (ttl == 0) {
        return setup.bg_color;
    }
    auto tmp = get_intersect(ray);
    if (tmp) {
        auto& [obj, intersect] = tmp.value();
        auto raycast_fn = std::bind(&Scene::raycast, this, _1, ttl - 1);
        return obj.material->sample(ray, intersect, *light_pdf.get(), raycast_fn);
    } else {
        return setup.bg_color;
    }
}

std::optional<std::pair<Object, Intersection>> Scene::get_intersect(const Ray& ray) {
    return bvh.get_intersect(ray, true);
}
