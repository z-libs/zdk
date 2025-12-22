/*
 * GENERATED FILE - DO NOT EDIT DIRECTLY
 * Source: zerror.c
 *
 * This file is part of the z-libs collection: https://github.com/z-libs
 * Licensed under the MIT License.
 */


/* ============================================================================
   z-libs Common Definitions (Bundled)
   This block is auto-generated. It is guarded so that if you include multiple
   z-libs it is only defined once.
   ============================================================================ */
#ifndef Z_COMMON_BUNDLED
#define Z_COMMON_BUNDLED


/*
 * zcommon.h — Common definitions for the Zen Development Kit (ZDK)
 * Part of ZDK
 *
 * This header defines shared macros, error codes, and compiler extensions
 * used across all ZDK libraries.
 *
 * License: MIT
 * Author: Zuhaitz
 * Repository: https://github.com/z-libs/z-core
 * Version: 1.0.0
 */

#ifndef ZCOMMON_H
#define ZCOMMON_H

#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

// Return codes and error handling.
#define Z_OK          0
#define Z_FOUND       1   // Element found (positive).
#define Z_ERR        -1   // Generic error.
#define Z_ENOMEM     -2   // Out of memory (malloc/realloc failed).
#define Z_EOOB       -3   // Out of bounds / range error.
#define Z_EEMPTY     -4   // Container is empty.
#define Z_ENOTFOUND  -5   // Element not found.
#define Z_EINVAL     -6   // Invalid argument / parameter.
#define Z_EEXIST     -7   // Element already exists.

// Memory management.

/* * If the user hasn't defined their own allocator, use the standard C library.
 * To override globally, define these macros before including any ZDK header.
 */
#ifndef Z_MALLOC
#   include <stdlib.h>
#   define Z_MALLOC(sz)       malloc(sz)
#   define Z_CALLOC(n, sz)    calloc(n, sz)
#   define Z_REALLOC(p, sz)   realloc(p, sz)
#   define Z_FREE(p)          free(p)
#endif


// Compiler extensions and optimization.

// Type inference (typeof)
#ifdef __cplusplus
#   include <type_traits>
#   define Z_TYPEOF(x) typename std::remove_reference<decltype(x)>::type
#   define Z_HAS_TYPEOF 1
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L
#   define Z_TYPEOF(x) typeof(x)
#   define Z_HAS_TYPEOF 1
#elif defined(__GNUC__) || defined(__clang__) || defined(__TINYC__)
#   define Z_TYPEOF(x) __typeof__(x)
#   define Z_HAS_TYPEOF 1
#else
#   define Z_HAS_TYPEOF 0
#endif

// Extensions (cleanup, attributes, branch prediction)
#if !defined(Z_NO_EXTENSIONS) && (defined(__GNUC__) || defined(__clang__) || defined(__TINYC__))
        
#   define Z_HAS_CLEANUP 1
#   define Z_CLEANUP(func) __attribute__((cleanup(func)))
#   define Z_NODISCARD     __attribute__((warn_unused_result))
    
    // TCC supports attributes but NOT __builtin_expect
#   if defined(__TINYC__)
#       define Z_LIKELY(x)     (x)
#       define Z_UNLIKELY(x)   (x)
#   else
#       define Z_LIKELY(x)     __builtin_expect(!!(x), 1)
#       define Z_UNLIKELY(x)   __builtin_expect(!!(x), 0)
#   endif

#else
    // Fallback for MSVC or strict standard C.
#   define Z_HAS_CLEANUP 0
#   define Z_CLEANUP(func) 
#   define Z_NODISCARD
#   define Z_LIKELY(x)     (x)
#   define Z_UNLIKELY(x)   (x)

#endif


// Metaprogramming and internal utils.

/* * Markers for the Z-Scanner tool to find type definitions.
 * For the C compiler, they are no-ops (they compile to nothing).
 */
#define DEFINE_VEC_TYPE(T, Name)
#define DEFINE_LIST_TYPE(T, Name)
#define DEFINE_MAP_TYPE(Key, Val, Name)
#define DEFINE_STABLE_MAP_TYPE(Key, Val, Name)
#define DEFINE_TREE_TYPE(Key, Val, Name)

// Token concatenation macros (useful for unique variable names in macros).
#define Z_CONCAT_(a, b) a ## b
#define Z_CONCAT(a, b) Z_CONCAT_(a, b)
#define Z_UNIQUE(prefix) Z_CONCAT(prefix, __LINE__)

