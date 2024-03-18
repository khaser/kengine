#include "Quaternion.h"
#include "Vec3.h"

#include <fstream>

std::istream& operator>>(std::istream &is, Quaternion &el) {
    return is >> el.v >> el.w;
}
