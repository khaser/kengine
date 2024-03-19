#pragma once

#include <random>
#include "Primitives/Vec3.h"

class Rnd {
private:
    static Rnd *instance;
    Rnd() {}
    Rnd(const Rnd&);
    Rnd& operator=(Rnd&);

    std::minstd_rand rnd;

public:
    static Rnd* getRnd();

    Vec3<double> in_sphere();

    double uniform(double Min, double Max);

    bool bernoulli(double success_rate = 0.5);

    int uniform_int(int Min, int Max);
};
