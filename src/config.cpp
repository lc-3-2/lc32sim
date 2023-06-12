#include <fstream>
#include <iostream>

#include "config.hpp"
#include "exceptions.hpp"

namespace lc32sim {
    // `instance` is modifiable, but only available in this file
    // `config` is const and available everywhere that includes config.hpp
    Config instance;
    const Config &config = instance;

    Config::Config() {
        if (this != &instance) {
            throw SimulatorException("Config should not be instantiated directly");
        }

        // Check for existence of config file
        std::ifstream config_file(CONFIG_FILE_NAME);
        if (!config_file.good()) {
            std::cout << "No config file found" << std::endl;
        }
        this->  data = json::parse(config_file);
        this->display_width = data["display"]["width"];
        std::cout << "Config file loaded" << std::endl;
    }
}