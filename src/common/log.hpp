#pragma once
#include <cstdio>
#include <cstdarg>

namespace Log {

inline void Info(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stdout, "[INFO] ");
    vfprintf(stdout, fmt, args);
    fprintf(stdout, "\n");
    va_end(args);
}

inline void Warn(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "[WARN] ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
}

inline void Error(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "[ERROR] ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
}

inline void Debug(const char* fmt, ...) {
#ifdef DEBUG_LOG
    va_list args;
    va_start(args, fmt);
    fprintf(stdout, "[DEBUG] ");
    vfprintf(stdout, fmt, args);
    fprintf(stdout, "\n");
    va_end(args);
#else
    (void)fmt;
#endif
}

} // namespace Log
