#pragma once

#include <vector>
#include <memory>
#include <fstream>
#include <cassert>
#include <cmath>

#include "Primitives/Vec3.h"

struct Image {
    uint16_t width, height;
    std::vector<uint8_t> pixels;

    Image(const std::vector<std::vector<Vec3<float>>> &img) : height(img.size()), width(img[0].size()) {
        pixels.reserve(width * height * 3);
        for (auto &row : img) {
            for (auto &pixel : row) {
                pixels.emplace_back(round(255 * pixel.x));
                pixels.emplace_back(round(255 * pixel.y));
                pixels.emplace_back(round(255 * pixel.z));
            }
        }
    };

    void write_ppm(std::ofstream os) {
        os << "P6\n";
        os << width << ' ' << height << '\n';
        os << "255\n";
        os.write((char*)pixels.data(), width * height * 3);
    }
};
