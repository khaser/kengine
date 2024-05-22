#pragma once

#include "Primitives.h"
#include "Camera.h"
#include "Object.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>

struct Setup {
    int ray_depth;
    uint16_t samples;
    Vec3<float> bg_color;
    Vec3<float> ambient_light;
    std::pair<uint16_t, uint16_t> dimensions;
};

class SceneBuilder {
public:
    Setup setup;
    Camera camera;
    std::vector<Object> objs;
};

class TrivialBuilder : public SceneBuilder {

public:
    TrivialBuilder (std::istream &is) {
        std::string token;

        while (is >> token) {
            if (token == "DIMENSIONS") {
                is >> setup.dimensions.first >> setup.dimensions.second;
            } else if (token == "BG_COLOR") {
                is >> setup.bg_color;
            } else if (token == "RAY_DEPTH") {
                is >> setup.ray_depth;
            } else if (token == "SAMPLES") {
                is >> setup.samples;
            } else if (token == "AMBIENT_LIGHT") {
                is >> setup.ambient_light;
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
                camera.calc_fov_y(setup.dimensions.first, setup.dimensions.second);
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
                Vec3<float> norm;
                is >> norm;
                objs.back().geometry = std::make_unique<Plane>(norm);
            } else if (token == "ELLIPSOID") {
                Vec3<float> r;
                is >> r;
                objs.back().geometry = std::make_unique<Ellipsoid>(r);
            } else if (token == "BOX") {
                Vec3<float> size;
                is >> size;
                objs.back().geometry = std::make_unique<Box>(size);
            } else if (token == "TRIANGLE") {
                Mat3<float> v;
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
                float ior;
                is >> ior;
                dynamic_pointer_cast<Dielectric>(objs.back().material)->ior = ior;
            } else if (token == "EMISSION") {
                Vec3<float> emission;
                is >> emission;
                dynamic_pointer_cast<Diffuse>(objs.back().material)->emission = emission;
            } else {
                std::cerr << "Unknown token: " << token << std::endl;
            }
        }
    }
};
