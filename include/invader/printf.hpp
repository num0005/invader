// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__PRINTF_HPP
#define INVADER__PRINTF_HPP

#ifndef NO_OUTPUT

#include <cstdlib>
#include <cstring>

// If NO_OUTPUT is not enabled, then we have eprintf and oprintf as a macro for, basically, printf
#include <cstdio>

#ifdef _WIN32
#include <windows.h>

#define ON_COLOR_TERM true

#define output_colored(color, fn, ...) { \
    CONSOLE_SCREEN_BUFFER_INFO info = {}; \
    auto stdouthandle = GetStdHandle(STD_OUTPUT_HANDLE); \
    GetConsoleScreenBufferInfo(stdouthandle, &info); \
    SetConsoleTextAttribute(stdouthandle, color); \
    fn(__VA_ARGS__); \
    SetConsoleTextAttribute(stdouthandle, info.wAttributes); \
    fn("\n"); \
}

#define eprintf_error(...) output_colored(0xC, eprintf, __VA_ARGS__);
#define eprintf_warn(...) output_colored(0xE, eprintf, __VA_ARGS__);
#define eprintf_warn_lesser(...) output_colored(0xD, eprintf, __VA_ARGS__);
#define oprintf_success(...) output_colored(0xA, oprintf, __VA_ARGS__);
#define oprintf_success_warn(...) output_colored(0xE, oprintf, __VA_ARGS__);
#define oprintf_success_lesser_warn(...) output_colored(0xD, oprintf, __VA_ARGS__);
#define oprintf_fail(...) output_colored(0xC, oprintf, __VA_ARGS__);

#else

#define ON_COLOR_TERM (std::getenv("TERM") && ((std::strcmp(std::getenv("TERM"), "xterm-256color") == 0 || std::strcmp(std::getenv("TERM"), "xterm-color") == 0 || std::strcmp(std::getenv("TERM"), "xterm-16color") == 0)))

#define eprintf_error(...) if(ON_COLOR_TERM) {\
    eprintf("\x1B[1;38;5;1m"); \
    eprintf(__VA_ARGS__); \
    eprintf("\x1B[m\n"); \
} \
else {\
    eprintf(__VA_ARGS__); \
    eprintf("\n"); \
}

#define eprintf_warn(...) if(ON_COLOR_TERM) {\
    eprintf("\x1B[1;38;5;3m"); \
    eprintf(__VA_ARGS__); \
    eprintf("\x1B[m\n"); \
} \
else {\
    eprintf(__VA_ARGS__); \
    eprintf("\n"); \
}

#define eprintf_warn_lesser(...) if(ON_COLOR_TERM) {\
    eprintf("\x1B[1;38;5;5m"); \
    eprintf(__VA_ARGS__); \
    eprintf("\x1B[m\n"); \
} \
else {\
    eprintf(__VA_ARGS__); \
    eprintf("\n"); \
}

#define oprintf_success(...) if(ON_COLOR_TERM) {\
    oprintf("\x1B[38;5;2m"); \
    oprintf(__VA_ARGS__); \
    oprintf("\x1B[m\n"); \
} \
else {\
    oprintf(__VA_ARGS__); \
    oprintf("\n"); \
}

#define oprintf_success_warn(...) if(ON_COLOR_TERM) {\
    oprintf("\x1B[1;38;5;3m"); \
    oprintf(__VA_ARGS__); \
    oprintf("\x1B[m\n"); \
} \
else {\
    oprintf(__VA_ARGS__); \
    oprintf("\n"); \
}
#define oprintf_success_lesser_warn(...) if(ON_COLOR_TERM) {\
    oprintf("\x1B[1;38;5;5m"); \
    oprintf(__VA_ARGS__); \
    oprintf("\x1B[m\n"); \
} \
else {\
    oprintf(__VA_ARGS__); \
    oprintf("\n"); \
}
#define oprintf_fail(...) if(ON_COLOR_TERM) {\
    oprintf("\x1B[1;38;5;1m"); \
    oprintf(__VA_ARGS__); \
    oprintf("\x1B[m\n"); \
} \
else {\
    oprintf(__VA_ARGS__); \
    oprintf("\n"); \
}

#endif

#define eprintf(...) std::fprintf(stderr, __VA_ARGS__)
#define oprintf(...) std::fprintf(stdout, __VA_ARGS__)
#define oflush() std::fflush(stdout)

#else

#define ON_COLOR_TERM false

// Otherwise, we have eprintf and oprintf as an inline, variadic function that does nothing. This is so we don't get any unused variable warnings.
static inline int eprintf(...) {}
static inline int oprintf(...) {}
static inline void oflush() {}
#endif

#endif
