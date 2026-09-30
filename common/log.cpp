#include "common/log.h"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <chrono>
#include <ctime>
#include <cstring>

namespace clog {

static FILE* s_file = nullptr;
static std::mutex s_mtx;

static const char* lvl_name(Level l) {
    switch (l) {
    case Level::Debug: return "DBG ";
    case Level::Info:  return "INFO";
    case Level::Warn:  return "WARN";
    case Level::Error: return "ERR ";
    }
    return "??? ";
}

bool init(const char* file_path) {
    std::lock_guard<std::mutex> g(s_mtx);
    if (!file_path) return false;
    if (s_file) return true;
    if (fopen_s(&s_file, file_path, "w") != 0 || !s_file) {
        s_file = nullptr;
        return false;
    }
    return true;
}

void shutdown() {
    std::lock_guard<std::mutex> g(s_mtx);
    if (s_file) { fclose(s_file); s_file = nullptr; }
}

void vlog(Level lvl, const char* tag, const char* fmt, va_list args) {
    char msg[2048];
    vsnprintf_s(msg, sizeof(msg), _TRUNCATE, fmt, args);

    char tbuf[32];
    using clock = std::chrono::system_clock;
    std::time_t t = clock::to_time_t(clock::now());
    struct tm lt;
    localtime_s(&lt, &t);
    strftime(tbuf, sizeof(tbuf), "%H:%M:%S", &lt);

    std::lock_guard<std::mutex> g(s_mtx);
    char line[2304];
    _snprintf_s(line, sizeof(line), _TRUNCATE, "[%s][%s][%s] %s\n", tbuf, lvl_name(lvl), tag, msg);
    OutputDebugStringA(line);
    if (s_file) { fputs(line, s_file); fflush(s_file); }
}

void log(Level lvl, const char* tag, const char* fmt, ...) {
    va_list a; va_start(a, fmt);
    vlog(lvl, tag, fmt, a);
    va_end(a);
}

} // namespace clog