// Growth strategy.

/* * Determines how containers expand when full.
 * Default is 2.0x (Geometric Growth).
 *
 * Optimization note:
 * 2.0x minimizes realloc calls but can waste memory.
 * 1.5x is often better for memory fragmentation and reuse.
 */
#ifndef Z_GROWTH_FACTOR
    // Default: Double capacity (2.0x).
#   define Z_GROWTH_FACTOR(cap) ((cap) == 0 ? 32 : (cap) * 2)
    
    // Alternative: 1.5x Growth (Uncomment to use in your project).
    // #define Z_GROWTH_FACTOR(cap) ((cap) == 0 ? 32 : (cap) + (cap) / 2)
#endif

#endif // ZCOMMON_H


#endif // Z_COMMON_BUNDLED
/* ============================================================================ */

/*
 * zerror.h — Error handling, result types, and panic machinery
 * Part of Zen Development Kit (ZDK)
 *
 * Usage:
 * #define ZERROR_IMPLEMENTATION
 * #define ZERROR_SHORT_NAMES
 * #include "zerror.h"
 *
 * License: MIT
 * Author: Zuhaitz
 * Repository: https://github.com/z-libs/zerror.h
 * Version: 1.1.1
 */

#ifndef ZERROR_H
#define ZERROR_H

// Enable POSIX extensions for localtime_r.
#if !defined(_POSIX_C_SOURCE) || _POSIX_C_SOURCE < 200809L
#   undef _POSIX_C_SOURCE
#   define _POSIX_C_SOURCE 200809L
#endif

// Include common definitions if available.
#ifdef __has_include
#   if __has_include("zcommon.h")
#       include "zcommon.h"
#   endif
#endif

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef __cplusplus
#   include <iostream>
#   include <string>
#   include <stdexcept>
#   include <type_traits>
#   include <utility>
#   include <new>
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Detect typeof support (GCC/Clang extension).
#ifndef Z_HAS_TYPEOF
#   if defined(__GNUC__) || defined(__clang__) || defined(__TINYC__)
#       define Z_HAS_TYPEOF 1
#       define Z_TYPEOF(x) __typeof__(x)
#   else
#       define Z_HAS_TYPEOF 0
#   endif
#endif

// Helper macros (Guarded to avoid zcommon.h conflicts).
#ifndef Z_CONCAT
#   define Z_CONCAT_IMPL(a, b) a##b
#   define Z_CONCAT(a, b) Z_CONCAT_IMPL(a, b)
#endif

#ifndef ZERROR_UID
#   define ZERROR_UID(prefix) Z_CONCAT(prefix, __LINE__)
#endif

/// @section API Reference (C)
///
/// @section Logging
/// @table Logging & Debugging
/// @columns Function / Macro | Description
/// @row `zlog_init(path, level)` | Initializes logging to a file (optional) and sets min level.
/// @row `zlog_set_level(level)` | Sets the minimum logging level at runtime.
/// @row `log_info(...)` | Logs an info message (White).
/// @row `log_warn(...)` | Logs a warning message (Yellow).
/// @row `log_error(...)` | Logs an error message (Red).
/// @row `log_debug(...)` | Logs a debug message (Cyan, if level permits).
/// @row `log_trace(...)` | Logs a trace message (Blue, if level permits).
/// @endgroup

// Logging config.

typedef enum 
{
    ZLOG_TRACE = 0,
    ZLOG_DEBUG,
    ZLOG_INFO,
    ZLOG_WARN,
    ZLOG_ERROR,
    ZLOG_FATAL,
    ZLOG_NONE
} zlog_level;

void zlog_init(const char *file_path, zlog_level min_level);
void zlog_set_level(zlog_level level);

// Internal function.
void zlog_msg(zlog_level level, const char *file, int line, const char *func, const char *fmt, ...);

// Pleasant macros.
#define log_trace(...) zlog_msg(ZLOG_TRACE, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define log_debug(...) zlog_msg(ZLOG_DEBUG, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define log_info(...)  zlog_msg(ZLOG_INFO,  __FILE__, __LINE__, __func__, __VA_ARGS__)
#define log_warn(...)  zlog_msg(ZLOG_WARN,  __FILE__, __LINE__, __func__, __VA_ARGS__)
#define log_error(...) zlog_msg(ZLOG_ERROR, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define log_fatal(...) zlog_msg(ZLOG_FATAL, __FILE__, __LINE__, __func__, __VA_ARGS__)

