#include <boost/algorithm/string/predicate.hpp>

#include "log.hpp"

inline LogLevel LogLevelFromString(std::string str) {
    if (boost::iequals(str, "debug")) return LogLevel::DEBUG;
    if (boost::iequals(str, "info")) return LogLevel::INFO;
    if (boost::iequals(str, "warn")) return LogLevel::WARN;
    if (boost::iequals(str, "error")) return LogLevel::ERROR;
    if (boost::iequals(str, "fatal")) return LogLevel::FATAL;

    throw std::invalid_argument("Invalid log level: " + str);
}

void Logger::initialize(std::string log_level_string) {
    LogLevel log_level = LogLevelFromString(log_level_string);
    debug.prefix = "[DEBUG] ";
    info.prefix = "[INFO] ";
    warn.prefix = "[WARN] ";
    error.prefix = "[ERROR] ";
    fatal.prefix = "[FATAL] ";

    debug.out = log_level <= LogLevel::DEBUG ? &std::cout : nullptr;
    info.out = log_level <= LogLevel::INFO ? &std::cout : nullptr;
    warn.out = log_level <= LogLevel::WARN ? &std::cout : nullptr;
    error.out = log_level <= LogLevel::ERROR ? &std::cerr : nullptr;
    fatal.out = log_level <= LogLevel::FATAL ? &std::cerr : nullptr;
}

Logger logger;
