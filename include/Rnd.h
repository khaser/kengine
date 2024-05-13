#pragma once

#include <random>
#include "Primitives/Vec3.h"

class Rnd {
private:
    static Rnd *instance;
#pragma omp threadprivate(instance)
    Rnd() {}
    Rnd(const Rnd&);
    Rnd& operator=(Rnd&);

    std::minstd_rand rnd;

public:
    static Rnd* getRnd();

    Vec3<float> in_sphere();

    float uniform(float Min, float Max);

    bool bernoulli(float success_rate = 0.5);

    int uniform_int(int Min, int Max);
};
