#pragma once

/**
 * Log - Utilitas logging konsol dengan warna.
 * Level: info (cyan), warn (kuning), error (merah).
 */
class Log {
public:
    static void init();
    static void info(const char* msg);
    static void warn(const char* msg);
    static void error(const char* msg);
};
