#include "Camera.h"
#include "Object.h"
#include "Primitives.h"
#include "Rnd.h"
#include "Distribution.h"

#include "Scene.h"

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <fstream>
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

Scene::Scene(std::ifstream is) {

    std::string token;
    std::vector<Object> objs;
    std::vector<std::shared_ptr<LightDistribution>> dists;

    while (is >> token) {
        if (token == "DIMENSIONS") {
            is >> dimensions.first >> dimensions.second;
        } else if (token == "BG_COLOR") {
            is >> bg_color;
        } else if (token == "RAY_DEPTH") {
            is >> ray_depth;
        } else if (token == "SAMPLES") {
            is >> samples;
        } else if (token == "AMBIENT_LIGHT") {
            is >> ambient_light;
        } else if (token == "CAMERA_POSITION") {
            is >> camera.position;
        } else if (token == "CAMERA_RIGHT") {
            is >> camera.right;
        } else if (token == "CAMERA_UP") {
            is >> camera.up;
        } else if (token == "CAMERA_FORWARD") {
            is >> camera.forward;
        } else if (token == "CAMERA_FOV_X") {
            is >> camera.fov_x;
            camera.calc_fov_y(dimensions.first, dimensions.second);
        } else if (token == "POSITION") {
            is >> objs.back().geometry->position;
        } else if (token == "ROTATION") {
            is >> objs.back().geometry->rotation;
        } else if (token == "COLOR") {
            is >> objs.back().material->color;
        } else if (token == "NEW_PRIMITIVE") {
            objs.emplace_back();
            objs.back().material = std::make_unique<Diffuse>();
        } else if (token == "PLANE") {
            Vec3<double> norm;
            is >> norm;
            objs.back().geometry = std::make_unique<Plane>(norm);
        } else if (token == "ELLIPSOID") {
            Vec3<double> r;
            is >> r;
            objs.back().geometry = std::make_unique<Ellipsoid>(r);
        } else if (token == "BOX") {
            Vec3<double> size;
            is >> size;
            objs.back().geometry = std::make_unique<Box>(size);
        } else if (token == "TRIANGLE") {
            Mat3<double> v;
            is >> v.x >> v.y >> v.z;
            objs.back().geometry = std::make_unique<Triangle>(v);
        } else if (token == "METALLIC") {
            auto color = objs.back().material->color;
            objs.back().material = std::make_unique<Metallic>();
            objs.back().material->color = color;
        } else if (token == "DIELECTRIC") {
            auto color = objs.back().material->color;
            objs.back().material = std::make_unique<Dielectric>();
            objs.back().material->color = color;
        } else if (token == "IOR") {
            double ior;
            is >> ior;
            dynamic_pointer_cast<Dielectric>(objs.back().material)->ior = ior;
        } else if (token == "EMISSION") {
            Vec3<double> emission;
            is >> emission;
            dynamic_pointer_cast<Diffuse>(objs.back().material)->emission = emission;

            if (auto t = std::dynamic_pointer_cast<Box>(objs.back().geometry)) {
                dists.push_back(std::make_unique<BoxDistribution>(t));
            } else if (auto t = std::dynamic_pointer_cast<Ellipsoid>(objs.back().geometry)) {
                dists.push_back(std::make_unique<EllipsoidDistribution>(t));
            } else if (auto t = std::dynamic_pointer_cast<Triangle>(objs.back().geometry)) {
                dists.push_back(std::make_unique<TriangleDistribution>(t));
            }

        } else {
            std::cerr << "Unknown token: " << token << std::endl;
        }
    }

    light_pdf = std::make_unique<MixedDistribution>(std::move(dists));
    for (auto &obj : objs) {
        if (auto t = dynamic_pointer_cast<Diffuse>(obj.material)) {
            t->dist = light_pdf.get();
        }
    }

    auto bvh_end = std::partition(objs.begin(), objs.end(), [] (const T& obj) -> bool {
        return dynamic_pointer_cast<Plane>(obj.geometry) == 0;
    });

    non_bvh_objs = std::vector(bvh_end, objs.end());
    bvh = BVH(std::nullopt, objs.begin(), bvh_end);
}


std::vector<std::vector<Vec3<double>>> Scene::render_scene() {
    std::vector<std::vector<Vec3<double>>> output(dimensions.second, std::vector<Vec3<double>>(dimensions.first));
    std::atomic_size_t samples_processed = 0;
    size_t samples_total = dimensions.first * dimensions.second * samples;
#pragma omp parallel
    {
#pragma omp single nowait
    {
    while (samples_processed != samples_total) {
        std::cerr << samples_processed << ' ' << samples_total << std::endl;
        std::cerr << "Generated " << std::fixed << std::setprecision(4) << 100 * 1.0 * samples_processed / samples_total << "% of samples\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    }
#pragma omp for schedule(dynamic)
    for (uint16_t x = 0; x < dimensions.first; ++x) {
        for (uint16_t y = 0; y < dimensions.second; ++y) {
            Vec3<double> pixel = {0, 0, 0};
            for (uint16_t sample = 0; sample < samples; ++sample) {
                double x_01 = (x + Rnd::getRnd()->uniform(0, 1)) / dimensions.first;
                double y_01 = (y + Rnd::getRnd()->uniform(0, 1)) / dimensions.second;
                double x_11 = x_01 * 2 - 1;
                double y_11 = y_01 * 2 - 1;
                pixel = pixel + raycast(camera.raycast(x_11, -y_11), ray_depth);
                samples_processed++;
            }
            output[y][x] = postprocess(pixel / samples);
        }
    }
    } // omp parallel
    return output;
}

// x, y in [-1, 1]
Vec3<double> Scene::postprocess(Vec3<double> in_color) {
    return gamma_correction(aces_tonemap(in_color));
}

Vec3<double> Scene::aces_tonemap(const Vec3<double> &x) {
    const double a = 2.51;
    const double b = 0.03;
    const double c = 2.43;
    const double d = 0.59;
    const double e = 0.14;
    return (x*(x*a+b))/(x*(x*c+d)+e);
}

Vec3<double> Scene::gamma_correction(const Vec3<double> &x) {
    return saturate(pow(x, 1 / 2.2));
}

Vec3<double> Scene::saturate(const Vec3<double> &color) {
    return color.clamp(Vec3<double>(0), Vec3<double>(1));
}

Vec3<double> Scene::raycast(const Ray& ray, int ttl) const {
    if (ttl == 0) {
        return bg_color;
    }
    auto tmp = get_intersect(ray);
    if (tmp) {
        auto& [obj, intersect] = tmp.value();
        auto raycast_fn = std::bind(&Scene::raycast, this, _1, ttl - 1);
        return obj.material->sample(ray, intersect, raycast_fn);
    } else {
        return bg_color;
    }
}

std::optional<std::pair<Object, Intersection>> Scene::get_intersect(const Ray& ray) const {
    std::vector<F> node_inters;
    std::transform(non_bvh_objs.begin() , non_bvh_objs.end(), std::back_inserter(node_inters), Map(ray));
    F non_bvh_inter = std::accumulate(node_inters.begin(), node_inters.end(), static_cast<F>(std::nullopt), Merge());
    return Merge() (non_bvh_inter, bvh.get_intersect(ray, true));
}
