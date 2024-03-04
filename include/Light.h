#pragma once

#include <array>
#include <memory>

#include "Vec3.h"

using Attenuation = std::array<double, 3>;

struct Light {
    Vec3<double> intensity;

    static std::unique_ptr<Light> from_istream(std::istream&);

    virtual double get_irradiance(const Vec3<double>&) = 0;
};

struct PointLight : public Light {
    Vec3<double> position;
    Attenuation attenuation;

    PointLight(const Vec3<double>& intensity, const Vec3<double>& position, const Attenuation& attenuation)
        : position(position), attenuation(attenuation) {
            this->intensity = intensity;
        }

    double get_irradiance(const Vec3<double>&);
};

struct DirectLight : public Light {
    Vec3<double> direction;

    DirectLight(const Vec3<double>& intensity, const Vec3<double>& direction) : direction(direction) {
        this->intensity = intensity;
    };

    double get_irradiance(const Vec3<double>&);
};
