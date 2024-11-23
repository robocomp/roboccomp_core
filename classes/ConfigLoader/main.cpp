#include "ConfigLoader.h"
#include <iostream>

int main(int argc, char* argv[]) {
    try {
        if (argc != 2) {
            std::cerr << "Usage: " << argv[0] << " <config_file_path>" << std::endl;
            return 1;
        }

        // Get the configuration file path from the command-line arguments
        std::string configFilePath = argv[1];

        ConfigLoader loader;
        loader.load(configFilePath);
        loader.printConfig();

        std::cout << "\n\n\n\nCompute Period: " << loader.get<int>("Period.Compute") << std::endl;
        std::cout << "Lidar3D.Endpoints: " << loader.get<std::string>("Lidar3D.Endpoints") << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
