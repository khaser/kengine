#pragma once

#include "Primitives/Vec3.h"
#include "Primitives/Intersection.h"
#include "Primitives/Ray.h"
#include "Distribution.h"

#include <functional>
#include <memory>

struct Material {
    Vec3<float> color;
    virtual Vec3<float> sample(Ray w_in, Intersection i, const Distribution& dist,
                                    const std::function<Vec3<float>(const Ray&)> &raycast) = 0;
};

struct Diffuse : public Material {
    Vec3<float> emission;
    Vec3<float> sample(Ray w_in, Intersection i, const Distribution& dist,
                            const std::function<Vec3<float>(const Ray&)> &raycast);
};

struct Metallic : public Material {
    Vec3<float> emission;
    Vec3<float> sample(Ray w_in, Intersection i, const Distribution& dist,
                            const std::function<Vec3<float>(const Ray&)> &raycast);
};

struct Dielectric : public Material {
    float ior;
    Vec3<float> sample(Ray w_in, Intersection i, const Distribution& dist,
                            const std::function<Vec3<float>(const Ray&)> &raycast);
private:
    float get_reflectness(float cos_phi1);
};

