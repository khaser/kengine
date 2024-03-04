#pragma once

#include <iostream>
#include <fstream>
#include <vector>

#include "Vec3.h"
#include "Camera.h"
#include "Primitive.h"
#include "Light.h"

struct Scene {
    std::pair<uint16_t, uint16_t> dimensions;
    Vec3<double> bg_color;
    Camera camera;
    std::vector<Primitive> objs;
    std::vector<std::unique_ptr<Light>> lights;

    int ray_depth;
    Vec3<double> ambient_light;

    Scene(std::ifstream is) {
        std::string token;
        while (is >> token) {
            if (token == "DIMENSIONS") {
                is >> dimensions.first >> dimensions.second;
            } else if (token == "BG_COLOR") {
                is >> bg_color;
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
                is >> objs.back().position;
            } else if (token == "ROTATION") {
                is >> objs.back().rotation;
            } else if (token == "COLOR") {
                is >> objs.back().color;
            } else if (token == "PLANE") {
                Vec3<double> norm;
                is >> norm;
                objs.back().geom = std::unique_ptr<Geometry>(new Plane(norm));
            } else if (token == "ELLIPSOID") {
                Vec3<double> r;
                is >> r;
                objs.back().geom = std::unique_ptr<Geometry>(new Ellipsoid(r));
            } else if (token == "BOX") {
                Vec3<double> size;
                is >> size;
                objs.back().geom = std::unique_ptr<Geometry>(new Box(size));
            } else if (token == "NEW_PRIMITIVE") {
                objs.emplace_back();
            } else if (token == "RAY_DEPTH") {
                is >> ray_depth;
            } else if (token == "AMBIENT_LIGHT") {
                is >> ambient_light;
            } else if (token == "NEW_LIGHT") {
                lights.emplace_back(Light::from_istream(is));
            } else if (token == "METALLIC") {
                objs.back().material = METALLIC;
            } else if (token == "DIELECTRIC") {
                objs.back().material = DIELECTRIC;
            } else if (token == "IOR") {
                is >> objs.back().ior;
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
                output[y][x] = render_pixel(x_01 * 2 - 1, -(y_01 * 2 - 1));
            }
        }
        return output;
    }

private:
    // x, y in [-1, 1]
    Vec3<double> render_pixel(double x, double y) {
        Ray ray = camera.raycast(x, y);
        std::pair<Vec3<double>, double> bound = {bg_color, 1e9};

        for (auto& obj : objs) {
            std::optional<Intersection> t = obj.get_intersect(ray);
            if (t.has_value() && bound.second > t.value().t) {
                bound = {obj.color, t.value().t};
                /* bound = {t.value().normal, t.value().t}; */
            }
        }

        return bound.first;
    }
};
