#include "Rnd.h"
#include "Primitives/Vec3.h"

#include <random>
#include <math.h>

Vec3<float> Rnd::in_sphere() {
    float z = uniform(-1.0, 1.0);
    float phi = uniform(0.0, 2 * M_PI);
    float sq = sqrt(1 - z * z);
    return {sq * cos(phi), sq * sin(phi), z};
}

float Rnd::uniform(float Min, float Max) {
    std::uniform_real_distribution<float> dis(Min, Max);
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

bool Rnd::bernoulli(float succ_rate) {
    std::bernoulli_distribution dis(succ_rate);
    return dis(rnd);
}

Rnd *Rnd::instance = nullptr;
