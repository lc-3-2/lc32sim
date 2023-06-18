#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <fstream>
#include <iostream>

#include "config.hpp"
#include "exceptions.hpp"
#include "log.hpp"

namespace lc32sim {
    // `Const_instance` is modifiable, so it should generally not be used directly
    // `Config` is a const reference and is preferable for anyone who is reading config values
    class Config config_instance;
    const class Config &Config = config_instance;

    Config::Config() {
        if (this != &config_instance) {
            throw SimulatorException("Config should not be instantiated directly");
        }
    }

    void Config::load_config(argparse::ArgumentParser program) {
        // Check for existence of config file
        std::ifstream config_file(program.get<std::string>("--config-file"));
        std::string config_message = "No config file found, using defaults";

        if (config_file.good()) {
            boost::property_tree::ptree data;
            boost::property_tree::read_json(config_file, data);

            try {
                #define get_config(name, description) this->name = data.get<decltype(name)>(#name, this->name);
                FOR_EACH_CONFIG_OPTION(get_config);
                #undef get_config
            } catch (boost::property_tree::json_parser::json_parser_error &e) {
                throw SimulatorException("JSON parsing error: " + std::string(e.what()));
            }

            config_message = "Config loaded";
        }

        // Certain command line options can override config file options
        using std::literals::string_literals::operator""s;
        if (program["--log-level"] != "use-config"s) {
            this->log_level = program.get<std::string>("--log-level");
        }
        if (program["--software-rendering"] == true) {
            this->display.accelerated_rendering = false;
        }

        try {
            logger.initialize(log_level);
        } catch (std::invalid_argument &e) {
            logger.initialize(DEFAULT_LOG_LEVEL);
            logger.error << "Invalid log level: " << log_level << ". Using default (" << DEFAULT_LOG_LEVEL << ")";
        }

        logger.info << config_message;
        #define log_config(name, description) logger.info << std::boolalpha << "    " << description << ": " << name;
        FOR_EACH_CONFIG_OPTION(log_config);
        #undef log_config
    }
}
