#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <fstream>
#include <iostream>

#include "config.hpp"
#include "exceptions.hpp"

namespace lc32sim {
    // `instance` is modifiable, but only available in this file
    // `Config` is const and available everywhere that includes config.hpp
    class Config instance;
    const class Config &Config = instance;

    #define get_config(name, type) this->name = data.get<type>(#name, this->name)
    Config::Config() {
        if (this != &instance) {
            throw SimulatorException("Config should not be instantiated directly");
        }

        // Check for existence of config file
        std::ifstream config_file(CONFIG_FILE_NAME);
        if (!config_file.good()) {
            std::cout << "No config file found, using defaults" << std::endl;

        } else {
            boost::property_tree::ptree data;
            boost::property_tree::read_json(config_file, data);

            try {
                get_config(display.width, int);
                get_config(display.height, int);
                get_config(display.hblank_length, int);
                get_config(display.vblank_length, int);
                get_config(display.frames_per_second, double);
                get_config(display.accelerated_rendering, bool);
                get_config(allow_unaligned_access, bool);
            } catch (boost::property_tree::json_parser::json_parser_error &e) {
                throw SimulatorException("JSON parsing error: " + std::string(e.what()));
            }

            std::cout << "Config loaded" << std::endl;
        }
        std::cout << "\tDisplay width: " << display.width << std::endl;
        std::cout << "\tDisplay height: " << display.height << std::endl;
        std::cout << "\tDisplay hblank length: " << display.hblank_length << std::endl;
        std::cout << "\tDisplay vblank length: " << display.vblank_length << std::endl;
        std::cout << "\tDisplay frames per second: " << display.frames_per_second << std::endl;
        std::cout << "\tDisplay accelerated rendering: " << (display.accelerated_rendering ? "true" : "false") << std::endl;
        std::cout << "\tAllow unaligned access: " << (allow_unaligned_access ? "true" : "false") << std::endl;
    }
}
