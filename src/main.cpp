#include <iostream>

#include "Image.h"
#include "Scene.h"
#include "SceneBuilder.h"

int main(int argc, char* argv[]) {
    /* std::cout << "Usage: kengine <path to scene> <width> <height> <spp> <path for output p6 image>\n"; */

    std::filesystem::path scene_path(argv[1]);
    std::string output_path(argv[argc-1]);

    SceneBuilder builder;
    std::ifstream fin(scene_path);
    Setup setup = {6, uint16_t(std::atoi(argv[4])), Vec3<float>(), Vec3<float>(),
                    {uint16_t(std::atoi(argv[2])), uint16_t(std::atoi(argv[3]))}};
    builder = GltfBuilder(fin, scene_path.parent_path(), std::move(setup));
    Scene scene(std::move(builder));
    std::cerr << "Scene parsed\n";

    Image img = scene.render_scene();
    std::cerr << "Scene rendered\n";
    img.write_ppm(std::ofstream(output_path));
    std::cerr << "Image dumped to " << output_path << '\n';

    return 0;
}
