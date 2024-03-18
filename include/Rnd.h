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

    Vec3<double> in_hemisphere(Vec3<double> v);

    Vec3<double> in_sphere();

    double uniform(double Min, double Max);
};