// Legacy/caps aliases.
#define LOG_INFO  log_info
#define LOG_WARN  log_warn
#define LOG_ERROR log_error
#define LOG_DEBUG log_debug
#define LOG_TRACE log_trace
#define LOG_FATAL log_fatal

/// @section Error Types
/// @table Data Structures
/// @columns Type | Description
/// @row `zerr` | Base error struct containing code, message, file, line, and trace info.
/// @row `zres` | Standard void result type (contains `is_ok` and `zerr`).
/// @row `ResInt` | Typed result carrying an `int` value or an error.
/// @row `ResPtr` | Typed result carrying a `void*` value or an error.
/// @endgroup

// Error types.

#ifndef ZERROR_PANIC_ACTION
#   define ZERROR_PANIC_ACTION() abort()
#endif

typedef struct 
{
    int code;
    const char *msg;
    const char *file;
    int line;
    const char *func;
    const char *source;
} zerr;

/// @section Error Management
/// @table Creation & Manipulation
/// @columns Function | Description
/// @row `zerr_create(code, msg)` | Creates a new error with current file/line context.
/// @row `zerr_errno(code, msg)` | Creates a new error, appending the string description of `errno`.
/// @row `zerr_wrap(e, fmt, ...)` | Wraps an existing error with a new context message.
/// @row `zerr_print(e)` | Prints a stylized error report to stderr.
/// @row `zerr_panic(msg)` | Prints a panic message and aborts the program.
/// @endgroup

zerr zerr_create_impl(int code, const char *file, int line, const char *func, const char *fmt, ...);
zerr zerr_errno_impl(int code, const char *file, int line, const char *func, const char *fmt, ...);

zerr zerr_wrap(zerr e, const char *fmt, ...);
zerr zerr_add_trace(zerr e, const char *func, const char *file, int line);

void zerr_print(zerr e);
void zerr_panic(const char *msg, const char *file, int line);

// Result types.

#define DEFINE_RESULT(T, Name)                                                              \
    typedef struct { bool is_ok; union { T val; zerr err; }; } Name;                        \
    static inline Name Name##_ok(T v) { Name r; r.is_ok = true; r.val = v; return r; }      \
    static inline Name Name##_err(zerr e) { Name r; r.is_ok = false; r.err = e; return r; }

typedef struct 
{ 
    bool is_ok; 
    zerr err; 
} zres;

static inline zres zres_ok(void) 
{ 
    return (zres)
    { 
        .is_ok = true, 
        .err = {0, NULL, NULL, 0, NULL, NULL} 
    }; 
}

static inline zres zres_err(zerr e) 
{ 
    return (zres)
    { 
        .is_ok = false, 
        .err = e 
    }; 
}

DEFINE_RESULT(int,      ResInt)
DEFINE_RESULT(float,    ResFloat)
DEFINE_RESULT(double,   ResDouble)
DEFINE_RESULT(bool,     ResBool)
DEFINE_RESULT(size_t,   ResSize)
DEFINE_RESULT(void*,    ResPtr)
DEFINE_RESULT(char*,    ResStr)

#if defined(ZERROR_DEBUG) && (defined(__GNUC__) || defined(__clang__))
#   define ZERROR_TRAP() __builtin_trap()
#elif defined(ZERROR_DEBUG) && defined(_MSC_VER)
#   define ZERROR_TRAP() __debugbreak()
#else
#   define ZERROR_TRAP() ((void)0)
#endif

#define zerr_create(code, ...) (ZERROR_TRAP(), zerr_create_impl((code), __FILE__, __LINE__, __func__, __VA_ARGS__))
#define zerr_errno(code, ...) (ZERROR_TRAP(), zerr_errno_impl((code), __FILE__, __LINE__, __func__, __VA_ARGS__))

// Runtime helpers
static inline zerr zerr_with_src(zerr e, const char *src) 
{
    if (NULL == e.source) 
    {
        e.source = src;
    }
    return e;
}

#ifdef ZERROR_ENABLE_TRACE
#   define ZERROR_TRACE_OP(e) zerr_add_trace(e, __func__, __FILE__, __LINE__)
#else
#   define ZERROR_TRACE_OP(e) (e)
#endif

int zerr_run(zres result);
#define ZERROR_RUN(expr) zerr_run(expr)

// Universal macros.

