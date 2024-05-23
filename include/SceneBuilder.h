#pragma once

#include "Primitives.h"
#include "Camera.h"
#include "Object.h"
#include "third-party/json.hpp"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <string>

using json = nlohmann::json;

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

class GltfBuilder : public SceneBuilder {

public:
    GltfBuilder (std::ifstream &fin, std::filesystem::path base_directory, Setup&& setup_) {
        // TODO: implement node hierarchy processing. See "children" field in doc
        setup = (std::move(setup_));

        json data = json::parse(fin);

        std::vector<std::ifstream> buffers;
        for (const auto &i : data["buffers"]) {
            std::filesystem::path filename = base_directory / std::string(i["uri"]);
            if (!std::filesystem::exists(filename)) {
                throw std::logic_error("buffer filename doesn't exists");
            }
            buffers.emplace_back(filename, std::ios_base::binary);
        }

        for (const auto &i : data["nodes"]) {
            if (i.contains("camera")) {
                int camera_id = i["camera"];
                Quaternion rotation = read_quat(i, "rotation", Quaternion{});
                camera.position = read_vec(i, "translation", Vec3<float>(0));
                camera.forward = rotation * Vec3<float>(0, 0, -1);
                camera.right = rotation * Vec3<float>(1, 0, 0);
                camera.up = rotation * Vec3<float>(0, 1, 0);
                auto &perspective = data["cameras"][camera_id]["perspective"];
                camera.fov_x = perspective["yfov"];
                camera.calc_fov_y(setup.dimensions.first, setup.dimensions.second);
                std::swap(camera.fov_x, camera.fov_y);
            } else if (i.contains("mesh")) {
                Vec3<float> scale = read_vec(i, "scale", Vec3<float>(1));
                Vec3<float> position = read_vec(i, "translation", Vec3<float>(0));
                Quaternion rotation = read_quat(i, "rotation", Quaternion{});
                size_t mesh_id = i["mesh"];
                const auto& primitives = data["meshes"][mesh_id]["primitives"];
                for (const auto &primitive : primitives) {
                    // Now parser support only default primitive mode: TRIANGLES,
                    // so we should ensure it
                    if (primitive.contains("mode") && primitive["mode"] != 4) {
                        throw std::logic_error("Unsupported mode for GLTF primitive");
                    }
                    size_t positions_acc_id = primitive["attributes"]["POSITION"];
                    auto position_acc = data["accessors"][positions_acc_id];
                    // We work only with triangles so we can calculate normals by vertexes
                    auto position_buff = data["bufferViews"][static_cast<size_t>(position_acc["bufferView"])];
                    // TODO: check buffer types

                    std::vector<Vec3<float>> v_positions;
                    {
                        std::ifstream &fin = buffers[position_buff["buffer"]];
                        size_t offset = position_buff["byteOffset"];
                        size_t len = position_buff["byteLength"];
                        fin.seekg(offset, std::ios_base::beg);
                        for (int i = 0; i < len; i += 3 * sizeof(float)) {
                            float x, y, z;
                            fin.read(reinterpret_cast<char*>(&x), sizeof(x));
                            fin.read(reinterpret_cast<char*>(&y), sizeof(y));
                            fin.read(reinterpret_cast<char*>(&z), sizeof(z));
                            v_positions.emplace_back(x, y, z);
                        }
                    }

                    auto indices_acc = data["accessors"][static_cast<size_t>(primitive["indices"])];
                    auto indices_buff = data["bufferViews"][static_cast<size_t>(indices_acc["bufferView"])];
                    std::vector<size_t> v_indices;
                    {
                        std::ifstream &fin = buffers[indices_buff["buffer"]];
                        size_t offset = indices_buff["byteOffset"];
                        size_t len = indices_buff["byteLength"];
                        fin.seekg(offset, std::ios_base::beg);
                        for (int i = 0; i < len; i += sizeof(uint16_t)) {
                            uint16_t x;
                            fin.read(reinterpret_cast<char*>(&x), sizeof(x));
                            v_indices.emplace_back(x);
                        }
                    }

                    if (v_indices.size() % 3 != 0) {
                        throw std::logic_error("3 is not divisor of v_indices size");
                    }
                    for (int i = 0; i < v_indices.size(); i += 3) {
                        objs.push_back({
                            std::make_shared<Diffuse> (),
                            std::make_shared<Triangle> (Mat3<float>{
                                v_positions[v_indices[i]],
                                v_positions[v_indices[i+1]],
                                v_positions[v_indices[i+2]]
                            })
                        });
                    }
                }
            } else {
                std::cerr << "Unparsed node";
            }
        }
    }

    Vec3<float> read_vec(const json &j, const std::string &field, const Vec3<float> &def) {
        if (j.contains(field)) {
            return {j[field][0], j[field][1], j[field][2]};
        } else {
            return def;
        }
    }

    Quaternion read_quat(const json &j, const std::string &field, const Quaternion &def) {
        if (j.contains(field)) {
            return {{j[field][0], j[field][1], j[field][2]}, j[field][3]};
        } else {
            return def;
        }
    }
};
