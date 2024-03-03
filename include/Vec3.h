#pragma once

#include <fstream>

template<typename T>
struct Vec3 {
    T x, y, z;
    Vec3<T> operator*(const T& k) const {
        return {x * k, y * k, z * k};
    }
    Vec3<T> operator*(const Vec3<T>& k) const {
        return {x * k.x, y * k.y, z * k.z};
    }
    Vec3<T> operator/(const Vec3<T>& k) const {
        return {x / k.x, y / k.y, z / k.z};
    }
    T operator%(const Vec3<T>& oth) const { //dot-product
        return x * oth.x + y * oth.y + z * oth.z;
    }
    Vec3<T> operator^(const Vec3<T>& oth) const { //cross-product
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
        return operator*(1.0 / len());
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
std::istream& operator >> (std::istream &is, Vec3<T> &el) {
    return is >> el.x >> el.y >> el.z;
}
template<typename T>
std::ostream& operator << (std::ostream &os, const Vec3<T> &el) {
    return os << '(' << el.x << ' ' << el.y << ' ' << el.z << ')';
}

