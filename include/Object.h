#pragma once

#include "Object/Material.h"
#include "Object/Geometry.h"

#include <memory>

struct Object {
    std::shared_ptr<Material> material;
    std::shared_ptr<Geometry> geometry;
};
