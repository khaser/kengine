#pragma once

#include "Primitives.h"
#include "Distribution.h"

#include <functional>
#include <memory>

struct Material {
    Vec3<double> color;
    virtual ~Material() {};
    virtual Vec3<double> sample(Ray w_in, Intersection i,
                                    const std::function<Vec3<double>(const Ray&)> &raycast) = 0;
};

struct Diffuse : public Material {
    Vec3<double> emission;
    std::shared_ptr<Distribution> dist;
    Vec3<double> sample(Ray w_in, Intersection i,
                            const std::function<Vec3<double>(const Ray&)> &raycast);
};

struct Metallic : public Material {
    Vec3<double> emission;
    Vec3<double> sample(Ray w_in, Intersection i,
                            const std::function<Vec3<double>(const Ray&)> &raycast);
};

struct Dielectric : public Material {
    double ior;
    Vec3<double> sample(Ray w_in, Intersection i,
                            const std::function<Vec3<double>(const Ray&)> &raycast);
private:
    double get_reflectness(double cos_phi1, bool is_inside);
};

