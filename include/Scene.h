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
#include "Light.h"
#include "Intersection.h"

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
        /* return x; */
    }

    Vec3<double> raycast(const Ray& ray, int ttl) const {
        if (ttl == 0) {
            /* return {}; */
            return bg_color;
        }
        auto tmp = get_intersect(ray);
        if (tmp) {
            auto& [obj, intersect] = tmp.value();
            Vec3<double> pos = ray.reveal(intersect.t);
            if (obj.material == DIFFUSE) {
                return get_illumination(pos, intersect.normal) * obj.color;
            } else if (obj.material == METALLIC) {
                Ray reflected = reflect(pos, ray.v, intersect.normal);
                return obj.color * raycast(reflected, ttl - 1);
            } else if (obj.material == DIELECTRIC) {
                Ray reflected = reflect(pos, ray.v, intersect.normal);
                double ior = obj.ior;
                if (!intersect.is_inside) {
                    ior = 1 / ior;
                }
                double cos_phi1 = intersect.normal % -ray.v;
                double sin_phi2 = ior * sqrt(1 - cos_phi1 * cos_phi1);
                if (sin_phi2 > 1) {
                    // zero infiltrate case
                    return raycast(reflected, ttl - 1) * obj.color;
                } else {
                    double cos_phi2 = sqrt(1 - sin_phi2 * sin_phi2);
                    Vec3<double> infiltrated_v = -ray.v * -ior + intersect.normal * (ior * cos_phi1 - cos_phi2);
                    Ray infiltrated = {pos, infiltrated_v.norm()};
                    infiltrated.start = infiltrated.start + infiltrated.v * 1e-5;
                    double r0 = (1 - ior) / (ior + 1);
                    r0 *= r0;
                    double reflectness = r0 + (1 - r0) * pow(1 - cos_phi1, 5);
                    return (raycast(reflected, ttl - 1) * reflectness + raycast(infiltrated, ttl - 1) * (1 - reflectness)) * obj.color;
                }
            }
            std::cerr << "Undefined material, black color used" << std::endl;
            /* return (intersect.normal + Vec3<double>{1, 1, 1}) * 0.5; */
            return {};
        } else {
            return bg_color;
        }
    }

    // TODO: move out
    Ray reflect(const Vec3<double> pos, const Vec3<double> d, const Vec3<double> normal) const {
        Ray reflected = {pos, d - normal * 2 * (normal % d)};
        reflected.start = reflected.start + reflected.v * 1e-5;
        return reflected;
    }

    Vec3<double> get_illumination(const Vec3<double>& p, const Vec3<double>& normal) const {
        Vec3<double> illumination = ambient_light;
        for (auto& light : lights) {
            double visibility = 0;
            if (dynamic_cast<PointLight*>(light.get()) != nullptr) {
                auto plight = dynamic_cast<PointLight*>(light.get());
                auto v = plight->position - p;
                Ray ray = {p + v * 1e-5, v.norm()};
                auto inter = get_intersect(ray);
                if (!inter || inter.value().second.t > v.len()) {
                    visibility = std::max(0.0, ray.v % normal);
                }
            } else if (dynamic_cast<DirectLight*>(light.get()) != nullptr) {
                auto dlight = dynamic_cast<DirectLight*>(light.get());
                if (!get_intersect(Ray{p + dlight->direction * 1e-5, dlight->direction})) {
                    visibility = std::max(0.0, dlight->direction % normal);
                }
            }
            illumination = illumination + light->get_irradiance(p) * visibility;
        }
        /* std::cerr << illumination << std::endl; */
        return illumination;
    }

    std::optional<std::pair<Primitive, Intersection>> get_intersect(const Ray& ray) const {
        std::optional<std::pair<Primitive, Intersection>> bound;
        for (auto& obj : objs) {
            std::optional<Intersection> t = obj.get_intersect(ray);
            if (t.has_value() && (!bound || bound.value().second.t > t.value().t)) {
                bound = {{obj, t.value()}};
            }
        }
        return bound;
    }
};
