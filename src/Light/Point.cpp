#include "Light.h"
#include "Vec3.h"

Vec3<double> PointLight::get_irradiance(const Vec3<double>& point) {
    double r = (point - position).len();
    return intensity * (1.0 / (attenuation[0] + attenuation[1] * r + attenuation[2] * r * r));
}
