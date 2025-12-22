/*
 * ztime.h — Cross-platform Time, Clocks, Sleep, and Profiling
 * Part of Zen Development Kit (ZDK)
 *
 * Usage:
 * #define ZTIME_IMPLEMENTATION
 * #include "ztime.h"
 *
 * License: MIT
 * Version: 1.0.0
 */

#ifndef ZTIME_H
#define ZTIME_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/// @section API Reference
///
/// @columns Function | Description
/// @table Core Time
/// @row `ztime_now_ns()` | Returns monotonic time in nanoseconds (for measuring intervals).
/// @row `ztime_now_us()` | Returns monotonic time in microseconds.
/// @row `ztime_now_ms()` | Returns monotonic time in milliseconds.
/// @row `ztime_epoch()` | Returns Unix timestamp (seconds since 1970).
/// @row `ztime_wall_ms()` | Returns milliseconds since the Unix epoch (wall clock).
/// @row `ztime_diff_ns(start, end)` | Returns `end - start`, or 0 if `start > end` (underflow protection).
/// @endgroup

// => Core time.

// Monotonic time (use for measuring intervals/benchmarks).
uint64_t ztime_now_ns(void);
uint64_t ztime_now_us(void);
uint64_t ztime_now_ms(void);

// Wall clock (use for timestamps).
uint64_t ztime_epoch(void);    // Unix timestamp (seconds).
uint64_t ztime_wall_ms(void);  // Milliseconds since epoch.

// Safe difference (returns 0 if start > end to prevent underflow).
uint64_t ztime_diff_ns(uint64_t start, uint64_t end);

/// @table Sleep
/// @row `ztime_sleep_ms(ms)` | Standard thread sleep for `ms` milliseconds.
/// @row `ztime_sleep_us(us)` | High-precision sleep. Uses spin-wait on Windows for small intervals.
/// @endgroup

// => Sleep.

// Thread-safe sleep.
void ztime_sleep_ms(uint32_t ms);

// High-precision microsecond sleep.
// On Windows, uses a hybrid spin-wait for precision < 16ms.
void ztime_sleep_us(uint32_t us);

/// @table Stopwatch
/// @row `ztime_sw_start(sw)` | Starts or restarts the stopwatch.
/// @row `ztime_sw_elapsed_ns(sw)` | Returns nanoseconds elapsed since start.
/// @row `ztime_sw_elapsed_ms(sw)` | Returns milliseconds (double) elapsed since start.
/// @endgroup

// => Stopwatch.
// Profiling tool to measure elapsed time.

typedef struct 
{
    uint64_t start_time;
    bool running;
} zstopwatch_t;

void     ztime_sw_start(zstopwatch_t *sw);
uint64_t ztime_sw_elapsed_ns(const zstopwatch_t *sw);
double   ztime_sw_elapsed_ms(const zstopwatch_t *sw);

/// @table Ticker (Loop Stabilizer)
/// @row `ztime_ticker_init(t, hz)` | Initializes ticker for a target frequency (for example, 60 Hz).
/// @row `ztime_ticker_tick(t)` | Sleeps until next tick. Returns delta time (seconds) since last frame.
/// @endgroup

// => Ticker / rate limiter.
// Stabilizes loops to a fixed frequency (for example, 60 Hz).

typedef struct {
    uint64_t interval_ns;
    uint64_t next_tick;
    uint64_t start_tick;
    uint64_t frame_count;
} zticker_t;

// Initialize ticker for target Hz (for example, 60).
void ztime_ticker_init(zticker_t *t, uint32_t target_hz);

// Sleeps until next tick. Returns delta time (seconds) since last tick.
double ztime_ticker_tick(zticker_t *t);

/// @table Timeouts
/// @row `ztime_timeout_start(ms)` | Creates a timeout check set to expire `ms` milliseconds from now.
/// @row `ztime_timeout_expired(t)` | Returns `true` if the deadline has passed.
/// @row `ztime_timeout_rem_ms(t)` | Returns milliseconds remaining (0 if expired).
/// @endgroup

// => Timeouts
// Simple API to handle "wait for X, but give up after Y".

