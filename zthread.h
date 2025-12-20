/*
 * GENERATED FILE - DO NOT EDIT DIRECTLY
 * Source: zthread.c
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
 * zthread.h — Portable threads, mutexes, and synchronization primitives
 * Part of Zen Development Kit (ZDK)
 *
 * A single-header, cross-platform threading library that abstracts Win32 and
 * POSIX threads into a unified, type-safe C API. It requires zero configuration
 * and works out of the box on Windows, Linux, and macOS.
 *
 * Features:
 * • Native Win32 and POSIX (pthread) backends.
 * • Type-safe thread creation macros (zthread_create).
 * • Unified Mutex and Condition Variable primitives.
 * • Optional short names via ZTHREAD_SHORT_NAMES.
 * • Full C++ RAII wrappers (z_thread::thread, z_thread::mutex).
 * • Zero dependencies (only standard system headers).
 *
 * License: MIT
 * Author: Zuhaitz
 * Repository: https://github.com/z-libs/zthread.h
 * Version: 1.1.2
 */

#ifndef ZTHREAD_H
#define ZTHREAD_H
// [Bundled] "zcommon.h" is included inline in this same file

#ifdef _WIN32
#   include <windows.h>
#   include <process.h>
    typedef HANDLE zthread_t;
    typedef CRITICAL_SECTION zmutex_t;
    typedef CONDITION_VARIABLE zcond_t;
    // Internal Windows thread signature.
#   define ZTHREAD_Func unsigned __stdcall
#else
#   include <pthread.h>
#   include <unistd.h>
#   include <time.h>
    typedef pthread_t zthread_t;
    typedef pthread_mutex_t zmutex_t;
    typedef pthread_cond_t zcond_t;
    // Internal POSIX thread signature.
#   define ZTHREAD_Func void*
#endif

