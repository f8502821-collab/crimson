#pragma once
// CRIMSON shared logger: file + OutputDebugString, thread-safe.
#include <cstdio>
#include <cstdarg>
#include <string>
#include <mutex>

namespace clog {

enum class Level : int { Debug = 0, Info = 1, Warn = 2, Error = 3 };

// Initialize with a full path; pass nullptr to skip file logging.
bool init(const char* file_path);
void shutdown();

void vlog(Level lvl, const char* tag, const char* fmt, va_list args);
void log(Level lvl, const char* tag, const char* fmt, ...);

inline void debug(const char* tag, const char* fmt, ...) {
    va_list a; va_start(a, fmt); vlog(Level::Debug, tag, fmt, a); va_end(a);
}
inline void info(const char* tag, const char* fmt, ...) {
    va_list a; va_start(a, fmt); vlog(Level::Info, tag, fmt, a); va_end(a);
}
inline void warn(const char* tag, const char* fmt, ...) {
    va_list a; va_start(a, fmt); vlog(Level::Warn, tag, fmt, a); va_end(a);
}
inline void error(const char* tag, const char* fmt, ...) {
    va_list a; va_start(a, fmt); vlog(Level::Error, tag, fmt, a); va_end(a);
}

} // namespace clog