typedef struct 
{
    uint64_t deadline_ns;
} ztimeout_t;

// Start a timeout for N milliseconds from now.
ztimeout_t ztime_timeout_start(uint32_t ms);

// Returns true if time has expired.
bool ztime_timeout_expired(const ztimeout_t *t);

// Returns remaining milliseconds (0 if expired).
uint64_t ztime_timeout_rem_ms(const ztimeout_t *t);

/// @table Formatting
/// @row `ztime_fmt_log(buf, len, t)` | Formats standard time `t` as "YYYY-MM-DD HH:MM:SS".
/// @row `ztime_fmt_now(buf, len)` | Formats current wall time with ms: "YYYY-MM-DD HH:MM:SS.mmm".
/// @endgroup

// Some formatting.

// Standard Log: "2023-12-20 14:30:05" (Buffer >= 20 bytes)
size_t ztime_fmt_log(char *buf, size_t size, time_t t);

// High-Res Log: "2023-12-20 14:30:05.123" (Buffer >= 26 bytes)
size_t ztime_fmt_now(char *buf, size_t size);

#ifdef __cplusplus
}
#endif
#endif // ZTIME_H

#ifdef ZTIME_IMPLEMENTATION
#ifndef ZTIME_IMPLEMENTATION_GUARD
#define ZTIME_IMPLEMENTATION_GUARD

#include <stdio.h>

#if defined(_WIN32)
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#else
#   if !defined(_POSIX_C_SOURCE) || _POSIX_C_SOURCE < 200809L
#       undef _POSIX_C_SOURCE
#       define _POSIX_C_SOURCE 200809L
#   endif
#   include <time.h>
#   include <sys/time.h>
#   include <errno.h>
#   include <unistd.h>
#endif

// Internal helpers.
#ifdef _WIN32
static uint64_t ztime__freq = 0;
static void ztime__init_freq(void) 
{
    LARGE_INTEGER li;
    QueryPerformanceFrequency(&li);
    ztime__freq = (uint64_t)li.QuadPart;
    timeBeginPeriod(1); // Set Windows timer resolution to 1ms.
}
#endif

// Core.
uint64_t ztime_now_ns(void) 
{
#   ifdef _WIN32
    if (0 == ztime__freq) 
    {
        ztime__init_freq();
    }
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    uint64_t whole = (uint64_t)li.QuadPart / ztime__freq;
    uint64_t part  = (uint64_t)li.QuadPart % ztime__freq;
    return (whole * 1000000000ULL) + (part * 1000000000ULL / ztime__freq);
#   else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
#   endif
}

uint64_t ztime_now_us(void) 
{ 
    return ztime_now_ns() / 1000; 
}

uint64_t ztime_now_ms(void) 
{ 
    return ztime_now_ns() / 1000000; 
}

uint64_t ztime_diff_ns(uint64_t s, uint64_t e) 
{ 
    return (e > s) ? (e - s) : 0; 
}

uint64_t ztime_epoch(void) 
{ 
    return (uint64_t)time(NULL); 
}

uint64_t ztime_wall_ms(void) 
{
#   ifdef _WIN32
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    ULARGE_INTEGER li = 
    { 
        .LowPart = ft.dwLowDateTime, 
        .HighPart = ft.dwHighDateTime 
    };
    return (li.QuadPart - 116444736000000000ULL) / 10000;
#   else
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
#   endif
}

// Sleep.
void ztime_sleep_ms(uint32_t ms) 
{
#   ifdef _WIN32
    Sleep((DWORD)ms);
#   else
    struct timespec ts = 
    { 
        .tv_sec = ms / 1000, 
        .tv_nsec = (ms % 1000) * 1000000 
    };
    while (-1 == nanosleep(&ts, &ts) && EINTR == errno);
#   endif
}

void ztime_sleep_us(uint32_t us) 
{
#   ifdef _WIN32
    if (us > 20000) 
    { 
        Sleep(us / 1000); 
    }
    else 
    {
        uint64_t start = ztime_now_us();
        while (ztime_now_us() - start < us) 
        {
#           if defined(_MSC_VER)
            _mm_pause();
#           else
            __builtin_ia32_pause();
#           endif
        }
    }
#   else
    struct timespec ts = 
    { 
        .tv_sec = us / 1000000, 
        .tv_nsec = (us % 1000000) * 1000 
    };
    while (-1 == nanosleep(&ts, &ts) && EINTR == errno);
#   endif
}

