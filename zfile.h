/*
 * GENERATED FILE - DO NOT EDIT DIRECTLY
 * Source: zfile.c
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
 * zfile.h — Cross-platform File I/O and Path Manipulation
 * Part of Zen Development Kit (ZDK)
 *
 * A single-header library for easy file system operations.
 * Wraps Windows/POSIX differences, provides buffered reading,
 * and integrates with zstr.h for path manipulation.
 *
 * Usage:
 * #define ZFILE_IMPLEMENTATION
 * #include "zfile.h"
 *
 * License: MIT
 * Author: Zuhaitz
 * Repository: https://github.com/z-libs/zfile.h
 * Version: 1.0.1
 */

#ifndef ZFILE_H
#define ZFILE_H

#include "zstr.h"
// [Bundled] "zcommon.h" is included inline in this same file

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/stat.h>

#ifdef _WIN32
#   define ZFILE_SEP '\\'
#   define ZFILE_SEP_S "\\"
#else
#   define ZFILE_SEP '/'
#   define ZFILE_SEP_S "/"
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Path manipulation.

bool zfile_exists(const char *path);
bool zfile_is_dir(const char *path);
bool zfile_is_file(const char *path);

zstr zfile_join(const char *base, const char *child);
zstr_view zfile_ext(const char *path);
zstr_view zfile_base(const char *path);
zstr_view zfile_dir(const char *path);
void zfile_normalize(zstr *path);

// Generates a temporary filename in system temp dir.
zstr zfile_tempname(const char *prefix, const char *ext);

// File I/O.

zstr zfile_read_all(const char *path);
int zfile_write_all(const char *path, const void *data, size_t len);
int zfile_append(const char *path, const void *data, size_t len);
int zfile_copy(const char *src, const char *dst);
int zfile_remove(const char *path);
int zfile_rename(const char *old_path, const char *new_path);
int zfile_mkdir_recursive(const char *path);

int64_t zfile_size(const char *path);

// Atomic save: Writes to a sibling temp file, then renames.
// Prevents data corruption on crash.
int zfile_save_atomic(const char *path, const void *data, size_t len);

// Buffered reader.

typedef struct 
{
    FILE *handle;
    char *buffer;
    size_t cap;
    size_t len;     
    size_t pos;     
    bool   eof;
    bool   own_buf; 
} zfile_reader;

zfile_reader zfile_reader_open(const char *path);
zfile_reader zfile_reader_open_buf(const char *path, char *buf, size_t cap);
void zfile_reader_close(zfile_reader *r);

bool zfile_reader_next_line(zfile_reader *r, zstr_view *out_line);

/* * Macro: ZFILE_FOR_EACH_LINE
 * Usage:
 * ZFILE_FOR_EACH_LINE("file.txt", line) { ... }
 */
