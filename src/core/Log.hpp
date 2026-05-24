#pragma once

// Logger - menulis pesan dengan timestamp ke stdout
//   LOG_INFO("pesan")   -> [INFO]  12:34:56 pesan
//   LOG_WARN("pesan")   -> [WARN]  12:34:56 pesan
//   LOG_ERROR("pesan")  -> [ERR]   12:34:56 pesan

#include <iostream>
#include <chrono>
#include <string_view>
#include <iomanip>
#include <ctime>

#define LOG_INFO(msg)    Vulkana::Log::write(Vulkana::Log::Level::Info, msg)
#define LOG_WARN(msg)    Vulkana::Log::write(Vulkana::Log::Level::Warn, msg)
#define LOG_ERROR(msg)   Vulkana::Log::write(Vulkana::Log::Level::Error, msg)

namespace Vulkana {

class Log {
public:
    enum class Level { Info, Warn, Error };

    static void write(Level level, std::string_view message)
    {
        auto now = std::chrono::system_clock::now();
        auto t = std::chrono::system_clock::to_time_t(now);
        std::tm tm;
        localtime_s(&tm, &t);

        const char* prefix = "";
        switch (level) {
            case Level::Info:  prefix = "[INFO]";  break;
            case Level::Warn:  prefix = "[WARN]";  break;
            case Level::Error: prefix = "[ERR]";   break;
        }

        std::cout << prefix << " "
                  << std::put_time(&tm, "%H:%M:%S")
                  << " " << message << std::endl;
    }
};

}
