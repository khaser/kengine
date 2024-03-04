#include "Light.h"
#include "Vec3.h"

Vec3<double> DirectLight::get_irradiance(const Vec3<double>& point) {
    return intensity;
}

