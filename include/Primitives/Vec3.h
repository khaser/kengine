#pragma once

#include <fstream>
#include <cmath>
#include <algorithm>

const double EPS = 1e-9;

template<typename T>
struct Vec3 {
    T x, y, z;

    explicit Vec3() : x(0), y(0), z(0) {}
    explicit Vec3(const T& k) : x(k), y(k), z(k) {}
    Vec3(const T& x, const T& y, const T& z) : x(x), y(y), z(z) { }

    bool operator==(const Vec3<T> &oth) const {
        return fabs(x - oth.x) < EPS && fabs(y - oth.y) < EPS && fabs(z - oth.z) < EPS;
    }
    bool operator!=(const Vec3<T> &oth) const {
        return fabs(x - oth.x) > EPS || fabs(y - oth.y) > EPS || fabs(z - oth.z) > EPS;
    }
    Vec3<T> operator*(const T& k) const {
        return {x * k, y * k, z * k};
    }
    Vec3<T> operator/(const T& k) const {
        return {x / k, y / k, z / k};
    }
    Vec3<T> operator+(const T& k) const {
        return {x + k, y + k, z + k};
    }
    Vec3<T> operator*(const Vec3<T>& k) const {
        return {x * k.x, y * k.y, z * k.z};
    }
    Vec3<T> operator/(const Vec3<T>& k) const {
        return {x / k.x, y / k.y, z / k.z};
    }
    T operator%(const Vec3<T>& oth) const { // dot-product
        return x * oth.x + y * oth.y + z * oth.z;
    }
    Vec3<T> operator^(const Vec3<T>& oth) const { // cross-product
        return {y * oth.z - z * oth.y, z * oth.x - x * oth.z, x * oth.y - y * oth.x};
    }
    Vec3<T> operator+(const Vec3<T>& oth) const {
        return {x + oth.x, y + oth.y, z + oth.z};
    }
    Vec3<T> operator-(const Vec3<T>& oth) const {
        return {x - oth.x, y - oth.y, z - oth.z};
    }
    Vec3<T> operator-() const {
        return {-x, -y, -z};
    }
    double len() const {
        return sqrt(x * x + y * y + z * z);
    }
    Vec3<T> norm() const {
        return (*this) * (1.0 / len());
    }

    Vec3<T> clamp(const Vec3<T> &Min, const Vec3<T> &Max) const {
        return { std::clamp(x, Min.x, Max.x), std::clamp(y, Min.y, Max.y), std::clamp(z, Min.z, Max.z) };
    }
};

template<typename T>
Vec3<T> min(const Vec3<T> &a, const Vec3<T> &b) {
    return { std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z) };
}
template<typename T>
Vec3<T> max(const Vec3<T> &a, const Vec3<T> &b) {
    return { std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z) };
}

template<typename T>
std::istream& operator>>(std::istream &is, Vec3<T> &el) {
    return is >> el.x >> el.y >> el.z;
}
template<typename T>
std::ostream& operator<<(std::ostream &os, const Vec3<T> &el) {
    return os << '(' << el.x << ' ' << el.y << ' ' << el.z << ')';
}

template<typename T>
Vec3<T> pow(const Vec3<T> &v, double p) {
    return { std::pow(v.x, p), std::pow(v.y, p), std::pow(v.z, p) };
}
