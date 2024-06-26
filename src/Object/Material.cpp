#include <memory>
#include <optional>
#include <functional>
#include <stdexcept>
#include <algorithm>
#include <math.h>
#include <iostream>
#include <cmath>

#include "Rnd.h"
#include "Primitives.h"
#include "Distribution.h"

#include "Object/Material.h"

Vec3<float> Diffuse::sample(Ray w_in, Intersection i, const Distribution& dist,
                            const std::function<Vec3<float>(const Ray&)> &raycast) {
    Vec3<float> pos = w_in.reveal(i.t);
    {
        Ray r = {pos, i.normal};
        r.bump();
        pos = r.start;
    }
    Vec3<float> w_out = dist.sample(pos, i.normal);
    Ray r_out = Ray {pos, w_out};

    r_out.bump();
    if (emission.len() > 1e-5) {
        return emission;
    }
    if (w_out % i.normal <= 1e-6) {
        return {0, 0, 0};
    }

    auto tmp = dist.pdf(pos, i.normal, w_out);
    return emission + (color / M_PI) * raycast(r_out) * (i.normal % w_out) / tmp;
}

static Ray reflect(const Vec3<float> pos, const Vec3<float> d, const Vec3<float> normal) {
    Ray reflected = {pos, d - normal * 2 * (normal % d)};
    reflected.bump();
    return reflected;
};

Vec3<float> Metallic::sample(Ray w_in, Intersection i, const Distribution& dist,
                        const std::function<Vec3<float>(const Ray&)> &raycast) {
    Vec3<float> pos = w_in.reveal(i.t);
    return emission + color * raycast(reflect(pos, w_in.v, i.normal));
}

Vec3<float> Dielectric::sample(Ray w_in, Intersection i, const Distribution& dist,
                        const std::function<Vec3<float>(const Ray&)> &raycast) {
    float k = (i.is_inside ? ior : 1 / ior);

    Vec3<float> pos = w_in.reveal(i.t);

    float cos_phi1 = i.normal % -w_in.v;
    float sin_phi2 = k * sqrt(1 - cos_phi1 * cos_phi1);
    if (sin_phi2 > 1) {
        // zero refract case
        Ray reflected = reflect(pos, w_in.v, i.normal);
        reflected.bump();
        return raycast(reflected);
    } else {
        float cos_phi2 = sqrt(1 - sin_phi2 * sin_phi2);
        float rnd_val = Rnd::getRnd()->uniform(0, 1);

        if (rnd_val > get_reflectness(cos_phi1)) {
            // refraction case
            Ray refracted = {pos, (w_in.v * k + i.normal * (k * cos_phi1 - cos_phi2)).norm()};
            refracted.bump();
            return (!i.is_inside ? color : Vec3<float>{1}) * raycast(refracted);
        } else {
            // reflection case
            Ray reflected = reflect(pos, w_in.v, i.normal);
            reflected.bump();
            return raycast(reflected);
        }
    }

}

float pow5(float x) {
    float tmp = x * x;
    return tmp * tmp * x;
}

float Dielectric::get_reflectness(float cos_phi1) {
    float r0 = (ior - 1) / (ior + 1);
    r0 *= r0;
    return r0 + (1 - r0) * pow5(1 - cos_phi1);
}