// Stopwatch.
void ztime_sw_start(zstopwatch_t *sw) 
{
    sw->running = true;
    sw->start_time = ztime_now_ns();
}

uint64_t ztime_sw_elapsed_ns(const zstopwatch_t *sw) 
{
    if (!sw || !sw->running) 
    {
        return 0;
    }
    return ztime_diff_ns(sw->start_time, ztime_now_ns());
}

double ztime_sw_elapsed_ms(const zstopwatch_t *sw) 
{
    return (double)ztime_sw_elapsed_ns(sw) / 1000000.0;
}

// Ticker.
void ztime_ticker_init(zticker_t *t, uint32_t target_hz) 
{
    if (0 == target_hz) 
    {
        target_hz = 60;
    }
    t->interval_ns = 1000000000ULL / target_hz;
    t->next_tick = ztime_now_ns() + t->interval_ns;
    t->start_tick = ztime_now_ns();
    t->frame_count = 0;
}

double ztime_ticker_tick(zticker_t *t) 
{
    uint64_t now = ztime_now_ns();
    if (now < t->next_tick) 
    {
        uint64_t wait_ns = t->next_tick - now;
        if (wait_ns > 2000000) 
        {
            ztime_sleep_ms((uint32_t)(wait_ns / 1000000));
        }
        while (ztime_now_ns() < t->next_tick) 
        { 
#           if defined(_MSC_VER)
            _mm_pause();
#           else
            __builtin_ia32_pause();
#           endif
        }
    }
    uint64_t finished_at = ztime_now_ns();
    double dt = (double)ztime_diff_ns(t->start_tick, finished_at) / 1000000000.0;
    t->start_tick = finished_at;
    t->next_tick += t->interval_ns;
    if (finished_at > t->next_tick + 100000000ULL) 
    {
        t->next_tick = finished_at + t->interval_ns;
    }
    t->frame_count++;
    return dt;
}

// Timeouts.
ztimeout_t ztime_timeout_start(uint32_t ms) 
{
    ztimeout_t t;
    t.deadline_ns = ztime_now_ns() + ((uint64_t)ms * 1000000ULL);
    return t;
}

bool ztime_timeout_expired(const ztimeout_t *t) 
{
    return ztime_now_ns() >= t->deadline_ns;
}

uint64_t ztime_timeout_rem_ms(const ztimeout_t *t) 
{
    uint64_t now = ztime_now_ns();
    if (now >= t->deadline_ns) 
    {
        return 0;
    }
    return (t->deadline_ns - now) / 1000000ULL;
}

// Formatting.
size_t ztime_fmt_log(char *buf, size_t size, time_t t) 
{
    if (!buf || size < 20) 
    {
        return 0;
    }
    struct tm tm;
#   ifdef _WIN32
    localtime_s(&tm, &t);
#   else
    localtime_r(&t, &tm);
#   endif
    return strftime(buf, size, "%Y-%m-%d %H:%M:%S", &tm);
}

size_t ztime_fmt_now(char *buf, size_t size) 
{
    if (!buf || size < 26) 
    {
        return 0;
    }
    uint64_t now_ms_total = ztime_wall_ms();
    time_t now_sec = (time_t)(now_ms_total / 1000);
    int ms = (int)(now_ms_total % 1000);
    struct tm tm;
#   ifdef _WIN32
    localtime_s(&tm, &now_sec);
#   else
    localtime_r(&now_sec, &tm);
#   endif
    size_t len = strftime(buf, size, "%Y-%m-%d %H:%M:%S", &tm);
    if (len > 0 && len + 4 < size) 
    {
        snprintf(buf + len, size - len, ".%03d", ms);
        return len + 4;
    }
    return len;
}

#endif // ZTIME_IMPLEMENTATION_GUARD
#endif // ZTIME_IMPLEMENTATION

