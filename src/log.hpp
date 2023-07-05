#pragma once

#include <iostream>
#include <string>
#include <utility>

#include "config.hpp"

// Three pieces of information used to define each log level:
// (1) name of the log level (used in enum and as prefix for log messages)
// (2) name of the variable used to access the log level (e.g. `logger.debug`)
// (3) stream to which log messages should be written
#define FOR_EACH_LOG_LEVEL(X) \
    X(TRACE, trace, std::cout) \
    X(DEBUG, debug, std::cout) \
    X(INFO, info, std::cout) \
    X(WARN, warn, std::cout) \
    X(ERROR, error, std::cerr) \
    X(FATAL, fatal, std::cerr)

namespace lc32sim {
    enum class LogLevel {
        #define initialize_enum(level, lower, stream) level,
        FOR_EACH_LOG_LEVEL(initialize_enum)
        #undef initialize_enum
        NUM_LOG_LEVELS
    };
    inline bool operator<=(LogLevel a, LogLevel b) {
        return std::to_underlying(a) <= std::to_underlying(b);
    }

    class Line {
        private:
            std::ostream *out;
            std::ios_base::fmtflags flags;
        public:
            Line(std::ostream *out, std::ios_base::fmtflags flags) : out(out), flags(flags) {}
            ~Line() {
                if (out) {
                    *out << std::endl;
                    out->flags(flags);
                }
            }
            template<typename T> Line &operator<<(T t) {
                if (out) {
                    *out << t;
                }
                return *this;
            }
    };
    class Log {
        private:
            std::ostream *out;
            std::string prefix;
        public:
            Log() : out(nullptr), prefix("") {}
            Log(std::string prefix) : out(nullptr), prefix(prefix) {}
            Log(std::ostream *out) : out(out), prefix("") {}
            Log(std::ostream *out, std::string prefix) : out(out), prefix(prefix) {}
            bool enabled() {
                return out != nullptr;
            }
            template<typename T> Line operator<<(T t) {
                if (!out) {
                    return Line(nullptr, std::ios_base::fmtflags());
                }
                std::ios_base::fmtflags flags = out->flags();
                *out << prefix << t;
                return Line(out, flags);
            }
            friend class Logger;
    };

    class Logger {
        public:
            #define declare_log(level, lower, stream) Log lower;
            FOR_EACH_LOG_LEVEL(declare_log)
            #undef declare_log

            void initialize(std::string log_level_string);
            Logger();

    };
    extern Logger logger;
}