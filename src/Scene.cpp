#include "Camera.h"
#include "Object.h"
#include "Primitives.h"
#include "Rnd.h"
#include "Distribution.h"

#include "Scene.h"

#include <iostream>
#include <algorithm>
#include <fstream>
#include <utility>
#include <type_traits>
#include <string>
#include <memory>
#include <vector>

using namespace std::placeholders;

Scene::Scene(std::ifstream is) {

    std::string token;
    std::vector<std::unique_ptr<Distribution>> dists;
    dists.push_back(std::make_unique<CosineDistribution>());

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
            }

        } else {
            std::cerr << "Unknown token: " << token << std::endl;
        }
    }

    auto dist = std::make_shared<MixedDistribution>(std::move(dists));
    for (auto &obj : objs) {
        if (auto t = dynamic_pointer_cast<Diffuse>(obj.material)) {
            t->dist = dist;
        }
    }
}


std::vector<std::vector<Vec3<double>>> Scene::render_scene() {
    std::vector<std::vector<Vec3<double>>> output(dimensions.second, std::vector<Vec3<double>>(dimensions.first));
#pragma omp parallel for
    for (uint16_t x = 0; x < dimensions.first; ++x) {
        for (uint16_t y = 0; y < dimensions.second; ++y) {
            Vec3<double> pixel = {0, 0, 0};
            for (uint16_t sample = 0; sample < samples; ++sample) {
                double x_01 = (x + Rnd::getRnd()->uniform(0, 1)) / dimensions.first;
                double y_01 = (y + Rnd::getRnd()->uniform(0, 1)) / dimensions.second;
                double x_11 = x_01 * 2 - 1;
                double y_11 = y_01 * 2 - 1;
                pixel = pixel + raycast(camera.raycast(x_11, -y_11), ray_depth);
            }
            output[y][x] = postprocess(pixel / samples);
        }
    }
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
        std::cerr << "No intersections\n";
        return bg_color;
    }
}

std::optional<std::pair<Object, Intersection>> Scene::get_intersect(const Ray& ray) const {
    std::optional<std::pair<Object, Intersection>> bound;
    for (auto& obj : objs) {
        std::vector<Intersection> obj_intersections = obj.geometry->get_intersect(ray);
        for (auto &inter : obj_intersections) {
            if (!bound || bound.value().second.t > inter.t) {
                bound = {{obj, inter}};
            }
        }
    }
    return bound;
}
