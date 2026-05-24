#pragma once

class Log {
public:
    static void init();
    static void info(const char* msg);
    static void warn(const char* msg);
    static void error(const char* msg);
};
