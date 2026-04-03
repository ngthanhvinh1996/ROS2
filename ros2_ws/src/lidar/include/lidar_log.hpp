// lidar_log.hpp
#ifndef LIDAR_LOG_HPP_
#define LIDAR_LOG_HPP_

#include <cstdio>
#include <cstdarg>
#include <string>
#include <functional>

namespace lidar {

using LogCallback = std::function<void(const std::string&)>;

class Logger {
public:
    static void setInfoCallback(LogCallback cb)  { info_cb_  = cb; }
    static void setWarnCallback(LogCallback cb)  { warn_cb_  = cb; }
    static void setErrorCallback(LogCallback cb) { error_cb_ = cb; }

    static void info(const char* fmt, ...) {
        if (!info_cb_) return;
        char buf[512];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);
        info_cb_(buf);
    }

    static void warn(const char* fmt, ...) {
        if (!warn_cb_) return;
        char buf[512];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);
        warn_cb_(buf);
    }

    static void error(const char* fmt, ...) {
        if (!error_cb_) return;
        char buf[512];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);
        error_cb_(buf);
    }

private:
    static inline LogCallback info_cb_  = nullptr;
    static inline LogCallback warn_cb_  = nullptr;
    static inline LogCallback error_cb_ = nullptr;
};

}  // namespace lidar

#define LOG_INFO(fmt, ...)  lidar::Logger::info(fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  lidar::Logger::warn(fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) lidar::Logger::error(fmt, ##__VA_ARGS__)

#endif
