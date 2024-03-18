#include "Rnd.h"
#include "Vec3.h"

#include <random>

Vec3<double> Rnd::in_sphere() {
    double z = uniform(-1, 1);
    double phi = uniform(0, 2 * std::numbers::pi);
    double sq = sqrt(1 - z * z);
    return {sq * cos(phi), sq * sin(phi), z};
}

double Rnd::uniform(double Min, double Max) {
    std::uniform_real_distribution<double> dis(Min, Max);
    return dis(rnd);
}

Vec3<double> Rnd::in_hemisphere(Vec3<double> v) {
    Vec3<double> res = in_sphere();
    if (res % v < 0) {
        res = -res;
    }
    return res;
}

Rnd* Rnd::getRnd() {
    if (!instance) {
        return instance = new Rnd();
    } else {
        return instance;
    }
}

Rnd *Rnd::instance = nullptr;
