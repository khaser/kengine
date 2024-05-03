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

    return 0;
}