#define ZERROR_ENSURE(cond, src, code, msg)         \
    do {                                            \
        if (!(cond))                                \
        {                                           \
            zerr _e = zerr_create((code), (msg));   \
            _e.source = src;                        \
            return zres_err(_e);                    \
        }                                           \
    } while(0)

#define ZERROR_ENSURE_INTO(RetType, cond, src, code, msg)   \
    do {                                                    \
        if (!(cond))                                        \
        {                                                   \
            zerr _e = zerr_create((code), (msg));           \
            _e.source = src;                                \
            return RetType##_err(_e);                       \
        }                                                   \
    } while(0)

#define ZERROR_CHECK_SYS(expr, fmt, ...)                            \
    do {                                                            \
        if ((expr) != 0)                                            \
        {                                                           \
            return zres_err(zerr_errno(errno, fmt, ##__VA_ARGS__)); \
        }                                                           \
    } while(0)

// Modern macros.

#if Z_HAS_TYPEOF && (!defined(__cplusplus) || defined(__GNUC__))
    
#   define ZERROR_CHECK(expr, src)                                              \
        do {                                                                    \
            Z_TYPEOF(expr) ZERROR_UID(_r) = (expr);                             \
            if (!ZERROR_UID(_r).is_ok)                                          \
            {                                                                   \
                ZERROR_UID(_r).err = zerr_with_src(ZERROR_UID(_r).err, src);    \
                ZERROR_UID(_r).err = ZERROR_TRACE_OP(ZERROR_UID(_r).err);       \
                return zres_err(ZERROR_UID(_r).err);                            \
            }                                                                   \
        } while(0)

#   define ZERROR_CHECK_INTO(RetType, expr, src)                                \
        do {                                                                    \
            Z_TYPEOF(expr) ZERROR_UID(_r) = (expr);                             \
            if (!ZERROR_UID(_r).is_ok)                                          \
            {                                                                   \
                ZERROR_UID(_r).err = zerr_with_src(ZERROR_UID(_r).err, src);    \
                ZERROR_UID(_r).err = ZERROR_TRACE_OP(ZERROR_UID(_r).err);       \
                return RetType##_err(ZERROR_UID(_r).err);                       \
            }                                                                   \
        } while(0)

#   define ZERROR_CHECK_WRAP(expr, src, fmt, ...)                                   \
        do {                                                                        \
            Z_TYPEOF(expr) ZERROR_UID(_r) = (expr);                                 \
            if (!ZERROR_UID(_r).is_ok)                                              \
            {                                                                       \
                ZERROR_UID(_r).err = zerr_with_src(ZERROR_UID(_r).err, src);        \
                ZERROR_UID(_r).err = ZERROR_TRACE_OP(ZERROR_UID(_r).err);           \
                return zres_err(zerr_wrap(ZERROR_UID(_r).err, fmt, ##__VA_ARGS__)); \
            }                                                                       \
        } while(0)
    
#   define ZERROR_CHECK_CTX(expr, src, fmt, ...)                                        \
        do {                                                                            \
            Z_TYPEOF(expr) ZERROR_UID(_r) = (expr);                                     \
            if (!ZERROR_UID(_r).is_ok)                                                  \
            {                                                                           \
                ZERROR_UID(_r).err = zerr_with_src(ZERROR_UID(_r).err, src);            \
                ZERROR_UID(_r).err = ZERROR_TRACE_OP(ZERROR_UID(_r).err);               \
                ZERROR_UID(_r).err = zerr_wrap(ZERROR_UID(_r).err, fmt, ##__VA_ARGS__); \
                return zres_err(ZERROR_UID(_r).err);                                    \
            }                                                                           \
        } while(0)

#   define ZERROR_TRY(expr, src)                                                    \
        ({  Z_TYPEOF(expr) ZERROR_UID(_res) = (expr);                               \
            if (!ZERROR_UID(_res).is_ok)                                            \
            {                                                                       \
                ZERROR_UID(_res).err = zerr_with_src(ZERROR_UID(_res).err, src);    \
                ZERROR_UID(_res).err = ZERROR_TRACE_OP(ZERROR_UID(_res).err);       \
                return ZERROR_UID(_res);                                            \
            }                                                                       \
            ZERROR_UID(_res).val;                                                   \
        })

#   define ZERROR_TRY_INTO(RetType, expr, src)                                      \
        ({  Z_TYPEOF(expr) ZERROR_UID(_res) = (expr);                               \
            if (!ZERROR_UID(_res).is_ok)                                            \
            {                                                                       \
                ZERROR_UID(_res).err = zerr_with_src(ZERROR_UID(_res).err, src);    \
                ZERROR_UID(_res).err = ZERROR_TRACE_OP(ZERROR_UID(_res).err);       \
                return RetType##_err(ZERROR_UID(_res).err);                         \
            }                                                                       \
            ZERROR_UID(_res).val;                                                   \
        })
    
#   define ZERROR_EXPECT(expr, msg)                         \
        ({  Z_TYPEOF(expr) ZERROR_UID(_res) = (expr);       \
            if (!ZERROR_UID(_res).is_ok)                    \
            {                                               \
                zerr_print(ZERROR_UID(_res).err);           \
                zerr_panic(msg, __FILE__, __LINE__);        \
            }                                               \
            ZERROR_UID(_res).val;                           \
        })

#   define ZERROR_DEFER_HK(l, c)                                \
        void Z_CONCAT(z_defer_fn_, l)(void *_) { (void)_; c; }  \
        __attribute__((cleanup(Z_CONCAT(z_defer_fn_, l))))      \
        char Z_CONCAT(z_defer_var_, l)
    
#   define zerr_defer(code) ZERROR_DEFER_HK(__LINE__, code)

#else
    // Strict fallback (Standard C / MSVC).
    
#   define ZERROR_CHECK(expr, src)                                      \
        do {                                                            \
            zres ZERROR_UID(_r) = (expr);                               \
            if (!ZERROR_UID(_r).is_ok)                                  \
            {                                                           \
                return zres_err(ZERROR_TRACE_OP(ZERROR_UID(_r).err));   \
            }                                                           \
        } while(0)

    #   define ZERROR_CHECK_INTO(RetType, expr, src)                        \
        do {                                                                \
            zres ZERROR_UID(_r) = (expr);                                   \
            if (!ZERROR_UID(_r).is_ok)                                      \
            {                                                               \
                return RetType##_err(ZERROR_TRACE_OP(ZERROR_UID(_r).err));  \
            }                                                               \
        } while(0)
    
#   define ZERROR_CHECK_WRAP(expr, src, fmt, ...)                                   \
        do {                                                                        \
            zres ZERROR_UID(_r) = (expr);                                           \
            if (!ZERROR_UID(_r).is_ok)                                              \
            {                                                                       \
                return zres_err(zerr_wrap(ZERROR_UID(_r).err, fmt, ##__VA_ARGS__)); \
            }                                                                       \
        } while(0)
    
#   define ZERROR_CHECK_CTX(expr, src, fmt, ...)                                        \
        do {                                                                            \
            zres ZERROR_UID(_r) = (expr);                                               \
            if (!ZERROR_UID(_r).is_ok)                                                  \
            {                                                                           \
                ZERROR_UID(_r).err = zerr_wrap(ZERROR_UID(_r).err, fmt, ##__VA_ARGS__); \
                return zres_err(ZERROR_UID(_r).err);                                    \
            }                                                                           \
        } while(0)

#endif

// Short names.
#ifdef ZERROR_SHORT_NAMES

    /// @section Macros
    /// @table Flow Control
    /// @columns Macro | Description
    /// @row `check(expr)` | Propagates error for functions returning `zres` or typed results.
    /// @row `check_into(Type, expr)` | Propagates error, converting it to a `Type` result.
    /// @row `check_wrap(expr, fmt)` | Propagates error with a wrapping context message.
    /// @row `try(expr)` | Evaluates `expr`. If error, returns it. Else returns the unwrapped value.
    /// @row `try_into(Type, expr)` | Same as `try`, but converts the error return type.
    /// @row `expect(expr, msg)` | Evaluates `expr`. If error, panics with `msg`.
    /// @row `ensure(cond, code, msg)` | Returns an error if `cond` is false.
    /// @row `run(expr)` | Executes entry point function (returning `zres`), printing errors on failure.
    /// @endgroup

#   define check(expr)                  ZERROR_CHECK(expr, #expr)
#   define check_into(T, expr)          ZERROR_CHECK_INTO(T, expr, #expr)
#   define check_wrap(expr, ...)        ZERROR_CHECK_WRAP(expr, #expr, __VA_ARGS__)
#   define check_ctx(expr, ...)         ZERROR_CHECK_CTX(expr, #expr, __VA_ARGS__)
#   define check_sys(expr, ...)         ZERROR_CHECK_SYS(expr, __VA_ARGS__)

#   define ensure(c, code, m)           ZERROR_ENSURE(c, #c, code, m)
#   define ensure_into(T, c, code, m)   ZERROR_ENSURE_INTO(T, c, #c, code, m)

#   ifdef ZERROR_TRY
#       define try(expr)            ZERROR_TRY(expr, #expr)
#       define try_into(T, expr)    ZERROR_TRY_INTO(T, expr, #expr)
#       define unwrap(expr)         ZERROR_EXPECT(expr, "unwrap() failed")
#       define expect(e, m)         ZERROR_EXPECT(e, m)
#   endif

#   ifdef ZERROR_DEFER_HK
#       define defer(code)          zerr_defer(code)
#   endif
    
#   define run(e)                   ZERROR_RUN(e)
#endif

#ifdef __cplusplus
} // extern "C"
#endif

#ifdef __cplusplus
namespace z_log 
{
    template <typename... Args> inline void info(const char* f, Args... a)  { log_info(f, a...); }
    template <typename... Args> inline void warn(const char* f, Args... a)  { log_warn(f, a...); }
    template <typename... Args> inline void error(const char* f, Args... a) { log_error(f, a...); }
    inline void info(const std::string &s) { log_info("%s", s.c_str()); }
    inline void error(const std::string &s) { log_error("%s", s.c_str()); }
}

/// @section API Reference (C++)
/// 
/// @section C++ Helpers
/// @table Namespace z_log
/// @columns Function | Description
/// @row `z_log::info(fmt, ...)` | Logs an info message using C-style formatting.
/// @row `z_log::warn(fmt, ...)` | Logs a warning message using C-style formatting.
/// @row `z_log::error(fmt, ...)` | Logs an error message using C-style formatting.
/// @row `z_log::info(str)` | Logs a `std::string` as info.
/// @row `z_log::error(str)` | Logs a `std::string` as error.
/// @endgroup
///
/// @section Result Type
/// @table Class z_error::result<T>
/// @columns Method | Description
/// @row `result(val)` | Constructors for creating a success result (move or copy).
/// @row `result(err)` | Constructors for creating a failure result from `zerr` or `zres`.
/// @row `ok()` | Returns `true` if the result contains a value (success).
/// @row `unwrap_val()` | Returns the contained value or panics if it is an error.
/// @row `err` | Public member accessing the underlying `zerr` struct (valid only if !ok()).
/// @row `operator bool()` | Implicit conversion to boolean (true = success).
/// @endgroup
///
/// @section C++ Macros
/// @table Shortcuts
/// @columns Macro | Description
/// @row `ztry(expr)` | Statement expression that unwraps a `result<T>` or returns the error immediately.
/// @endgroup

namespace z_error 
{
    template <typename T> 
    class result 
    {
        bool is_ok_;
        union 
        { 
            T val_; 
            ::zerr err_; 
        };
     public:
        ::zerr err;
        result(T &&v) : is_ok_(true), err{} 
        { 
            new (&val_) T(std::move(v)); 
        }

        result(const T &v) : is_ok_(true), err{} 
        { 
            new (&val_) T(v); 
        }
    
        result(::zerr e) : is_ok_(false), err_(e), err(e) {}

        result(::zres r) : is_ok_(false), err{} 
        {
            if (r.is_ok) 
            {
                ::zerr_panic("Constructed result<T> from zres_ok", __FILE__, __LINE__);
            }
            err_ = r.err; err = r.err;
        }

        result(result &&other) : is_ok_(other.is_ok_), err(other.err) 
        {
            if (is_ok_) 
            {
                new (&val_) T(std::move(other.val_)); 
            }
            else 
            {
                err_ = other.err_;
            }
        }

        ~result() 
        { 
            if (is_ok_) 
            {
                val_.~T(); 
            }
        }

        bool ok() const 
        { 
            return is_ok_; 
        }

        operator bool() const 
        { 
            return is_ok_; 
        }

        T &unwrap_val() 
        { 
            if (!is_ok_) 
            { 
                ::zerr_print(err_); 
                abort(); 
            } 
            return val_; 
        }
    };

    template <> 
    class result<void> 
    {
        bool is_ok_;
        ::zerr err_;
     public:
        ::zerr err; 

        result() : is_ok_(true), err_{0, NULL, NULL, 0, NULL, NULL}, err{} {}

        result(::zerr e) : is_ok_(false), err_(e), err(e) {}

        result(::zres r) : is_ok_(r.is_ok), err_(r.err), err(r.err) {}

        static result<void> success() 
        { 
            return result<void>(); 
        }

        bool ok() const 
        { 
            return is_ok_; 
        }

        operator bool() const 
        { 
            return is_ok_; 
        }

        void unwrap_val() 
        { 
            if (!is_ok_) 
            { 
                ::zerr_print(err_);
                abort(); 
            } 
        }
    };
    
    template <typename T> inline result<T> from_c_res(T val) 
    { 
        return result<T>(val); 
    }
}

#if defined(ZERROR_SHORT_NAMES) && (defined(__GNUC__) || defined(__clang__))
#   define ztry(expr) ({ auto _r = (expr); if (!_r.ok()) return _r.err; std::move(_r.unwrap_val()); })
#endif

#endif // __cplusplus

#endif // ZERROR_H

#ifdef ZERROR_IMPLEMENTATION
#ifndef ZERROR_IMPLEMENTATION_GUARD
#define ZERROR_IMPLEMENTATION_GUARD

#include <time.h> 

#if defined(_WIN32)
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#else
#   include <pthread.h>
#   include <sys/time.h>
#   include <unistd.h>
#endif

static struct 
{
    FILE *fp;
    zlog_level level;
    bool colors;
    bool init;
#   if defined(_WIN32)
    CRITICAL_SECTION mutex;
#   else
    pthread_mutex_t mutex;
#   endif
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wmissing-field-initializers"
} zlog__state = { NULL, ZLOG_INFO, true, false };
#pragma GCC diagnostic pop

static const char *zlog__colors[] = 
{
    "\x1b[94m", 
    "\x1b[36m", 
    "\x1b[32m", 
    "\x1b[33m", 
    "\x1b[31m", 
    "\x1b[35m"
};

static const char *zlog__labels[] = 
{ 
    "TRACE", 
    "DEBUG", 
    "INFO ", 
    "WARN ", 
    "ERROR", 
    "FATAL" 
};

#if defined(_MSC_VER)
    static __declspec(thread) char z_err_buf[2048];
#else
    static __thread char z_err_buf[2048];
#endif

// Helpers.
static void zlog__init_mutex(void) 
{
    if (zlog__state.init) 
    {
        return;
    }
#   if defined(_WIN32)
    InitializeCriticalSection(&zlog__state.mutex);
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    SetConsoleMode(hOut, dwMode | 0x0004);
#   else
    pthread_mutex_init(&zlog__state.mutex, NULL);
#   endif
    zlog__state.init = true;
}

static void zlog__lock(void) 
{
    if (!zlog__state.init) 
    {
        zlog__init_mutex();
    }
#   if defined(_WIN32)
    EnterCriticalSection(&zlog__state.mutex);
#   else
    pthread_mutex_lock(&zlog__state.mutex);
#   endif
}

static void zlog__unlock(void) 
{
    if (!zlog__state.init) 
    {
        return;
    }
#   if defined(_WIN32)
    LeaveCriticalSection(&zlog__state.mutex);
#   else
    pthread_mutex_unlock(&zlog__state.mutex);
#   endif
}

static void zlog__get_time(char *buf, size_t size) 
{
#   if defined(ZTIME_H)
    ztime_fmt_now(buf, size);
#   else
    time_t t = time(NULL);
    struct tm tm;
    memset(&tm, 0, sizeof(tm));

#   ifdef _WIN32
    localtime_s(&tm, &t);
#   else
    struct tm *ptr = localtime(&t);
    if (ptr) 
    {
        tm = *ptr;
    }
#   endif
    strftime(buf, size, "%Y-%m-%d %H:%M:%S", &tm);
#endif
}

void zlog_init(const char *file_path, zlog_level min_level) 
{
    zlog__init_mutex();
    zlog__state.level = min_level;
    if (file_path) 
    {
        zlog__state.fp = fopen(file_path, "a");
    }
}

void zlog_set_level(zlog_level level) 
{ 
    zlog__state.level = level; 
}

static void zlog__print_internal(zlog_level lvl, const char *label, const char *time_str, 
                                 const char *msg, const char *file, int line, const char *func, const char *extra) 
{
    if (zlog__state.colors) 
    {
        fprintf(stderr, "\n%s[%s] %s:%s %s\n", zlog__colors[lvl], time_str, label, "\x1b[0m", msg);
        fprintf(stderr, "    \x1b[90mat\x1b[0m %s (%s:%d)%s\n", func ? func : "?", file, line, extra ? extra : "");
    } 
    else 
    {
        fprintf(stderr, "\n[%s] %s: %s\n", time_str, label, msg);
        fprintf(stderr, "    at %s (%s:%d)%s\n", func ? func : "?", file, line, extra ? extra : "");
    }
    if (zlog__state.fp) 
    {
        fprintf(zlog__state.fp, "\n[%s] %s: %s\n", time_str, label, msg);
        fprintf(zlog__state.fp, "    at %s (%s:%d)%s\n", func ? func : "?", file, line, extra ? extra : "");
        fflush(zlog__state.fp);
    }
}

void zlog_msg(zlog_level level, const char *file, int line, const char *func, const char *fmt, ...) 
{
    if (level < zlog__state.level) 
    {
        return;
    }
    char time_buf[64];
    zlog__get_time(time_buf, sizeof(time_buf));
    char msg_buf[2048];
    va_list args;
    va_start(args, fmt);
    vsnprintf(msg_buf, sizeof(msg_buf), fmt, args);
    va_end(args);

    zlog__lock();
    zlog__print_internal(level, zlog__labels[level], time_buf, msg_buf, file, line, func, NULL);
    zlog__unlock();
}

void zerr_print(zerr e) 
{
    char time_buf[64];
    zlog__get_time(time_buf, sizeof(time_buf));
    char extra_buf[1024] = {0};
    if (e.source) 
    {
        snprintf(extra_buf, sizeof(extra_buf), "\n    [Expr] %s", e.source);
    }

    zlog__lock();
    zlog__print_internal(ZLOG_ERROR, "Error", time_buf, e.msg, e.file, e.line, e.func, extra_buf);
    zlog__unlock();
}

void zerr_panic(const char *msg, const char *file, int line) 
{
    char time_buf[64];
    zlog__get_time(time_buf, sizeof(time_buf));
    zlog__lock();
    zlog__print_internal(ZLOG_FATAL, "PANIC", time_buf, msg, file, line, "!", NULL);
    zlog__unlock();
    ZERROR_TRAP();
    ZERROR_PANIC_ACTION();
}

zerr zerr_create_impl(int code, const char *file, int line, const char *func, const char *fmt, ...) 
{
    va_list args;
    va_start(args, fmt);
    vsnprintf(z_err_buf, sizeof(z_err_buf), fmt, args);
    va_end(args);
    return (zerr)
    { 
        .code = code, 
        .msg = z_err_buf, 
        .file = file, 
        .line = line, 
        .func = func, 
        .source = NULL 
    };
}

zerr zerr_errno_impl(int code, const char *file, int line, const char *func, const char *fmt, ...) 
{
    char temp[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(temp, sizeof(temp), fmt, args);
    va_end(args);
    snprintf(z_err_buf, sizeof(z_err_buf), "%s: %s", temp, strerror(errno));
    return (zerr)
    { 
        .code = code,
         .msg = z_err_buf, 
         .file = file, 
         .line = line, 
         .func = func, 
         .source = NULL 
        };
}

zerr zerr_add_trace(zerr e, const char *func, const char *file, int line) 
{
    char combined[2048];
    const char *msg = e.msg ? e.msg : "Unknown Error";
    snprintf(combined, sizeof(combined), "%s\n    at %s (%s:%d)", msg, func, file, line);
    snprintf(z_err_buf, sizeof(z_err_buf), "%s", combined);
    e.msg = z_err_buf;
    return e;
}

zerr zerr_wrap(zerr e, const char *fmt, ...) 
{ 
    char new_msg[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(new_msg, sizeof(new_msg), fmt, args);
    va_end(args);
    
    char combined[2048];
    snprintf(combined, sizeof(combined), "%s: %s", new_msg, e.msg);
    snprintf(z_err_buf, sizeof(z_err_buf), "%s", combined);
    e.msg = z_err_buf;
    return e; 
} 

int zerr_run(zres result) 
{
    if (!result.is_ok) 
    {
        zerr_print(result.err);
        return 1;
    }
    return 0;
}

#endif // ZERROR_IMPLEMENTATION_GUARD
#endif // ZERROR_IMPLEMENTATION
