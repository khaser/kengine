#include <memory>
#include <optional>
#include <functional>
#include <stdexcept>

#include "Rnd.h"
#include "Primitives.h"

#include "Object/Material.h"

Vec3<double> Diffuse::sample(Ray w_in, Intersection i,
                        const std::function<Vec3<double>(const Ray&)> &raycast) {
    Vec3<double> pos = w_in.reveal(i.t);
    Vec3<double> w_out = Rnd::getRnd()->in_hemisphere(i.normal);
    Ray r_out = Ray {pos, w_out};
    r_out.bump();
    return emission + color * 2 * raycast(r_out) * (i.normal % w_out);
}

static Ray reflect(const Vec3<double> pos, const Vec3<double> d, const Vec3<double> normal) {
    Ray reflected = {pos, d - normal * 2 * (normal % d)};
    reflected.bump();
    return reflected;
};

Vec3<double> Metallic::sample(Ray w_in, Intersection i,
                        const std::function<Vec3<double>(const Ray&)> &raycast) {
    Vec3<double> pos = w_in.reveal(i.t);
    return emission + color * raycast(reflect(pos, w_in.v, i.normal));
}

Vec3<double> Dielectric::sample(Ray w_in, Intersection i,
                        const std::function<Vec3<double>(const Ray&)> &raycast) {
    double k = (i.is_inside ? ior : 1 / ior);

    Vec3<double> pos = w_in.reveal(i.t);

    double cos_phi1 = i.normal % -w_in.v;
    double sin_phi2 = k * sqrt(1 - cos_phi1 * cos_phi1);
    if (sin_phi2 > 1) {
        // zero refract case
        Ray reflected = reflect(pos, w_in.v, i.normal);
        reflected.bump();
        return raycast(reflected);
    } else {
        double cos_phi2 = sqrt(1 - sin_phi2 * sin_phi2);
        double rnd_val = Rnd::getRnd()->uniform(0, 1);

        if (rnd_val > get_reflectness(cos_phi1, i.is_inside)) {
            // refraction case
            Ray refracted = {pos, (w_in.v * k + i.normal * (k * cos_phi1 - cos_phi2)).norm()};
            refracted.bump();
            return color * raycast(refracted);
        } else {
            // reflection case
            Ray reflected = reflect(pos, w_in.v, i.normal);
            reflected.bump();
            return raycast(reflected);
        }
    }

}

double Dielectric::get_reflectness(double cos_phi1, bool is_inside) {
    double r0 = (ior - 1) / (ior + 1);
    r0 *= r0;
    return r0 + (1 - r0) * pow(1 - cos_phi1, 5);
}

