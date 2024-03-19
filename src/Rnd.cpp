#include "Rnd.h"
#include "Primitives/Vec3.h"

#include <random>
#include <math.h>

Vec3<double> Rnd::in_sphere() {
    double z = uniform(-1.0, 1.0);
    double phi = uniform(0.0, 2 * M_PI);
    double sq = sqrt(1 - z * z);
    return {sq * cos(phi), sq * sin(phi), z};
}

double Rnd::uniform(double Min, double Max) {
    std::uniform_real_distribution<double> dis(Min, Max);
    return dis(rnd);
}

int Rnd::uniform_int(int Min, int Max) {
    std::uniform_int_distribution<int> dis(Min, Max - 1);
    return dis(rnd);
}

Rnd* Rnd::getRnd() {
    if (!instance) {
        return instance = new Rnd();
    } else {
        return instance;
    }
}

bool Rnd::bernoulli(double succ_rate) {
    std::bernoulli_distribution dis(succ_rate);
    return dis(rnd);
}

Rnd *Rnd::instance = nullptr;