#define ZFILE_FOR_EACH_LINE(path, line_var)                                             \
    for (zfile_reader _r_##line_var = zfile_reader_open(path);                          \
         _r_##line_var.handle != NULL;                                                  \
         zfile_reader_close(&_r_##line_var))                                            \
        for (zstr_view line_var; zfile_reader_next_line(&_r_##line_var, &line_var); )

// Directory iteration.

typedef enum
{ 
    ZDIR_FILE, 
    ZDIR_DIR, 
    ZDIR_UNKNOWN 
} zdir_type;

typedef struct 
{
    char name[512]; 
    zdir_type type;
} zdir_entry;

// Opaque iterator handle.
typedef struct zdir_iter_s zdir_iter;

zdir_iter* zdir_open(const char *path);
bool zdir_next(zdir_iter *it, zdir_entry *out_entry);
void zdir_close(zdir_iter *it);

// Optional short names.
#ifdef ZFILE_SHORT_NAMES
#   define file_exists   zfile_exists
#   define file_is_dir   zfile_is_dir
#   define file_is_file  zfile_is_file
#   define path_join     zfile_join
#   define file_read     zfile_read_all
#   define file_write    zfile_write_all
#   define file_remove   zfile_remove
#   define dir_open      zdir_open
#   define dir_next      zdir_next
#   define dir_close     zdir_close
#   define file_foreach_line ZFILE_FOR_EACH_LINE
#endif

#ifdef __cplusplus
} // extern "C"
#endif

// C++ API.

#ifdef __cplusplus
#include <memory>
namespace z_file 
{
    class path 
    {
        z_str::string s;
     public:
        path(const char *p) : s(p) {}
        path(const z_str::string &p) : s(p) {}
        path operator/(const char *other) const
        {
            zstr temp = zfile_join(s.c_str(), other);
            path ret(zstr_cstr(&temp));
            zstr_free(&temp);
            return ret;
        }

        path operator/(const path &other) const 
        { 
            return *this / other.s.c_str(); 
        }

        bool exists() const 
        { 
            return zfile_exists(s.c_str()); 
        }

        bool is_dir() const 
        { 
            return zfile_is_dir(s.c_str()); 
        }

        z_str::view extension() const 
        { 
            zstr_view v = zfile_ext(s.c_str()); 
            return z_str::view(v.data, v.len); 
        }

        const z_str::string &string() const 
        { 
            return s; 
        }

        const char *c_str() const 
        { 
            return s.c_str(); 
        }

        operator const char*() const 
        { 
            return s.c_str(); 
        }
    };
    class dir_iterable 
    {
        zstr_view root;
     public:
        dir_iterable(const char *p) : root(zstr_view_from(p)) {}
        struct iterator 
        {
            std::shared_ptr<zdir_iter> handle;
            zdir_entry current;
            bool done;
            
            iterator(const char *p) : done(false) 
            {
                if (p) 
                {
                    zdir_iter *raw = zdir_open(p);
                    if (raw) 
                    {
                        handle = std::shared_ptr<zdir_iter>(raw, zdir_close);
                        next();
                    } 
                    else 
                    {
                        done = true;
                    }
                } 
                else 
                {
                    done = true;
                }
            }
            
            void next() 
            {
                if (!handle) 
                { 
                    done = true; 
                    return; 
                }
                memset(&current, 0, sizeof(current));
                
                if (!zdir_next(handle.get(), &current)) 
                {
                    done = true;
                    handle.reset();
                }
            }

            iterator &operator++() 
            { 
                next(); 
                return *this; 
            }

            bool operator!=(const iterator &other) const 
            { 
                return done != other.done; 
            }

            const zdir_entry &operator*() const 
            { 
                return current; 
            }
        };
        iterator begin() 
        { 
            return iterator(root.data); 
        }

        iterator end() 
        { 
            return iterator(NULL); 
        }
    };
}
#endif // __cplusplus

#endif // ZFILE_H

#ifdef ZFILE_IMPLEMENTATION

// Feature test macros for Linux/POSIX (Must be first).
#if !defined(_DEFAULT_SOURCE)
#   define _DEFAULT_SOURCE
#endif
#if !defined(_BSD_SOURCE)
#   define _BSD_SOURCE
#endif
#if !defined(_POSIX_C_SOURCE)
#   define _POSIX_C_SOURCE 200809L
#endif

#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <string.h>

#ifdef _WIN32
#   include <windows.h>
#   include <direct.h>
#   define z_stat_struct _stat
#else
#   include <dirent.h>
#   include <unistd.h>
#   define z_stat_struct stat
#   define zfile__fopen fopen
#endif

// Internal utilities.

static uint32_t zfile__rng_state = 0;

static inline uint32_t zfile__rand(void) 
{
    if (0 == zfile__rng_state) 
    {
        zfile__rng_state = (uint32_t)time(NULL) ^ (uint32_t)(uintptr_t)&zfile__rng_state;
    }
    // XORShift32.
    uint32_t x = zfile__rng_state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    zfile__rng_state = x;
    return x;
}

#ifdef __cplusplus
extern "C" {
#endif

// Win32 Helper.
#ifdef _WIN32
    static inline wchar_t* zfile__to_wstr(const char *utf8) 
    {
        if (!utf8) 
        {
            return NULL;
        }
        int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
        if (len <= 0) 
        {
            return NULL;
        }
        wchar_t *wbuf = (wchar_t*)Z_MALLOC(len * sizeof(wchar_t));
        MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wbuf, len);
        return wbuf;
    }
    
    static FILE* zfile__fopen(const char *path, const char *mode) 
    {
        wchar_t *wpath = zfile__to_wstr(path);
        wchar_t *wmode = zfile__to_wstr(mode);
        FILE *f = _wfopen(wpath, wmode);
        Z_FREE(wpath); Z_FREE(wmode);
        return f;
    }
#endif

// Path utils.

bool zfile_exists(const char *path)
{
    struct z_stat_struct s;
#   ifdef _WIN32
    wchar_t *wpath = zfile__to_wstr(path);
    int res = _wstat(wpath, &s);
    Z_FREE(wpath);
    return 0 == res;
#   else
    return 0 == stat(path, &s);
#   endif
}

bool zfile_is_dir(const char *path)
{
    struct z_stat_struct s;
#   ifdef _WIN32
    wchar_t *wpath = zfile__to_wstr(path);
    int res = _wstat(wpath, &s);
    Z_FREE(wpath);
    if (0 != res)
    {
        return false;
    }
    return (0 != (s.st_mode & S_IFDIR));
#   else
    if (0 != stat(path, &s))
    {
        return false;
    }
    return S_ISDIR(s.st_mode);
#   endif
}

bool zfile_is_file(const char *path)
{
    return zfile_exists(path) && !zfile_is_dir(path);
}

zstr zfile_join(const char *base, const char *child)
{
    zstr s = zstr_init();
    zstr_cat(&s, base);

    if (!zstr_is_empty(&s)) 
    {
        char last = zstr_cstr(&s)[zstr_len(&s) - 1]; 
        if ('/' == last || '\\' == last)
        {
            zstr_pop_char(&s); 
        }
    }
    while ('/' == *child || '\\' == *child)
    {
        child++;
    }
    zstr_push(&s, ZFILE_SEP);
    zstr_cat(&s, child);
    return s;
}

zstr_view zfile_ext(const char *path)
{
    zstr_view v = zstr_view_from(path);
    if (0 == v.len)
    {
        return (zstr_view){"", 0};
    }
    
    for (size_t i = v.len; i > 0; i--) 
    {
        char c = v.data[i - 1];
        if ('.' == c) 
        {
            return zstr_sub(v, i - 1, v.len - (i - 1));
        }
        if ('/' == c || '\\' == c)
        {
            break; 
        }
    }
    return (zstr_view){"", 0};
}

zstr_view zfile_base(const char *path)
{
    zstr_view v = zstr_view_from(path);
    const char *sep = strrchr(v.data, ZFILE_SEP);
#   ifdef _WIN32
    const char *alt = strrchr(v.data, '/');
    if (alt > sep)
    {
        sep = alt;
    }
#   endif
    if (!sep)
    {
        return v;
    }
    size_t offset = (sep - v.data) + 1;
    return zstr_sub(v, offset, v.len - offset);
}

zstr_view zfile_dir(const char *path)
{
    zstr_view v = zstr_view_from(path);
    const char *sep = strrchr(v.data, ZFILE_SEP);
#   ifdef _WIN32
    const char *alt = strrchr(v.data, '/');
    if (alt > sep)
    {
        sep = alt;
    }
#   endif
    if (!sep)
    {
        return (zstr_view){".", 1};
    }
    size_t len = sep - v.data;
    if (0 == len)
    {
        len = 1;
    }
    return zstr_sub(v, 0, len);
}

void zfile_normalize(zstr *path)
{
    char *p = zstr_data(path);
    size_t len = zstr_len(path);
    for (size_t i = 0; i < len; i++) 
    {
#       ifdef _WIN32
        if ('/' == p[i]) 
        {
        p[i] = '\\';
        }
#       else
        if ('\\' == p[i]) 
        {
            p[i] = '/';
        }
#       endif
    }
}

zstr zfile_tempname(const char *prefix, const char *ext)
{
    uint32_t r = zfile__rand();
    zstr s = zstr_init();
#   ifdef _WIN32
    char tmp[MAX_PATH];
    if (GetTempPathA(MAX_PATH, tmp)) 
    {
        zstr_cat(&s, tmp);
    }
#   else
    const char *env = getenv("TMPDIR");
    if (env)
    {
        zstr_cat(&s, env);
    }
    else 
    {
        zstr_cat(&s, "/tmp/");
    }
#   endif

    if (!zstr_is_empty(&s)) 
    {
        char last = zstr_cstr(&s)[zstr_len(&s)-1];
        if (last != '/' && last != '\\') 
        {
            zstr_push(&s, ZFILE_SEP);
        }
    }
    if (prefix)
    {
        zstr_cat(&s, prefix);
    }
    zstr_fmt(&s, "_%x", r);
    if (ext)
    {
        if (ext[0] != '.')
        {
            zstr_push(&s, '.');
        }
        zstr_cat(&s, ext);
    }
    return s;
}

int64_t zfile_size(const char *path)
{
    struct z_stat_struct s;
#   ifdef _WIN32
    wchar_t *wpath = zfile__to_wstr(path);
    int res = _wstat(wpath, &s);
    Z_FREE(wpath);
#   else
    int res = stat(path, &s);
#   endif
    if (res != 0)
    {
        return -1;
    }
    return (int64_t)s.st_size;
}

int zfile_mkdir_recursive(const char *path)
{
    zstr p = zstr_init();
    zstr_cat(&p, path);

    size_t len = zstr_len(&p);
    char *str = zstr_data(&p);
    
    for (size_t i = 0; i < len; i++) 
    {
        if ('/' == str[i] || '\\' == str[i]) 
        {
            if (0 == i) 
            {
                continue; 
            }
            str[i] = '\0';
            if (!zfile_exists(str)) 
            {
#               ifdef _WIN32
                wchar_t *wstr = zfile__to_wstr(str);
                int r = _wmkdir(wstr);
                free(wstr);
                if (0 != r && EEXIST != errno)
                { 
                    zstr_free(&p);
                    return -1;
                }
#               else
                if (0 != mkdir(str, 0755) && EEXIST != errno)
                { 
                    zstr_free(&p);
                    return -1;
                }
#               endif
            }
            str[i] = ZFILE_SEP; 
        }
    }
    
    if (!zfile_exists(str)) 
    {
#       ifdef _WIN32
        wchar_t *wstr = zfile__to_wstr(str);
        int r = _wmkdir(wstr);
        free(wstr);
        if (0 != r && EEXIST != errno) 
        { 
            zstr_free(&p); 
            return -1; 
        }
#       else
        if (0 != mkdir(str, 0755) && EEXIST != errno) 
        { 
            zstr_free(&p); 
            return -1; 
        }
#       endif
    }
    zstr_free(&p);
    return 0;
}

// IO utils.

zstr zfile_read_all(const char *path)
{
    FILE *f = zfile__fopen(path, "rb");
    if (!f) 
    {
        return zstr_init();
    }

    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);

    zstr s = zstr_init();
    if (len > 0) 
    {
        if (Z_OK == zstr_reserve(&s, len)) 
        {
            size_t n = fread(zstr_data(&s), 1, len, f);
            if (s.is_long)
            { 
                s.l.len = n; 
                s.l.ptr[n] = '\0'; 
            }
            else 
            { 
                s.s.len = (uint8_t)n; 
                s.s.buf[n] = '\0'; 
            }
        }
    } 
    else 
    {
        char buf[1024];
        size_t n;
        while ((n = fread(buf, 1, sizeof(buf), f)) > 0) 
        {
            zstr_cat_len(&s, buf, n);
        }
    }
    fclose(f);
    return s;
}

static int zfile__write_mode(const char *path, const void *data, size_t len, const char *mode)
{
    FILE *f = zfile__fopen(path, mode);
    if (!f) 
    {
        return errno;
    }
    size_t written = fwrite(data, 1, len, f);
    fclose(f);
    return (written == len) ? 0 : EIO;
}

int zfile_write_all(const char *path, const void *data, size_t len)
{
    return zfile__write_mode(path, data, len, "wb");
}

int zfile_append(const char *path, const void *data, size_t len)
{
    return zfile__write_mode(path, data, len, "ab");
}

int zfile_copy(const char *src, const char *dst)
{
    FILE *fs = zfile__fopen(src, "rb");
    if (!fs) 
    {
        return errno;
    }
    FILE *fd = zfile__fopen(dst, "wb");
    if (!fd) 
    { 
        fclose(fs); 
        return errno; 
    }
    char buf[4096]; size_t n; int err = 0;
    while ((n = fread(buf, 1, sizeof(buf), fs)) > 0) 
    {
        if (fwrite(buf, 1, n, fd) != n) 
        { 
            err = EIO; 
            break; 
        }
    }
    fclose(fs); fclose(fd);
    return err;
}

int zfile_remove(const char *path)
{
#   ifdef _WIN32
    wchar_t *wpath = zfile__to_wstr(path);
    int res = _wremove(wpath);
    free(wpath);
    return res == 0 ? 0 : errno;
#   else
    return remove(path) == 0 ? 0 : errno;
#   endif
}

int zfile_rename(const char *old_path, const char *new_path)
{
#   ifdef _WIN32
    wchar_t *wold = zfile__to_wstr(old_path);
    wchar_t *wnew = zfile__to_wstr(new_path);
    int res = MoveFileExW(wold, wnew, MOVEFILE_REPLACE_EXISTING) ? 0 : -1;
    free(wold); free(wnew);
    return res;
#   else
    return rename(old_path, new_path) == 0 ? 0 : errno;
#   endif
}

int zfile_save_atomic(const char *path, const void *data, size_t len)
{
    zstr tmp = zstr_init();
    zstr_cat(&tmp, path);
    zstr_cat(&tmp, ".tmp_");
    
    uint32_t r = zfile__rand();
    zstr_fmt(&tmp, "%x", r);

    int res = zfile_write_all(zstr_cstr(&tmp), data, len);
    
    if (0 != res)
    {
        zfile_remove(zstr_cstr(&tmp));
        zstr_free(&tmp);
        return res;
    }
    
    res = zfile_rename(zstr_cstr(&tmp), path);
    zstr_free(&tmp);
    return res;
}

// Buffered reader.

#define ZFILE_DEFAULT_CAP 4096
zfile_reader zfile_reader_open_buf(const char *path, char *buf, size_t cap) {
    zfile_reader r;
    memset(&r, 0, sizeof(r)); 
    
    r.handle = zfile__fopen(path, "rb");
    if (!r.handle) 
    {
        return r;
    }
    r.buffer = buf; 
    r.cap = cap; 
    r.own_buf = false;
    return r;
}

zfile_reader zfile_reader_open(const char *path) 
{
    char *buf = (char*)Z_MALLOC(ZFILE_DEFAULT_CAP);
    zfile_reader r = zfile_reader_open_buf(path, buf, ZFILE_DEFAULT_CAP);
    if (!r.handle) 
    { 
        Z_FREE(buf); 
        return r; 
    }
    r.own_buf = true;
    return r;
}

void zfile_reader_close(zfile_reader *r) 
{
    if (r->handle) 
    {
        fclose(r->handle);
    }
    if (r->own_buf && r->buffer) 
    {
        Z_FREE(r->buffer);
    }
    memset(r, 0, sizeof(zfile_reader));
}

bool zfile_reader_next_line(zfile_reader *r, zstr_view *out_line) 
{
    if (!r->handle) 
    {
        return false;
    }
    *out_line = (zstr_view){ NULL, 0 };
    while (1) 
    {
        char *start = r->buffer + r->pos;
        char *end = r->buffer + r->len;
        char *newline = (char*)memchr(start, '\n', end - start);
        if (newline) 
        {
            size_t line_len = newline - start;
            size_t view_len = line_len;
            if (view_len > 0 && '\r' == start[view_len - 1]) 
            {
                view_len--;
            }
            *out_line = (zstr_view){ start, view_len };
            r->pos += line_len + 1;
            return true;
        }
        if (r->eof) 
        {
            if (r->pos < r->len) 
            {
                *out_line = (zstr_view){ start, r->len - r->pos };
                r->pos = r->len;
                return true;
            }
            return false;
        }
        size_t leftover = r->len - r->pos;
        if (leftover > 0 && r->pos > 0) 
        {
            memmove(r->buffer, r->buffer + r->pos, leftover);
        }
        r->len = leftover; r->pos = 0;
        if (r->len == r->cap) 
        {
            *out_line = (zstr_view){ r->buffer, r->cap };
            r->pos = r->cap;
            return true; 
        }
        size_t read = fread(r->buffer + r->len, 1, r->cap - r->len, r->handle);
        if (0 == read) 
        { 
            r->eof = true; 
            continue; 
        }
        r->len += read;
    }
}

// Directory iteration.

struct zdir_iter_s 
{
#   ifdef _WIN32
    HANDLE hFind;
    WIN32_FIND_DATAW ffd; 
    bool first;
#   else
    DIR *dir;
#   endif
};

zdir_iter* zdir_open(const char *path) 
{
    zdir_iter *it = (zdir_iter*)Z_MALLOC(sizeof(zdir_iter));
    if (!it) 
    {
        return NULL;
    }
#   ifdef _WIN32
    zstr search_path = zfile_join(path, "*");
    wchar_t *wpath = zfile__to_wstr(zstr_cstr(&search_path));
    it->hFind = FindFirstFileW(wpath, &it->ffd); 
    free(wpath); 
    zstr_free(&search_path);
    if (INVALID_HANDLE_VALUE == it->hFind) 
    { 
        Z_FREE(it); 
        return NULL; 
    }
    it->first = true;
#   else
    it->dir = opendir(path);
    if (!it->dir) 
    { 
        Z_FREE(it); 
        return NULL; 
    }
#   endif
    return it;
}

bool zdir_next(zdir_iter *it, zdir_entry *out) 
{
    if (!it) 
    {
        return false;
    }
#   ifdef _WIN32
    if (!it->first) 
    { 
        if (!FindNextFileW(it->hFind, &it->ffd)) 
        {
            return false; 
        }
    }
    it->first = false;
    if (0 == wcscmp(it->ffd.cFileName, L".") || 0 == wcscmp(it->ffd.cFileName, L"..")) 
    {
        return zdir_next(it, out);
    }
    WideCharToMultiByte(CP_UTF8, 0, it->ffd.cFileName, -1, out->name, sizeof(out->name), NULL, NULL);
    out->type = (it->ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? ZDIR_DIR : ZDIR_FILE;
    return true;
#   else
    struct dirent *ent;
    while (NULL != (ent = readdir(it->dir))) 
    {
        if (0 == strcmp(ent->d_name, ".") || 0 == strcmp(ent->d_name, "..")) 
        {
            continue;
        }
        strncpy(out->name, ent->d_name, sizeof(out->name)-1);
#       if defined(_DIRENT_HAVE_D_TYPE) && defined(DT_DIR)
        if (DT_DIR == ent->d_type) 
        {
            out->type = ZDIR_DIR;
        }
        else if (DT_REG == ent->d_type)
        {
            out->type = ZDIR_FILE;
        }
        else 
        {
            out->type = ZDIR_UNKNOWN;
        }
#       else
        out->type = ZDIR_UNKNOWN; 
#       endif
        return true;
    }
    return false;
#   endif
}

void zdir_close(zdir_iter *it) 
{
    if (!it) 
    {
        return;
    }
#   ifdef _WIN32
    FindClose(it->hFind);
#   else
    closedir(it->dir);
#   endif
    Z_FREE(it);
}

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ZFILE_IMPLEMENTATION
