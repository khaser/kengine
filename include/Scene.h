#pragma once

#include <iostream>
#include <fstream>
#include <vector>

#include "Image.h"
#include "Vec3.h"
#include "Camera.h"
#include "Primitive.h"
#include "Light.h"

struct Scene {
    std::pair<uint16_t, uint16_t> dimensions;
    Vec3<double> bg_color;
    Camera camera;
    std::vector<Primitive> objs;

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
                Vec3<double> norm;
                is >> norm;
                objs.back().geom = std::unique_ptr<Geometry>(new Ellipsoid(norm));
            } else if (token == "BOX") {
                Vec3<double> norm;
                is >> norm;
                objs.back().geom = std::unique_ptr<Geometry>(new Box(norm));
            } else if (token == "NEW_PRIMITIVE") {
                objs.emplace_back();
            } else {
                std::cerr << "Unknown token: " << token << std::endl;
                /* is.ignore(std::numeric_limits<std::streamsize>::max()); */
            }
        }
    }

    Image render_scene() {
        std::vector<Vec3<double>> output(dimensions.first * dimensions.second * 3);
        for (uint16_t x = 0; x < dimensions.first; ++x) {
            for (uint16_t y = 0; y < dimensions.second; ++y) {
                double x_01 = (x + 0.5) / dimensions.first;
                double y_01 = (y + 0.5) / dimensions.second;
                output[y * dimensions.first + x] = render_pixel(x_01 * 2 - 1, -(y_01 * 2 - 1));
            }
        }
        return Image(dimensions.first, dimensions.second, output);
    }

private:
    // x, y in [-1, 1]
    Vec3<double> render_pixel(double x, double y) {
        Ray ray = camera.raycast(x, y);
        std::pair<Vec3<double>, double> bound = {bg_color, 1e9};

        for (auto& obj : objs) {
            auto t = obj.get_intersect(ray);
            if (t && bound.second > *t) {
                bound = {obj.color, *t};
            }
        }

        return bound.first;
    }
};
