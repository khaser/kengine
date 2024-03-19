#include <iostream>

#include "Image.h"
#include "Scene.h"

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: kengine <path to scene> <path for output p6 image>\n";
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

    /* const int samples = 1'000'000; */
    /* BoxDistribution dist = { std::dynamic_pointer_cast<Box>(scene.objs[5].geometry) }; */
    /* Vec3<double> pos = {0, 0, 15}; */
    /* Vec3<double> n = {0, 0, -1}; */
    /* UniformDistribution uniform; */
    /* std::cerr << dist.box->size << ' ' << dist.faces_square << std::endl; */
    /* double res = 0; */
    /* for (int i = 0; i < samples; ++i) { */
    /*     auto d = uniform.sample(pos, n); */
    /*     auto p = dist.pdf(pos, n, d) ; */
    /*     res += p / uniform.pdf(pos, n, d); */
    /* } */
    /* res /= samples; */
    /* std::cerr << res << '\n'; */


    return 0;
}
