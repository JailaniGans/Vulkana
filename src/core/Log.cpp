#include "core/Log.hpp"
#include <cstdio>
#include <ctime>
#include <windows.h>

#define WARNA_DEFAULT 7
#define WARNA_CYAN    3
#define WARNA_KUNING  6
#define WARNA_MERAH   4

static HANDLE s_handleKonsol = nullptr;

void Log::init() {
    s_handleKonsol = GetStdHandle(STD_OUTPUT_HANDLE);
    Log::info("Logger diinisialisasi");
}

void Log::info(const char* msg) {
    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_s(&timeinfo, &now);
    char bufWaktu[16];
    strftime(bufWaktu, sizeof(bufWaktu), "%H:%M:%S", &timeinfo);
    SetConsoleTextAttribute(s_handleKonsol, WARNA_CYAN);
    printf("[%s] ", bufWaktu);
    SetConsoleTextAttribute(s_handleKonsol, WARNA_DEFAULT);
    printf("INFO: %s\n", msg);
}

void Log::warn(const char* msg) {
    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_s(&timeinfo, &now);
    char bufWaktu[16];
    strftime(bufWaktu, sizeof(bufWaktu), "%H:%M:%S", &timeinfo);
    SetConsoleTextAttribute(s_handleKonsol, WARNA_KUNING);
    printf("[%s] ", bufWaktu);
    SetConsoleTextAttribute(s_handleKonsol, WARNA_DEFAULT);
    printf("WARN: %s\n", msg);
}

void Log::error(const char* msg) {
    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_s(&timeinfo, &now);
    char bufWaktu[16];
    strftime(bufWaktu, sizeof(bufWaktu), "%H:%M:%S", &timeinfo);
    SetConsoleTextAttribute(s_handleKonsol, WARNA_MERAH);
    printf("[%s] ", bufWaktu);
    SetConsoleTextAttribute(s_handleKonsol, WARNA_DEFAULT);
    printf("ERROR: %s\n", msg);
}
