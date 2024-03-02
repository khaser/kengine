#include <iostream>
#include <fstream>
#include <string>
#include <fstream>
#include <vector>
#include <memory>
#include <optional>
#include <cmath>
#include <algorithm>
#include <cassert>

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

struct Quaternion {
    Vec3<double> v = {0, 0, 0};
    double w = 1;
    Quaternion operator*(const Quaternion& oth) const {
        return {v * oth.w + oth.v * w + (v ^ oth.v), w * oth.w - v % oth.v};
    }
    Quaternion operator-() const {
        return {-v, w};
    }
};

template<typename T>
Vec3<T> operator*(const Quaternion &q, const Vec3<T> &v) {
    Quaternion res = q * (Quaternion {v, 0}) * -q;
    /* std::cerr << v << ' ' << res.v << ' ' << res.w << std::endl; */
    return res.v;
}

std::istream& operator >> (std::istream &is, Quaternion &el) {
    return is >> el.v >> el.w;
}

struct Ray {
    Vec3<double> start;
    Vec3<double> v; // normalized ray direction vector

    Vec3<double> reveal(double t) const {
        return start + v * t;
    }
};

struct Camera {
    Vec3<double> position;
    Vec3<double> right;
    Vec3<double> up;
    Vec3<double> forward;
    double fov_x;
    double fov_y;
    void calc_fov_y(int w, int h) {
        fov_y = 2 * atan(h / (w * tan(fov_x/2)));
    }

    // x, y in [-1, 1]
    Ray raycast(double x, double y) {
        Vec3<double> v = {x * tan(fov_x / 2), y * tan(fov_y / 2), 1};
        Vec3<double> v_rotated = forward * v.z + up * v.y + right * v.x;
        return Ray {position, v_rotated.norm()};
    }
};

struct Geometry {
    virtual std::optional<double> get_intersect(const Ray& ray) = 0;
};

struct Plane : public Geometry {
    Vec3<double> norm;
    Plane(const Vec3<double> &v) : norm(v.norm()) {}
    std::optional<double> get_intersect(const Ray& ray) {
        double t = -(ray.start % norm) / (ray.v % norm);
        return (t > 0 ? std::optional<double>(t) : std::nullopt);
    };
};

struct Ellipsoid : public Geometry {
    Vec3<double> r;
    Ellipsoid(const Vec3<double> &v) : r(v) {}
    std::optional<double> get_intersect(const Ray& ray) {
        // start % start + 2 * t * (start % v) + t * t * (v % v) = R * R
        double a = (ray.v / r) % (ray.v / r);
        double b = ((ray.start / r) % (ray.v / r)) * 2;
        double c = (ray.start / r) % (ray.start / r) - 1;
        double D = b * b - 4 * a * c;
        if (D < 0) return {};
        double t1 = (-b - sqrt(D)) / (2 * a);
        double t2 = (-b + sqrt(D)) / (2 * a);
        if (t1 > 0) {
            return t1;
        } else if (t2 > 0) {
            return t2;
        } else return {};
    };
};

struct Box : public Geometry {
    Vec3<double> size;
    Box(const Vec3<double> &v) : size(v) {}
    std::optional<double> get_intersect(const Ray& ray) {
        Vec3<double> t1v = (size - ray.start) / ray.v;
        Vec3<double> t2v = (-size - ray.start) / ray.v;
        Vec3<double> tmin = min(t1v, t2v);
        Vec3<double> tmax = max(t1v, t2v);
        auto t1 = std::max({tmin.x, tmin.y, tmin.z});
        auto t2 = std::min({tmax.x, tmax.y, tmax.z});
        if (t1 > t2) {
            return {};
        } else {
            return std::min(t1, t2);
        }
    };
};

struct Primitive {
    std::unique_ptr<Geometry> geom;
    Quaternion rotation;
    Vec3<double> position;
    Vec3<double> color;

    std::optional<double> get_intersect(Ray ray) const {
        ray.start = -rotation * (ray.start - position);
        ray.v = -rotation * ray.v;
        std::optional<double> t = geom->get_intersect(ray);
        return t;
    }
};

