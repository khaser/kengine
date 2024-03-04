#include <array>
#include <memory>
#include <iostream>
#include <optional>
#include <stdexcept>

#include "Light.h"
#include "Vec3.h"

std::unique_ptr<Light> Light::from_istream(std::istream& is) {
    std::string token;

    std::optional<Vec3<double>> intensity;
    std::optional<Vec3<double>> direction;
    std::optional<Vec3<double>> position;
    std::optional<Attenuation> attenuation;

    while (is >> token) {
        if (token == "LIGHT_INTENSITY") {
            Vec3<double> i;
            is >> i;
            intensity = i;
        } else if (token == "LIGHT_DIRECTION") {
            Vec3<double> dir;
            is >> dir;
            direction = dir;
        } else if (token == "LIGHT_POSITION") {
            Vec3<double> pos;
            is >> pos;
            position = pos;
        } else if (token == "LIGHT_ATTENUATION") {
            Attenuation att;
            is >> att[0] >> att[1] >> att[2];
            attenuation = att;
        } else {
            throw std::invalid_argument("failed to build Light class");
        }

        if (intensity && direction) {
            return std::unique_ptr<Light>(reinterpret_cast<Light*>(new DirectLight{intensity.value(), direction.value()}));
        } else if (intensity && position && attenuation) {
            return std::unique_ptr<Light>(reinterpret_cast<Light*>(new PointLight{intensity.value(), position.value(), attenuation.value()}));
        }
    }
    throw std::invalid_argument("failed to build Light class");
}
