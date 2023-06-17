#include <boost/algorithm/string/predicate.hpp>

#include "log.hpp"

inline LogLevel LogLevelFromString(std::string str) {
    #define check_string(level, lower, stream) if (boost::iequals(str, #lower)) return LogLevel::level;
    FOR_EACH_LOG_LEVEL(check_string)
    #undef check_string

    throw std::invalid_argument("Invalid log level: " + str);
}

void Logger::initialize(std::string log_level_string) {
    LogLevel log_level = LogLevelFromString(log_level_string);
    #define initialize_log(level, lower, stream) lower = log_level <= LogLevel::level ? Log(&stream, "["#level"] ") : Log();
    FOR_EACH_LOG_LEVEL(initialize_log)
    #undef initialize_log
}

Logger logger;
