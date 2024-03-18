#pragma once

#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>
#include <optional>
#include <utility>
#include <type_traits>

#include "Vec3.h"
#include "Camera.h"
#include "Primitive.h"
#include "Intersection.h"
#include "Quaternion.h"

using namespace std::placeholders;

struct Scene {
    std::pair<uint16_t, uint16_t> dimensions;
    Vec3<double> bg_color;
    Vec3<double> ambient_light;
    Camera camera;
    std::vector<Object> objs;

    int ray_depth;
    int samples;

    Scene(std::ifstream is) {
        std::string token;
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
            } else if (token == "NEW_PRIMITIVE") {
                objs.emplace_back();
            } else if (token == "POSITION") {
                is >> objs.back().geometry->position;
            } else if (token == "ROTATION") {
                is >> objs.back().geometry->rotation;
            } else if (token == "COLOR") {
                is >> objs.back().material->color;
            } else if (token == "PLANE") {
                Vec3<double> norm;
                is >> norm;
                objs.back().geometry = std::unique_ptr<Geometry>(new Plane(norm));
            } else if (token == "ELLIPSOID") {
                Vec3<double> r;
                is >> r;
                objs.back().geometry = std::unique_ptr<Geometry>(new Ellipsoid(r));
            } else if (token == "BOX") {
                Vec3<double> size;
                is >> size;
                objs.back().geometry = std::unique_ptr<Geometry>(new Box(size));
            } else if (token == "NEW_PRIMITIVE") {
                objs.emplace_back();
            } else if (token == "METALLIC") {
                objs.back().material = std::unique_ptr<Material>(new Metallic());
            } else if (token == "DIELECTRIC") {
                objs.back().material = std::unique_ptr<Material>(new Dielectric());
            } else if (token == "IOR") {
                double ior;
                is >> ior;
                dynamic_cast<Dielectric*>(objs.back().material.get())->ior = ior;
            } else if (token == "EMISSION") {
                Vec3<double> emission;
                is >> emission;
                dynamic_cast<Diffuse*>(objs.back().material.get())->emission = emission;
            } else {
                std::cerr << "Unknown token: " << token << std::endl;
                /* is.ignore(std::numeric_limits<std::streamsize>::max()); */
            }
        }
    }

    std::vector<std::vector<Vec3<double>>> render_scene() {
        std::vector<std::vector<Vec3<double>>> output(dimensions.second, std::vector<Vec3<double>>(dimensions.first));
        for (uint16_t x = 0; x < dimensions.first; ++x) {
            for (uint16_t y = 0; y < dimensions.second; ++y) {
                double x_01 = (x + 0.5) / dimensions.first;
                double y_01 = (y + 0.5) / dimensions.second;
                // TODO render pixel several times due to Monte-Carlo sampling
                output[y][x] = render_pixel(x_01 * 2 - 1, -(y_01 * 2 - 1));
            }
        }
        return output;
    }

private:
    // x, y in [-1, 1]
    Vec3<double> render_pixel(double x, double y) {
        Ray ray = camera.raycast(x, y);
        auto res = aces_tonemap(raycast(ray, ray_depth));
        return gamma_correction(res);
    }

    Vec3<double> saturate(const Vec3<double> &color) {
        return color.clamp(Vec3<double>(0), Vec3<double>(1));
    }

    Vec3<double> aces_tonemap(const Vec3<double> &x) {
        const double a = 2.51;
        const double b = 0.03;
        const double c = 2.43;
        const double d = 0.59;
        const double e = 0.14;
        return (x*(x*a+b))/(x*(x*c+d)+e);
    }

    Vec3<double> gamma_correction(const Vec3<double> &x) {
        return saturate(pow(x, 1 / 2.2));
    }

    Vec3<double> raycast(const Ray& ray, int ttl) const {
        if (ttl == 0) {
            return {};
        }
        auto tmp = get_intersect(ray);
        if (tmp) {
            auto& [obj, intersect] = tmp.value();
            auto raycast_fn = std::bind(&Scene::raycast, this, _1, ttl - 1);
            obj.material->sample(ray, intersect, raycast_fn);
            std::cerr << "Undefined material, black color used" << std::endl;
            /* return (intersect.normal + Vec3<double>{1, 1, 1}) * 0.5; */
            return {};
        } else {
            return bg_color;
        }
    }

    std::optional<std::pair<Object, Intersection>> get_intersect(const Ray& ray) const {
        std::optional<std::pair<Object, Intersection>> bound;
        for (auto& obj : objs) {
            std::optional<Intersection> t = obj.geometry->get_intersect(ray);
            if (t.has_value() && (!bound || bound.value().second.t > t.value().t)) {
                bound = {{obj, t.value()}};
            }
        }
        return bound;
    }
};
