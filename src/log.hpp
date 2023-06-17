#pragma once

#include <iostream>
#include <string>
#include <utility>

#include "config.hpp"

enum class LogLevel : size_t {
    DEBUG, INFO, WARN, ERROR, FATAL,
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
        Log debug, info, warn, error, fatal;
        void initialize(std::string log_level_string);
        Logger() = default;

};
extern Logger logger;