// C++ moment.
#ifdef __cplusplus
extern "C" {
#endif

// Allocator overrides (user may redefine).
#ifndef ZTHREAD_MALLOC
    #define ZTHREAD_MALLOC(sz)      Z_MALLOC(sz)
#endif

#ifndef ZTHREAD_CALLOC
    #define ZTHREAD_CALLOC(n, sz)   Z_CALLOC(n, sz)
#endif

#ifndef ZTHREAD_REALLOC
    #define ZTHREAD_REALLOC(p, sz)  Z_REALLOC(p, sz)
#endif

#ifndef ZTHREAD_FREE
    #define ZTHREAD_FREE(p)         Z_FREE(p)
#endif

// The raw function signature required by the OS.
typedef void (*zthread_proxy_fn)(void* arg);

// Our API.

// Internal raw creation function. Returns Z_OK on success.
int zthread__create_ptr(zthread_t *t, zthread_proxy_fn func, void *arg);

/* * Type-safe creation macro.
 * Automatically casts the function and argument to void*.
 * Usage: zthread_create(&t, my_func, &my_data);
*/
#define zthread_create(t, func, arg) \
    zthread__create_ptr((t), (zthread_proxy_fn)(func), (void*)(arg))


/* * Strict wrapper macro.
 * Generates a void* proxy to ensure strict standard compliance.
 * Usage: ZTHREAD_WRAP(my_worker, int*, count) { ... }
*/
#define ZTHREAD_WRAP(name, type, var)                               \
    void name##__impl(type var);                                    \
    void name(void* _void_arg) { name##__impl((type)_void_arg); }   \
    void name##__impl(type var)


// Thread control.
void zthread_join(zthread_t t);
void zthread_detach(zthread_t t);
void zthread_sleep(int ms);

// Mutexes.
void zmutex_init(zmutex_t *m);
void zmutex_lock(zmutex_t *m);
void zmutex_unlock(zmutex_t *m);
void zmutex_destroy(zmutex_t *m);

// Condition variables.
void zcond_init(zcond_t *c);
void zcond_wait(zcond_t *c, zmutex_t *m);
void zcond_signal(zcond_t *c);
void zcond_broadcast(zcond_t *c);
void zcond_destroy(zcond_t *c);

// Short names (optional).
#ifdef ZTHREAD_SHORT_NAMES
    typedef zthread_t   thread_t;
    typedef zmutex_t    mutex_t;
    typedef zcond_t     cond_t;

#   define thread_create   zthread_create
#   define thread_join     zthread_join
#   define thread_detach   zthread_detach
#   define thread_sleep    zthread_sleep
    
    // Macro wrapper alias.
#   define THREAD_WRAP     ZTHREAD_WRAP

#   define mutex_init      zmutex_init
#   define mutex_lock      zmutex_lock
#   define mutex_unlock    zmutex_unlock
#   define mutex_destroy   zmutex_destroy

#   define cond_init       zcond_init
#   define cond_wait       zcond_wait
#   define cond_signal     zcond_signal
#   define cond_broadcast  zcond_broadcast
#   define cond_destroy    zcond_destroy
#endif

#ifdef __cplusplus
}
#endif

// namespace: z_thread.
#ifdef __cplusplus

#include <utility>
#include <exception>
#include <functional>

namespace z_thread 
{
    class mutex 
    {
        ::zmutex_t inner;
        friend class cond;

     public:
        // RAII: Initialize on construction.
        mutex() 
        { 
            ::zmutex_init(&inner); 
        }

        // RAII: Destroy on destruction.
        ~mutex() 
        { 
            ::zmutex_destroy(&inner); 
        }

        // Non-copyable.
        mutex(const mutex&) = delete;
        mutex &operator=(const mutex&) = delete;

        void lock() 
        { 
            ::zmutex_lock(&inner); 
        }

        void unlock() 
        { 
            ::zmutex_unlock(&inner); 
        }
        
        // Raw access if needed.
        ::zmutex_t *native_handle() 
        { 
            return &inner; 
        }
    };

    // Simple RAII lock guard (scoped lock).
    class lock_guard 
    {
        mutex& m_mutex;
     public:
        explicit lock_guard(mutex& m) : m_mutex(m) 
        { 
            m_mutex.lock(); 
        }

        ~lock_guard() 
        { 
            m_mutex.unlock(); 
        }
        
        // Non-copyable.
        lock_guard(const lock_guard&) = delete;
        lock_guard &operator=(const lock_guard&) = delete;
    };

    class cond 
    {
        ::zcond_t inner;

     public:
        cond() 
        { 
            ::zcond_init(&inner); 
        }

        ~cond() 
        { 
            ::zcond_destroy(&inner); 
        }

        // Non-copyable.
        cond(const cond&) = delete;
        cond &operator=(const cond&) = delete;

        void wait(mutex &m) 
        { 
            ::zcond_wait(&inner, &m.inner); 
        }

        void signal() 
        { 
            ::zcond_signal(&inner); 
        }

        void broadcast() 
        { 
            ::zcond_broadcast(&inner);
        }
        
        ::zcond_t *native_handle() 
        { 
            return &inner; 
        }
    };

    class thread 
    {
        ::zthread_t inner;
        bool joinable;

        // Internal proxy for C++ lambdas/functions.
        struct impl_base 
        { 
            virtual void run() = 0; 
            virtual ~impl_base() {} 
        };
        
        template <typename Func>
        struct impl_type : impl_base 
        {
            Func f;
            impl_type(Func &&func) : f(std::forward<Func>(func)) {}
            void run() override { f(); }
        };

        // Static thunk matching zthread_proxy_fn signature (void(*)(void*)).
        static void proxy_thunk(void *arg) 
        {
            impl_base *p = static_cast<impl_base*>(arg);
            p->run();
            delete p;
        }

    public:
        thread() : joinable(false) 
        {
#           ifdef _WIN32
            inner = NULL;
#           else
            inner = 0;
#           endif
        }

        // Move constructor.
        thread(thread &&other) noexcept : inner(other.inner), joinable(other.joinable) 
        {
            other.joinable = false;
#           ifdef _WIN32
            other.inner = NULL;
#           else
            other.inner = 0;
#           endif
        }

        thread &operator=(thread &&other) noexcept 
        {
            if (joinable) 
            {
                std::terminate();
            }
            inner = other.inner;
            joinable = other.joinable;
            other.joinable = false;
            return *this;
        }

        // Standard thread constructor (Function + Args).
        // Usage: z_thread::thread t([]{ printf("Hello"); });
        template <typename Function, typename... Args>
        explicit thread(Function &&f, Args&&... args) 
        {
            auto bound_func = std::bind(std::forward<Function>(f), std::forward<Args>(args)...);

            impl_base* p = new impl_type<decltype(bound_func)>(std::move(bound_func));
            
            if (::zthread__create_ptr(&inner, proxy_thunk, p) == Z_OK) 
            {
                joinable = true;
            } 
            else 
            {
                delete p; // Cleanup if creation failed.
                joinable = false;
            }
        }

        ~thread() 
        {
            if (joinable) 
            {
                std::terminate();
            }
        }

        void join() 
        {
            if (joinable) 
            {
                ::zthread_join(inner);
                joinable = false;
            }
        }

        void detach() 
        {
            if (joinable) 
            {
                ::zthread_detach(inner);
                joinable = false;
            }
        }

        bool joinable_state() const 
        { 
            return joinable; 
        }

        ::zthread_t native_handle() 
        { 
            return inner; 
        }
        
        static void sleep(int ms) 
        { 
            ::zthread_sleep(ms); 
        }
    };
}

#endif // __cplusplus

#endif // ZTHREAD_H

#ifdef ZTHREAD_IMPLEMENTATION
#ifndef ZTHREAD_IMPLEMENTATION_GUARD
#define ZTHREAD_IMPLEMENTATION_GUARD

struct zthread__wrap 
{ 
    zthread_proxy_fn f; 
    void *arg; 
};

#ifdef _WIN32
static unsigned __stdcall zthread__proxy_entry(void *p) 
{
    struct zthread__wrap *w = (struct zthread__wrap*)p;
    w->f(w->arg); 
    ZTHREAD_FREE(w); 
    return 0;
}
int zthread__create_ptr(zthread_t *t, zthread_proxy_fn func, void *arg) 
{
    struct zthread__wrap *w = (struct zthread__wrap*)ZTHREAD_MALLOC(sizeof(*w)); 
    if (!w) 
    {
        return Z_ENOMEM;
    }
    w->f = func; 
    w->arg = arg;
    *t = (HANDLE)_beginthreadex(NULL, 0, zthread__proxy_entry, w, 0, NULL);
    
    if (NULL == *t) 
    {
        ZTHREAD_FREE(w);
        return Z_ERR;
    }
    return Z_OK;
}

void zthread_join(zthread_t t) 
{ 
    WaitForSingleObject(t, INFINITE); 
    CloseHandle(t); 
}

void zthread_detach(zthread_t t) 
{ 
    CloseHandle(t); 
}

void zthread_sleep(int ms) 
{ 
    Sleep(ms); 
}

void zmutex_init(zmutex_t *m) 
{ 
    InitializeCriticalSection(m); 
}

void zmutex_lock(zmutex_t *m) 
{ 
    EnterCriticalSection(m); 
}

void zmutex_unlock(zmutex_t *m) 
{ 
    LeaveCriticalSection(m); 
}

void zmutex_destroy(zmutex_t *m) 
{ 
    DeleteCriticalSection(m); 
}

void zcond_init(zcond_t *c) 
{ 
    InitializeConditionVariable(c); 
}
void zcond_wait(zcond_t *c, zmutex_t *m) 
{ 
    SleepConditionVariableCS(c, m, INFINITE); 
}

void zcond_signal(zcond_t *c) 
{ 
    WakeConditionVariable(c); 
}

void zcond_broadcast(zcond_t *c) 
{ 
    WakeAllConditionVariable(c); 
}

void zcond_destroy(zcond_t *c) 
{ 
    (void)c; 
} 

#else
// POSIX implementation.

static void* zthread__proxy_entry(void *p) 
{
    struct zthread__wrap *w = (struct zthread__wrap*)p;
    w->f(w->arg); 
    ZTHREAD_FREE(w); 
    return NULL;
}

int zthread__create_ptr(zthread_t *t, zthread_proxy_fn func, void *arg) 
{
    struct zthread__wrap *w = (struct zthread__wrap*)ZTHREAD_MALLOC(sizeof(*w));
    if (!w) 
    {
        return Z_ENOMEM; 
    }
    w->f = func; 
    w->arg = arg;

    if (0 != pthread_create(t, NULL, zthread__proxy_entry, w)) 
    {
        ZTHREAD_FREE(w);
        return Z_ERR;
    }
    return Z_OK;
}

void zthread_join(zthread_t t) 
{ 
    pthread_join(t, NULL); 
}

void zthread_detach(zthread_t t) 
{ 
    pthread_detach(t); 
}

void zthread_sleep(int ms) 
{ 
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

void zmutex_init(zmutex_t *m) 
{ 
    pthread_mutex_init(m, NULL); 
}
void zmutex_lock(zmutex_t *m) 
{ 
    pthread_mutex_lock(m); 
}

void zmutex_unlock(zmutex_t *m) 
{ 
    pthread_mutex_unlock(m); 
}

void zmutex_destroy(zmutex_t *m) 
{ 
    pthread_mutex_destroy(m); 
}

void zcond_init(zcond_t *c) 
{ 
    pthread_cond_init(c, NULL); 
}

void zcond_wait(zcond_t *c, zmutex_t *m) 
{ 
    pthread_cond_wait(c, m); 
}

void zcond_signal(zcond_t *c) 
{ 
    pthread_cond_signal(c); 
}

void zcond_broadcast(zcond_t *c) 
{ 
    pthread_cond_broadcast(c); 
}

void zcond_destroy(zcond_t *c) 
{ 
    pthread_cond_destroy(c); 
}
#endif

#endif // ZTHREAD_IMPLEMENTATION_GUARD

#endif // ZTHREAD_IMPLEMENTATION
