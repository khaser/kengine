#include "Primitives/Quaternion.h"

#include <fstream>

std::istream& operator>>(std::istream &is, Quaternion &el) {
    return is >> el.v >> el.w;
}
