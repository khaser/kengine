#pragma once

#include <vector>
#include <memory>
#include <fstream>
#include <cassert>
#include <cmath>

#include "Vec3.h"

struct Image {
    uint16_t width, height;
    std::vector<uint8_t> pixels;

    Image(int w, int h, const std::vector<Vec3<double>> &data) : width(w), height(h) {
        assert(data.size() >= w * h);
        pixels.reserve(w * h * 3);
        for (auto pixel : data) {
            pixels.emplace_back(round(255 * pixel.x));
            pixels.emplace_back(round(255 * pixel.y));
            pixels.emplace_back(round(255 * pixel.z));
        }
    };

    void write_ppm(std::ofstream os) {
        os << "P6\n";
        os << width << ' ' << height << '\n';
        os << "255\n";
        os.write((char*)pixels.data(), width * height * 3);
    }
};