struct Image {
    uint16_t width, height;
    std::vector<uint8_t> pixels;

    Image(int w, int h, const std::vector<Vec3<double>> &data) : width(w), height(h) {
        assert(data.size() >= w * h);
        pixels.reserve(w * h * 3);
        for (auto pixel : data) {
            pixels.emplace_back(255 * pixel.x);
            pixels.emplace_back(255 * pixel.y);
            pixels.emplace_back(255 * pixel.z);
        }
    };

    void write_ppm(std::ofstream os) {
        os << "P6\n";
        os << width << ' ' << height << '\n';
        os << "255\n";
        os.write((char*)pixels.data(), width * height * 3);
    }
};

struct Scene {
    std::pair<uint16_t, uint16_t> dimensions;
    Vec3<double> bg_color;
    Camera camera;
    std::vector<Primitive> objs;

    Scene(std::ifstream is) {
        std::string token;
        while (is >> token) {
            if (token == "DIMENSIONS") {
                is >> dimensions.first >> dimensions.second;
            } else if (token == "BG_COLOR") {
                is >> bg_color;
            } else if (token == "CAMERA_POSITION") {
                is >> camera.position;
            } else if (token == "CAMERA_RIGHT") {
                is >> camera.right;
            } else if (token == "CAMERA_UP") {
                is >> camera.up;
            } else if (token == "CAMERA_FORWARD") {
                is >> camera.forward;
            } else if (token == "CAMERA_FOV_X") {
                is >> camera.fov_x;
                camera.calc_fov_y(dimensions.first, dimensions.second);
            } else if (token == "NEW_PRIMITIVE") {
                objs.emplace_back();
            } else if (token == "POSITION") {
                is >> objs.back().position;
            } else if (token == "ROTATION") {
                is >> objs.back().rotation;
            } else if (token == "COLOR") {
                is >> objs.back().color;
            } else if (token == "PLANE") {
                Vec3<double> norm;
                is >> norm;
                objs.back().geom = std::unique_ptr<Geometry>(new Plane(norm));
            } else if (token == "ELLIPSOID") {
                Vec3<double> norm;
                is >> norm;
                objs.back().geom = std::unique_ptr<Geometry>(new Ellipsoid(norm));
            } else if (token == "BOX") {
                Vec3<double> norm;
                is >> norm;
                objs.back().geom = std::unique_ptr<Geometry>(new Box(norm));
            } else if (token == "NEW_PRIMITIVE") {
                objs.emplace_back();
            } else {
                std::cerr << "Unknown token: " << token << std::endl;
            }
        }
    }

    Image render_scene() {
        std::vector<Vec3<double>> output(dimensions.first * dimensions.second * 3);
        for (uint16_t x = 0; x < dimensions.first; ++x) {
            for (uint16_t y = 0; y < dimensions.second; ++y) {
                double x_01 = (x + 0.5) / dimensions.first;
                double y_01 = (y + 0.5) / dimensions.second;
                output[y * dimensions.first + x] = render_pixel(x_01 * 2 - 1, -(y_01 * 2 - 1));
            }
        }
        return Image(dimensions.first, dimensions.second, output);
    }

private:
    // x, y in [-1, 1]
    Vec3<double> render_pixel(double x, double y) {
        Ray ray = camera.raycast(x, y);
        std::pair<Vec3<double>, double> bound = {bg_color, 1e9};

        for (auto& obj : objs) {
            auto t = obj.get_intersect(ray);
            if (t && bound.second > *t) {
                bound = {obj.color, *t};
            }
        }

        return bound.first;
    }
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: kengine <path to scene> <path for output p6 image>";
        return 1;
    }
    std::string scene_path(argv[1]);
    std::string output_path(argv[2]);

    Scene scene = std::ifstream(scene_path);
    std::cerr << "Scene parsed\n";

    Image img = scene.render_scene();
    std::cerr << "Scene rendered\n";

    img.write_ppm(std::ofstream(output_path));
    std::cerr << "Image dumped to " << output_path << '\n';

    return 0;
}
