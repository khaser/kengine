#include "Rnd.h"
#include "Primitives/Vec3.h"

#include <random>
#include <math.h>

Vec3<float> Rnd::in_sphere() {
    while (true) {
        Vec3<float> v = {uniform(-1.0, 1.0), uniform(-1.0, 1.0), uniform(-1.0, 1.0)};
        if (v.len() < 1) {
            return v.norm();
        }
    }
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
