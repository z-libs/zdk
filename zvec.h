/*
 * GENERATED FILE - DO NOT EDIT DIRECTLY
 * Source: zvec.c
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
 */

#ifndef ZCOMMON_H
#define ZCOMMON_H

#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

// Return codes and error handling.

// Success.
#define Z_OK          0
#define Z_FOUND       1   // Element found (positive).

// Generic errors.
#define Z_ERR        -1   // Generic error.

// Resource errors.
#define Z_ENOMEM     -2   // Out of memory (malloc/realloc failed).

// Access errors.
#define Z_EOOB       -3   // Out of bounds / range error.
#define Z_EEMPTY     -4   // Container is empty.
#define Z_ENOTFOUND  -5   // Element not found.

// Logic errors.
#define Z_EINVAL     -6   // Invalid argument / parameter.
#define Z_EEXIST     -7   // Element already exists (for example, unique keys).

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

/* * We check for GCC/Clang features to enable RAII-style cleanup and optimization hints.
 * Define Z_NO_EXTENSIONS to disable this manually.
 */
#if !defined(Z_NO_EXTENSIONS) && (defined(__GNUC__) || defined(__clang__))
        
#   define Z_HAS_CLEANUP 1
    
    // RAII cleanup (destructors).
    // Usage: zvec_autofree(Int) v = zvec_init(Int);
#   define Z_CLEANUP(func) __attribute__((cleanup(func)))
    
    // Warn if the return value (e.g., an Error Result) is ignored.
#   define Z_NODISCARD     __attribute__((warn_unused_result))
    
    // Branch prediction hints for the compiler.
#   define Z_LIKELY(x)     __builtin_expect(!!(x), 1)
#   define Z_UNLIKELY(x)   __builtin_expect(!!(x), 0)

#else
        
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
 * zvec.h — Type-safe, zero-overhead dynamic arrays
 * Part of Zen Development Kit (ZDK)
 *
 * This is a macro-generated, single-header dynamic array library that produces
 * fully type-safe vector implementations at compile time. It supports both C and C++
 * with zero-cost abstraction.
 *
 * Features:
 * • C11 _Generic interface (zvec_push(v, x))
 * • Full RAII C++ wrapper in namespace z_vec::vector<T>
 * • Fast path (asserts + int error codes) and safe path (zres<T>)
 * • Optional short names via ZVEC_SHORT_NAMES
 * • Automatic type registration via z_registry.h or manual DEFINE_ macros
 *
 * License: MIT
 * Author: Zuhaitz
 * Repository: https://github.com/z-libs/zvec.h
 * Version: 1.0.3
 */

#ifndef ZVEC_H
#define ZVEC_H
// [Bundled] "zcommon.h" is included inline in this same file
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#if defined(__has_include) && __has_include("zerror.h")
#   include "zerror.h"
#   define Z_HAS_ZERROR 1
#elif defined(ZERROR_H)
#   define Z_HAS_ZERROR 1
#else
#   define Z_HAS_ZERROR 0
#endif

// C++ interop preamble.
#ifdef __cplusplus
#include <initializer_list>
#include <stdexcept>
#include <utility>
#include <iterator>
#include <algorithm>
#include <new>

namespace z_vec
{
    // Traits struct - connects C++ wrapper to generated C functions.
    template <typename T>
    struct traits
    {
        static_assert(0 == sizeof(T), "No zvec implementation registered for this type. "
                                     "Use REGISTER_ZVEC_TYPES() or Z_AUTOGEN_VECS() before including zvec.h.");
    };

    // C++ RAII vector wrapper.
    template <typename T>
    class vector
    {
        using Traits = traits<T>;
        using CType = typename Traits::c_type;

        CType inner;

    public:
        using value_type      = T;
        using size_type       = size_t;
        using difference_type = std::ptrdiff_t;
        using pointer         = T *;
        using const_pointer   = const T *;
        using iterator        = T *;
        using const_iterator  = const T *;

        // Constructors and destructor (RAII).

        vector() 
        { 
            Traits::init(inner); 
        }

        explicit vector(size_t capacity) 
        { 
            Traits::init_cap(inner, capacity); 
        }

        vector(std::initializer_list<T> init)
        {
            Traits::init_cap(inner, init.size());
            Traits::extend(inner, init.begin(), init.size());
        }

        vector(const vector &other)
        {
            Traits::init_cap(inner, other.size());
            Traits::extend(inner, other.data(), other.size());
        }

        vector(vector &&other) noexcept
        {
            inner = other.inner;
            Traits::init(other.inner);
        }

        ~vector() 
        { 
            Traits::free(inner); 
        }

        vector &operator=(const vector &other)
        {
            if (&other != this)
            {
                Traits::free(inner);
                Traits::init_cap(inner, other.size());
                Traits::extend(inner, other.data(), other.size());
            }
            return *this;
        }

        vector &operator=(vector &&other) noexcept
        {
            if (&other != this)
            {
                Traits::free(inner);
                inner = other.inner;
                Traits::init(other.inner);
            }
            return *this;
        }

        // Accessors.

        T *data()             
        { 
            return inner.data; 
        }

        const T *data() const 
        { 
            return inner.data; 
        }

        size_t size() const
        { 
            return inner.length; 
        }

        size_t capacity() const 
        { 
            return inner.capacity; 
        }

        bool empty() const      
        { 
            return 0 == inner.length; 
        }

        T &operator[](size_t idx)
        { 
            return inner.data[idx]; 
        }

        const T &operator[](size_t idx) const 
        { 
            return inner.data[idx]; 
        }

        T &at(size_t idx)
        {
            if (idx >= size()) 
            {
                throw std::out_of_range("vector::at");
            }
            return inner.data[idx];
        }

        const T &at(size_t idx) const
        {
            if (idx >= size()) throw std::out_of_range("vector::at");
            return inner.data[idx];
        }

        T &front()
        {
            return inner.data[0]; 
        }

        const T &front() const 
        { 
            return inner.data[0]; 
        }

        T &back()
        {
            return inner.data[inner.length - 1]; 
        }

        const T &back() const 
        { 
            return inner.data[inner.length - 1]; 
        }

        // Modifiers.

        void push_back(const T &val)
        { 
            if (Z_OK != Traits::push(inner, val))
            {
                throw std::bad_alloc();
            }
        }

        void pop_back()
        {
            Traits::pop(inner);
        }

        void clear()
        {
            Traits::clear(inner);
        }
        
        void reserve(size_t cap)
        {
            if (Z_OK != Traits::reserve(inner, cap))
            {
                throw std::bad_alloc();
            }
        }

        void shrink_to_fit()
        {
            Traits::shrink(inner);
        }
        
        void reverse()
        {
            Traits::reverse(inner);
        }

        void remove(size_t idx)
        {
            Traits::remove(inner, idx); 
        }

        void swap_remove(size_t idx)
        {
            Traits::swap_remove(inner, idx);
        }

        // Iterators.

        iterator begin()
        {
            return inner.data;
        }

        iterator end()
        {
            return inner.data + inner.length; 
        }
        
        const_iterator begin() const 
        { 
            return inner.data; 
        }

        const_iterator end() const
        {
            return inner.data + inner.length;
        }
    };
}

extern "C" {
#endif

// Allocator overrides (user may redefine).
#ifndef ZVEC_MALLOC
    #define ZVEC_MALLOC(sz)      Z_MALLOC(sz)
#endif

#ifndef ZVEC_CALLOC
    #define ZVEC_CALLOC(n, sz)   Z_CALLOC(n, sz)
#endif

#ifndef ZVEC_REALLOC
    #define ZVEC_REALLOC(p, sz)  Z_REALLOC(p, sz)
#endif

#ifndef ZVEC_FREE
    #define ZVEC_FREE(p)         Z_FREE(p)
#endif

/*
 * Safe API generation (requires zerror.h).
 *
 * When Z_HAS_ZERROR is true, each generated vector type gets safe variants.
 * * MODIFIED: Disabled for C++ to prevent errors with unions containing classes.
 */
#if Z_HAS_ZERROR && !defined(__cplusplus)

    static inline zerr zvec_err_impl(int code, const char *msg,
                                     const char *file, int line, const char *func)
    {
        return zerr_create_impl(code, file, line, func, "%s", msg);
    }

    #define ZVEC_GEN_SAFE_IMPL(T, Name)                                                         \
        DEFINE_RESULT(T, Res_##Name)                                                            \
                                                                                                \
        static inline zres zvec_push_safe_##Name(zvec_##Name *v, T value,                       \
                                                const char *f, int l, const char *fn)           \
        {                                                                                       \
            if (v->length >= v->capacity)                                                       \
            {                                                                                   \
                size_t new_cap = Z_GROWTH_FACTOR(v->capacity);                                  \
                T *new_data = (T *)ZVEC_REALLOC(v->data, new_cap * sizeof(T));                  \
                if (!new_data)                                                                  \
                {                                                                               \
                    return zres_err(zvec_err_impl(Z_ENOMEM, "Vector Push OOM", f, l, fn));      \
                }                                                                               \
                v->data = new_data;                                                             \
                v->capacity = new_cap;                                                          \
            }                                                                                   \
            v->data[v->length++] = value;                                                       \
            return zres_ok();                                                                   \
        }                                                                                       \
                                                                                                \
        static inline zres zvec_reserve_safe_##Name(zvec_##Name *v, size_t cap,                 \
                                                   const char *f, int l, const char *fn)        \
        {                                                                                       \
            if (cap <= v->capacity)                                                             \
            {                                                                                   \
                return zres_ok();                                                               \
            }                                                                                   \
            T *new_data = (T *)ZVEC_REALLOC(v->data, cap * sizeof(T));                          \
            if (!new_data)                                                                      \
            {                                                                                   \
                return zres_err(zvec_err_impl(Z_ENOMEM, "Vector Reserve OOM", f, l, fn));       \
            }                                                                                   \
            v->data = new_data;                                                                 \
            v->capacity = cap;                                                                  \
            return zres_ok();                                                                   \
        }                                                                                       \
                                                                                                \
        static inline Res_##Name zvec_pop_safe_##Name(zvec_##Name *v,                           \
                                                     const char *f, int l, const char *fn)      \
        {                                                                                       \
            if (0 == v->length)                                                                 \
            {                                                                                   \
                return Res_##Name##_err(zvec_err_impl(Z_EEMPTY, "Pop empty vec", f, l, fn));    \
            }                                                                                   \
            return Res_##Name##_ok(v->data[--v->length]);                                       \
        }                                                                                       \
                                                                                                \
        static inline Res_##Name zvec_at_safe_##Name(zvec_##Name *v, size_t i,                  \
                                                    const char *f, int l, const char *fn)       \
        {                                                                                       \
            if (i >= v->length)                                                                 \
            {                                                                                   \
                return Res_##Name##_err(zvec_err_impl(Z_EOOB, "Index out of bounds", f, l, fn));\
            }                                                                                   \
            return Res_##Name##_ok(v->data[i]);                                                 \
        }                                                                                       \
                                                                                                \
        static inline Res_##Name zvec_last_safe_##Name(zvec_##Name *v,                          \
                                                      const char *f, int l, const char *fn)     \
        {                                                                                       \
            if (0 == v->length)                                                                 \
            {                                                                                   \
                return Res_##Name##_err(zvec_err_impl(Z_EEMPTY, "Vector is empty", f, l, fn));  \
            }                                                                                   \
            return Res_##Name##_ok(v->data[v->length - 1]);                                     \
        }

#else
#   define ZVEC_GEN_SAFE_IMPL(T, Name)
#endif

// C++ compatibility layers.

#ifdef __cplusplus
#   define ZVEC_IMPL_ALLOC(T, Name)                                             \
        static inline int zvec_reserve_##Name(zvec_##Name *v, size_t new_cap)   \
        {                                                                       \
            if (new_cap <= v->capacity)                                         \
            {                                                                   \
                return Z_OK;                                                    \
            }                                                                   \
            try {                                                               \
                T* new_data = new T[new_cap];                                   \
                for (size_t i = 0; i < v->length; ++i)                          \
                {                                                               \
                    new_data[i] = std::move(v->data[i]);                        \
                }                                                               \
                if (v->data)                                                    \
                {                                                               \
                    delete[] v->data;                                           \
                }                                                               \
                v->data = new_data;                                             \
                v->capacity = new_cap;                                          \
            } catch (...) { return Z_ENOMEM; }                                  \
            return Z_OK;                                                        \
        }                                                                       \
                                                                                \
        static inline void zvec_free_##Name(zvec_##Name *v)                     \
        {                                                                       \
            if (v->data)                                                        \
            {                                                                   \
                delete[] v->data;                                               \
            }                                                                   \
            v->data = NULL;                                                     \
            v->length = 0;                                                      \
            v->capacity = 0;                                                    \
        }                                                                       \
                                                                                \
        static inline void zvec_remove_##Name(zvec_##Name *v, size_t index)     \
        {                                                                       \
            if (index >= v->length)                                             \
            {                                                                   \
                return;                                                         \
            }                                                                   \
            for (size_t i = index; i < v->length - 1; ++i)                      \
            {                                                                   \
                v->data[i] = std::move(v->data[i + 1]);                         \
            }                                                                   \
            v->data[v->length - 1] = T();                                       \
            v->length--;                                                        \
        }                                                                       \
                                                                                \
        static inline void zvec_shrink_to_fit_##Name(zvec_##Name *v)            \
        {                                                                       \
            if (v->length == v->capacity)                                       \
            {                                                                   \
                return;                                                         \
            }                                                                   \
            if (0 == v->length)                                                 \
            {                                                                   \
                zvec_free_##Name(v);                                            \
                return;                                                         \
            }                                                                   \
            try                                                                 \
            {                                                                   \
                T* new_data = new T[v->length];                                 \
                for (size_t i = 0; i < v->length; ++i)                          \
                {                                                               \
                    new_data[i] = std::move(v->data[i]);                        \
                }                                                               \
                if (v->data) delete[] v->data;                                  \
                v->data = new_data;                                             \
                v->capacity = v->length;                                        \
            }                                                                   \
            catch (...) {}                                                      \
        }

    // C++ dispatch: generates inline overloads so C++ can find functions without _Generic.
#   define ZVEC_CPP_DISPATCH_IMPL(T, Name)                                                  \
        static inline int zvec_reserve_dispatch(zvec_##Name *v, size_t n)                   \
        {                                                                                   \
            return zvec_reserve_##Name(v, n);                                               \
        }                                                                                   \
                                                                                            \
        static inline int zvec_push_dispatch(zvec_##Name *v, T val)                         \
        {                                                                                   \
            return zvec_push_##Name(v, val);                                                \
        }                                                                                   \
                                                                                            \
        static inline T* zvec_push_slot_dispatch(zvec_##Name *v)                            \
        {                                                                                   \
            return zvec_push_slot_##Name(v);                                                \
        }                                                                                   \
                                                                                            \
        static inline int zvec_extend_dispatch(zvec_##Name *v, const T *arr, size_t n)      \
        {                                                                                   \
            return zvec_extend_##Name(v, arr, n);                                           \
        }                                                                                   \
                                                                                            \
        static inline int zvec_is_empty_dispatch(zvec_##Name *v)                            \
        {                                                                                   \
            return zvec_is_empty_##Name(v);                                                 \
        }                                                                                   \
                                                                                            \
        static inline T* zvec_at_dispatch(zvec_##Name *v, size_t i)                         \
        {                                                                                   \
            return zvec_at_##Name(v, i);                                                    \
        }                                                                                   \
                                                                                            \
        static inline T* zvec_data_dispatch(zvec_##Name *v)                                 \
        {                                                                                   \
            return zvec_data_##Name(v);                                                     \
        }                                                                                   \
                                                                                            \
        static inline T* zvec_last_dispatch(zvec_##Name *v)                                 \
        {                                                                                   \
            return zvec_last_##Name(v);                                                     \
        }                                                                                   \
                                                                                            \
        static inline void zvec_free_dispatch(zvec_##Name *v)                               \
        {                                                                                   \
            zvec_free_##Name(v);                                                            \
        }                                                                                   \
                                                                                            \
        static inline void zvec_pop_dispatch(zvec_##Name *v)                                \
        {                                                                                   \
            zvec_pop_##Name(v);                                                             \
        }                                                                                   \
                                                                                            \
        static inline T zvec_pop_get_dispatch(zvec_##Name *v)                               \
        {                                                                                   \
            return zvec_pop_get_##Name(v);                                                  \
        }                                                                                   \
                                                                                            \
        static inline void zvec_shrink_to_fit_dispatch(zvec_##Name *v)                      \
        {                                                                                   \
            zvec_shrink_to_fit_##Name(v);                                                   \
        }                                                                                   \
                                                                                            \
        static inline void zvec_remove_dispatch(zvec_##Name *v, size_t i)                   \
        {                                                                                   \
            zvec_remove_##Name(v, i);                                                       \
        }                                                                                   \
                                                                                            \
        static inline void zvec_swap_remove_dispatch(zvec_##Name *v, size_t i)              \
        {                                                                                   \
            zvec_swap_remove_##Name(v, i);                                                  \
        }                                                                                   \
                                                                                            \
        static inline void zvec_clear_dispatch(zvec_##Name *v)                              \
        {                                                                                   \
            zvec_clear_##Name(v);                                                           \
        }                                                                                   \
                                                                                            \
        static inline void zvec_reverse_dispatch(zvec_##Name *v)                            \
        {                                                                                   \
            zvec_reverse_##Name(v);                                                         \
        }                                                                                   \
                                                                                            \
        static inline void zvec_sort_dispatch(zvec_##Name *v,                               \
                                              int (*cmp)(const T*, const T*))               \
        {                                                                                   \
            zvec_sort_##Name(v, cmp);                                                       \
        }                                                                                   \
                                                                                            \
        static inline T* zvec_bsearch_dispatch(zvec_##Name *v, const T* k,                  \
                                               int (*cmp)(const T*, const T*))              \
        {                                                                                   \
            return zvec_bsearch_##Name(v, k, cmp);                                          \
        }                                                                                   \
                                                                                            \
        static inline T* zvec_lower_bound_dispatch(zvec_##Name *v, const T* k,              \
                                                   int (*cmp)(const T*, const T*))          \
        {                                                                                   \
            return zvec_lower_bound_##Name(v, k, cmp);                                      \
        }
#else
    // C implementation: uses realloc / memmove / free.
    #define ZVEC_IMPL_ALLOC(T, Name)                                                            \
        static inline int zvec_reserve_##Name(zvec_##Name *v, size_t new_cap)                   \
        {                                                                                       \
            if (new_cap <= v->capacity)                                                         \
            {                                                                                   \
                return Z_OK;                                                                    \
            }                                                                                   \
            T* new_data = (T*)ZVEC_REALLOC(v->data, new_cap * sizeof(T));                       \
            if (!new_data)                                                                      \
            {                                                                                   \
                return Z_ENOMEM;                                                                \
            }                                                                                   \
            v->data = new_data;                                                                 \
            v->capacity = new_cap;                                                              \
            return Z_OK;                                                                        \
        }                                                                                       \
                                                                                                \
        static inline void zvec_free_##Name(zvec_##Name *v)                                     \
        {                                                                                       \
            ZVEC_FREE(v->data);                                                                 \
            memset(v, 0, sizeof(zvec_##Name));                                                  \
        }                                                                                       \
                                                                                                \
        static inline void zvec_remove_##Name(zvec_##Name *v, size_t index)                     \
        {                                                                                       \
            if (index >= v->length)                                                             \
            {                                                                                   \
                return;                                                                         \
            }                                                                                   \
            memmove(&v->data[index], &v->data[index + 1], (v->length - index - 1) * sizeof(T)); \
            v->length--;                                                                        \
        }                                                                                       \
                                                                                                \
        static inline void zvec_shrink_to_fit_##Name(zvec_##Name *v)                            \
        {                                                                                       \
            if (v->length == v->capacity)                                                       \
            {                                                                                   \
                return;                                                                         \
            }                                                                                   \
            if (0 == v->length)                                                                 \
            {                                                                                   \
                zvec_free_##Name(v);                                                            \
                return;                                                                         \
            }                                                                                   \
            T* new_data = (T*)ZVEC_REALLOC(v->data, v->length * sizeof(T));                     \
            if (new_data)                                                                       \
            {                                                                                   \
                v->data = new_data;                                                             \
                v->capacity = v->length;                                                        \
            }                                                                                   \
        }

#   define ZVEC_CPP_DISPATCH_IMPL(T, Name) // Empty in C.
#endif

/*
 * ZVEC_GENERATE_IMPL(T, Name)
 *
 * Primary generation macro.
 *
 * Use manually via:
 * #define REGISTER_ZVEC_TYPES(X)   \
 * X(int, Int)                      \
 * X(float, Float)                  \
 * X(MyType, MyType)
 * #include "zvec.h"
 *
 * This creates:
 * • struct zvec_Int / zvec_Float, etc.
 * • All functions: zvec_push_Int, zvec_reserve_Float, etc.
 * • Full _Generic dispatch.
 * • C++ traits specialization for z_vec::vector<T>
 *
 * Fast path: returns int (Z_OK / Z_ENOMEM), asserts on logic errors.
 * Safe path (when zerror.h present): returns zres / zresult<T>.
 */
#define ZVEC_GENERATE_IMPL(T, Name)                                                         \
                                                                                            \
    typedef T zvec_T_##Name;                                                                \
                                                                                            \
    typedef struct                                                                          \
    {                                                                                       \
        T *data;                                                                            \
        size_t length;                                                                      \
        size_t capacity;                                                                    \
    } zvec_##Name;                                                                          \
                                                                                            \
    /* Forward declaration for C++ allocators to see. */                                    \
    static inline int zvec_reserve_##Name(zvec_##Name *v, size_t new_cap);                  \
                                                                                            \
    ZVEC_IMPL_ALLOC(T, Name)                                                                \
                                                                                            \
    static inline zvec_##Name zvec_init_capacity_##Name(size_t cap)                         \
    {                                                                                       \
        zvec_##Name v;                                                                      \
        memset(&v, 0, sizeof(zvec_##Name));                                                 \
        if (cap > 0)                                                                        \
        {                                                                                   \
            zvec_reserve_##Name(&v, cap);                                                   \
        }                                                                                   \
        return v;                                                                           \
    }                                                                                       \
                                                                                            \
    static inline zvec_##Name zvec_from_array_##Name(const T *arr, size_t count)            \
    {                                                                                       \
        zvec_##Name v = zvec_init_capacity_##Name(count);                                   \
        if (v.data)                                                                         \
        {                                                                                   \
            size_t i;                                                                       \
            for(i = 0; i<count; ++i)                                                        \
            {                                                                               \
                v.data[i] = arr[i];                                                         \
            }                                                                               \
            v.length = count;                                                               \
        }                                                                                   \
        return v;                                                                           \
    }                                                                                       \
                                                                                            \
    static inline int zvec_is_empty_##Name(zvec_##Name *v)                                  \
    {                                                                                       \
        return 0 == v->length;                                                              \
    }                                                                                       \
                                                                                            \
    static inline T *zvec_push_slot_##Name(zvec_##Name *v)                                  \
    {                                                                                       \
        if (v->length >= v->capacity)                                                       \
        {                                                                                   \
            size_t new_cap = Z_GROWTH_FACTOR(v->capacity);                                  \
            if (Z_OK != zvec_reserve_##Name(v, new_cap))                                    \
            {                                                                               \
                return NULL;                                                                \
            }                                                                               \
        }                                                                                   \
        return &v->data[v->length++];                                                       \
    }                                                                                       \
                                                                                            \
    static inline int zvec_push_##Name(zvec_##Name *v, T value)                             \
    {                                                                                       \
        T *slot = zvec_push_slot_##Name(v);                                                 \
        if (!slot)                                                                          \
        {                                                                                   \
            return Z_ENOMEM;                                                                \
        }                                                                                   \
        *slot = value;                                                                      \
        return Z_OK;                                                                        \
    }                                                                                       \
                                                                                            \
    static inline int zvec_extend_##Name(zvec_##Name *v, const T *items, size_t count)      \
    {                                                                                       \
        if (v->length + count > v->capacity)                                                \
        {                                                                                   \
            size_t new_cap = v->capacity ? v->capacity : Z_GROWTH_FACTOR(0);                \
            while (new_cap < v->length + count)                                             \
            {                                                                               \
                new_cap = Z_GROWTH_FACTOR(new_cap);                                         \
            }                                                                               \
            if (Z_OK != zvec_reserve_##Name(v, new_cap))                                    \
            {                                                                               \
                return Z_ENOMEM;                                                            \
            }                                                                               \
        }                                                                                   \
        size_t i;                                                                           \
        for( i = 0; i<count; ++i)                                                           \
        {                                                                                   \
            v->data[v->length + i] = items[i];                                              \
        }                                                                                   \
        v->length += count;                                                                 \
        return Z_OK;                                                                        \
    }                                                                                       \
                                                                                            \
    static inline void zvec_pop_##Name(zvec_##Name *v)                                      \
       {                                                                                    \
        assert(v->length > 0 && "Popping empty vector");                                    \
        v->length--;                                                                        \
    }                                                                                       \
                                                                                            \
    static inline T zvec_pop_get_##Name(zvec_##Name *v)                                     \
    {                                                                                       \
        assert(v->length > 0 && "Vector is empty");                                         \
        return v->data[--v->length];                                                        \
    }                                                                                       \
                                                                                            \
    static inline T *zvec_at_##Name(zvec_##Name *v, size_t index)                           \
    {                                                                                       \
        return (index < v->length) ? &v->data[index] : NULL;                                \
    }                                                                                       \
                                                                                            \
    static inline T *zvec_data_##Name(zvec_##Name *v)                                       \
    {                                                                                       \
        return v->data;                                                                     \
    }                                                                                       \
                                                                                            \
    static inline T *zvec_last_##Name(zvec_##Name *v)                                       \
    {                                                                                       \
        return (v->length > 0) ? &v->data[v->length - 1] : NULL;                            \
    }                                                                                       \
                                                                                            \
    static inline void zvec_swap_remove_##Name(zvec_##Name *v, size_t index)                \
    {                                                                                       \
        if (index >= v->length) return;                                                     \
        v->data[index] = v->data[--v->length];                                              \
    }                                                                                       \
                                                                                            \
    static inline void zvec_clear_##Name(zvec_##Name *v)                                    \
    {                                                                                       \
        v->length = 0;                                                                      \
    }                                                                                       \
                                                                                            \
    static inline void zvec_reverse_##Name(zvec_##Name *v)                                  \
    {                                                                                       \
        if (v->length < 2)                                                                  \
        {                                                                                   \
            return;                                                                         \
        }                                                                                   \
        size_t i = 0;                                                                       \
        size_t j = v->length - 1;                                                           \
        while (i < j)                                                                       \
        {                                                                                   \
            T temp = v->data[i];                                                            \
            v->data[i] = v->data[j];                                                        \
            v->data[j] = temp;                                                              \
            i++;                                                                            \
            j--;                                                                            \
        }                                                                                   \
    }                                                                                       \
                                                                                            \
    static inline void zvec_sort_##Name(zvec_##Name *v,                                     \
                                        int (*compar)(const T *, const T *))                \
    {                                                                                       \
        if (v->length > 1)                                                                  \
        {                                                                                   \
            qsort(v->data, v->length, sizeof(T),                                            \
                (int(*)(const void *, const void *))compar);                                \
        }                                                                                   \
    }                                                                                       \
                                                                                            \
    static inline T *zvec_bsearch_##Name(zvec_##Name *v, const T *key,                      \
                                         int (*compar)(const T *, const T *))               \
    {                                                                                       \
        if (0 == v->length)                                                                 \
        {                                                                                   \
            return NULL;                                                                    \
        }                                                                                   \
        return (T *)bsearch(key, v->data, v->length, sizeof(T),                             \
                            (int(*)(const void *, const void *))compar);                    \
    }                                                                                       \
                                                                                            \
    static inline T *zvec_lower_bound_##Name(zvec_##Name *v, const T *key,                  \
                                             int (*compar)(const T *, const T *))           \
    {                                                                                       \
        size_t l = 0;                                                                       \
        size_t r = v->length;                                                               \
        while (l < r)                                                                       \
        {                                                                                   \
            size_t mid = l + (r - l) / 2;                                                   \
            if (compar((const T *)&v->data[mid], key) < 0)                                  \
            {                                                                               \
                l = mid + 1;                                                                \
            }                                                                               \
            else                                                                            \
            {                                                                               \
                r = mid;                                                                    \
            }                                                                               \
        }                                                                                   \
        return (l < v->length) ? &v->data[l] : NULL;                                        \
    }                                                                                       \
                                                                                            \
    /* Inject safe API. */                                                                  \
    ZVEC_GEN_SAFE_IMPL(T, Name)

// Dispatch table entries for _Generic.
#define PUSH_ENTRY(T, Name)         zvec_##Name *: zvec_push_##Name,
#define PUSH_SLOT_ENTRY(T, Name)    zvec_##Name *: zvec_push_slot_##Name,
#define EXTEND_ENTRY(T, Name)       zvec_##Name *: zvec_extend_##Name,
#define RESERVE_ENTRY(T, Name)      zvec_##Name *: zvec_reserve_##Name,
#define IS_EMPTY_ENTRY(T, Name)     zvec_##Name *: zvec_is_empty_##Name,
#define AT_ENTRY(T, Name)           zvec_##Name *: zvec_at_##Name,
#define DATA_ENTRY(T, Name)         zvec_##Name *: zvec_data_##Name,
#define LAST_ENTRY(T, Name)         zvec_##Name *: zvec_last_##Name,
#define FREE_ENTRY(T, Name)         zvec_##Name *: zvec_free_##Name,
#define POP_ENTRY(T, Name)          zvec_##Name *: zvec_pop_##Name,
#define POP_GET_ENTRY(T, Name)      zvec_##Name *: zvec_pop_get_##Name,
#define SHRINK_ENTRY(T, Name)       zvec_##Name *: zvec_shrink_to_fit_##Name,
#define REMOVE_ENTRY(T, Name)       zvec_##Name *: zvec_remove_##Name,
#define SWAP_REM_ENTRY(T, Name)     zvec_##Name *: zvec_swap_remove_##Name,
#define CLEAR_ENTRY(T, Name)        zvec_##Name *: zvec_clear_##Name,
#define REVERSE_ENTRY(T, Name)      zvec_##Name *: zvec_reverse_##Name,
#define SORT_ENTRY(T, Name)         zvec_##Name *: zvec_sort_##Name,
#define BSEARCH_ENTRY(T, Name)      zvec_##Name *: zvec_bsearch_##Name,
#define LOWER_BOUND_ENTRY(T, Name)  zvec_##Name *: zvec_lower_bound_##Name,

#if Z_HAS_ZERROR
#   define RESERVE_SAFE_ENTRY(T, Name) zvec_##Name *: zvec_reserve_safe_##Name,
#   define PUSH_SAFE_ENTRY(T, Name)    zvec_##Name *: zvec_push_safe_##Name,
#   define POP_SAFE_ENTRY(T, Name)     zvec_##Name *: zvec_pop_safe_##Name,
#   define AT_SAFE_ENTRY(T, Name)      zvec_##Name *: zvec_at_safe_##Name,
#   define LAST_SAFE_ENTRY(T, Name)    zvec_##Name *: zvec_last_safe_##Name,
#endif

#ifndef REGISTER_ZVEC_TYPES
#   if defined(__has_include) && __has_include("z_registry.h")
#       include "z_registry.h"
#   endif
#endif

#ifndef REGISTER_ZVEC_TYPES
#   define REGISTER_ZVEC_TYPES(X)
#endif

#ifndef Z_AUTOGEN_VECS
#   define Z_AUTOGEN_VECS(X)
#endif

#define Z_ALL_VECS(X)       \
    Z_AUTOGEN_VECS(X)       \
    REGISTER_ZVEC_TYPES(X)

Z_ALL_VECS(ZVEC_GENERATE_IMPL)

// Public convenience macros.

#define zvec_from(Name, ...)                                                                \
    zvec_from_array_##Name((zvec_T_##Name[]){__VA_ARGS__},                                  \
                           sizeof((zvec_T_##Name[]){__VA_ARGS__}) / sizeof(zvec_T_##Name))

#define zvec_init(Name)           zvec_init_capacity_##Name(0)
#define zvec_init_with_cap(Name, cap) zvec_init_capacity_##Name(cap)

#if Z_HAS_CLEANUP
#   define zvec_autofree(Name)  Z_CLEANUP(zvec_free_##Name) zvec_##Name
#endif

// Generic dispatch using _Generic.

#ifdef __cplusplus
    // C++ Overload Dispatch
#   define zvec_push(v, val)          zvec_push_dispatch(v, val)
#   define zvec_push_slot(v)          zvec_push_slot_dispatch(v)
#   define zvec_extend(v, arr, count) zvec_extend_dispatch(v, arr, count)
#   define zvec_reserve(v, cap)       zvec_reserve_dispatch(v, cap)
#   define zvec_is_empty(v)           zvec_is_empty_dispatch(v)
#   define zvec_at(v, idx)            zvec_at_dispatch(v, idx)
#   define zvec_data(v)               zvec_data_dispatch(v)
#   define zvec_last(v)               zvec_last_dispatch(v)
#   define zvec_free(v)               zvec_free_dispatch(v)
#   define zvec_pop(v)                zvec_pop_dispatch(v)
#   define zvec_pop_get(v)            zvec_pop_get_dispatch(v)
#   define zvec_shrink_to_fit(v)      zvec_shrink_to_fit_dispatch(v)
#   define zvec_remove(v, i)          zvec_remove_dispatch(v, i)
#   define zvec_swap_remove(v, i)     zvec_swap_remove_dispatch(v, i)
#   define zvec_clear(v)              zvec_clear_dispatch(v)
#   define zvec_reverse(v)            zvec_reverse_dispatch(v)
#   define zvec_sort(v, cmp)          zvec_sort_dispatch(v, cmp)
#   define zvec_bsearch(v, k, c)      zvec_bsearch_dispatch(v, k, c)
#   define zvec_lower_bound(v, k, c)  zvec_lower_bound_dispatch(v, k, c)
#else
    // C _Generic Dispatch
#   define zvec_push(v, val)          _Generic((v), Z_ALL_VECS(PUSH_ENTRY)          default: 0)(v, val)
#   define zvec_push_slot(v)          _Generic((v), Z_ALL_VECS(PUSH_SLOT_ENTRY)     default: (void *)0)(v)
#   define zvec_extend(v, arr, count) _Generic((v), Z_ALL_VECS(EXTEND_ENTRY)        default: 0)(v, arr, count)
#   define zvec_reserve(v, cap)       _Generic((v), Z_ALL_VECS(RESERVE_ENTRY)       default: 0)(v, cap)
#   define zvec_is_empty(v)           _Generic((v), Z_ALL_VECS(IS_EMPTY_ENTRY)      default: 0)(v)
#   define zvec_at(v, idx)            _Generic((v), Z_ALL_VECS(AT_ENTRY)            default: (void *)0)(v, idx)
#   define zvec_data(v)               _Generic((v), Z_ALL_VECS(DATA_ENTRY)          default: (void *)0)(v)
#   define zvec_last(v)               _Generic((v), Z_ALL_VECS(LAST_ENTRY)          default: (void *)0)(v)
#   define zvec_free(v)               _Generic((v), Z_ALL_VECS(FREE_ENTRY)          default: (void)0)(v)
#   define zvec_pop(v)                _Generic((v), Z_ALL_VECS(POP_ENTRY)           default: (void)0)(v)
#   define zvec_pop_get(v)            _Generic((v), Z_ALL_VECS(POP_GET_ENTRY)       default: (void)0)(v)
#   define zvec_shrink_to_fit(v)      _Generic((v), Z_ALL_VECS(SHRINK_ENTRY)        default: (void)0)(v)
#   define zvec_remove(v, i)          _Generic((v), Z_ALL_VECS(REMOVE_ENTRY)        default: (void)0)(v, i)
#   define zvec_swap_remove(v, i)     _Generic((v), Z_ALL_VECS(SWAP_REM_ENTRY)      default: (void)0)(v, i)
#   define zvec_clear(v)              _Generic((v), Z_ALL_VECS(CLEAR_ENTRY)         default: (void)0)(v)
#   define zvec_reverse(v)            _Generic((v), Z_ALL_VECS(REVERSE_ENTRY)       default: (void)0)(v)
#   define zvec_sort(v, cmp)          _Generic((v), Z_ALL_VECS(SORT_ENTRY)          default: (void)0)(v, cmp)
#   define zvec_bsearch(v, k, c)      _Generic((v), Z_ALL_VECS(BSEARCH_ENTRY)       default: (void *)0)(v, k, c)
#   define zvec_lower_bound(v, k, c)  _Generic((v), Z_ALL_VECS(LOWER_BOUND_ENTRY)   default: (void *)0)(v, k, c)
#endif

/* * Explicit declaration macro (portable C99)
 * Usage: zvec_foreach_decl(Int, &vec, it) { ... }
 */
#define zvec_foreach_decl(Name, v, iter)                                        \
    for (zvec_T_##Name *iter = (v)->data;                                       \
         iter < (v)->data + (v)->length;                                        \
         ++iter)

// Smart iteration helpers (auto-inference).
#if defined(__GNUC__) || defined(__clang__)

    #define zvec_foreach(v, iter)                                               \
        for (__typeof__((v)->data) iter = (v)->data;                            \
             iter < (v)->data + (v)->length;                                    \
             ++iter)

#else

    /* Standard C fallback: User must declare 'iter' before loop.
     * Usage: int *it; zvec_foreach(&vec, it) { ... }
     */
    #define zvec_foreach(v, iter)                                               \
        for ((iter) = (v)->data;                                                \
             (iter) < (v)->data + (v)->length;                                  \
             ++(iter))

#endif

// Safe API dispatch (zerror.h required).
#if Z_HAS_ZERROR && !defined(__cplusplus)
    static inline zres zres_err_dummy(void *v, ...) 
    {
        (void)v; 
        return zres_err(zerr_create(-1, "Unknown vector type")); 
    }

#   define zvec_reserve_safe(v, cap) \
        _Generic((v), Z_ALL_VECS(RESERVE_SAFE_ENTRY) default: zres_err_dummy)(v, cap, __FILE__, __LINE__, __func__)

#   define zvec_push_safe(v, val) \
        _Generic((v), Z_ALL_VECS(PUSH_SAFE_ENTRY) default: zres_err_dummy)(v, val, __FILE__, __LINE__, __func__)

#   define zvec_pop_safe(v) \
        _Generic((v), Z_ALL_VECS(POP_SAFE_ENTRY) default: zres_err_dummy)(v, __FILE__, __LINE__, __func__)

#   define zvec_at_safe(v, i) \
        _Generic((v), Z_ALL_VECS(AT_SAFE_ENTRY) default: zres_err_dummy)(v, i, __FILE__, __LINE__, __func__)

#   define zvec_last_safe(v) \
        _Generic((v), Z_ALL_VECS(LAST_SAFE_ENTRY) default: zres_err_dummy)(v, __FILE__, __LINE__, __func__)
#endif

// Optional short names (never enabled by default).
#ifdef ZVEC_SHORT_NAMES
#   define vec(Name)              zvec_##Name
#   define vec_init               zvec_init
#   define vec_init_with_cap      zvec_init_with_cap
#   define vec_from               zvec_from
#   define vec_autofree           zvec_autofree
#   define vec_push               zvec_push
#   define vec_push_slot          zvec_push_slot
#   define vec_extend             zvec_extend
#   define vec_reserve            zvec_reserve
#   define vec_is_empty           zvec_is_empty
#   define vec_at                 zvec_at
#   define vec_data               zvec_data
#   define vec_last               zvec_last
#   define vec_free               zvec_free
#   define vec_pop                zvec_pop
#   define vec_pop_get            zvec_pop_get
#   define vec_shrink_to_fit      zvec_shrink_to_fit
#   define vec_remove             zvec_remove
#   define vec_swap_remove        zvec_swap_remove
#   define vec_clear              zvec_clear
#   define vec_reverse            zvec_reverse
#   define vec_sort               zvec_sort
#   define vec_bsearch            zvec_bsearch
#   define vec_lower_bound        zvec_lower_bound
#   define vec_foreach            zvec_foreach
#   if Z_HAS_ZERROR && !defined(__cplusplus)
#       define vec_reserve_safe   zvec_reserve_safe
#       define vec_push_safe      zvec_push_safe
#       define vec_pop_safe       zvec_pop_safe
#       define vec_at_safe        zvec_at_safe
#       define vec_last_safe      zvec_last_safe
#   endif
#endif

#ifdef __cplusplus
} // extern "C"

Z_ALL_VECS(ZVEC_CPP_DISPATCH_IMPL)

namespace z_vec
{
    #define ZVEC_CPP_TRAITS(T, Name)                                        \
        template <> struct traits<T>                                        \
        {                                                                   \
            using c_type = ::zvec_##Name;                                   \
            static inline void init(c_type &v)                              \
            {                                                               \
                v = ::zvec_init_capacity_##Name(0);                         \
            }                                                               \
                                                                            \
            static inline void init_cap(c_type &v, size_t c)                \
            {                                                               \
                v = ::zvec_init_capacity_##Name(c);                         \
            }                                                               \
                                                                            \
            static inline void free(c_type &v)                              \
            {                                                               \
                ::zvec_free_##Name(&v);                                     \
            }                                                               \
                                                                            \
            static inline int push(c_type &v, T val)                        \
            {                                                               \
                return ::zvec_push_##Name(&v, val);                         \
            }                                                               \
                                                                            \
            static inline int extend(c_type &v, const T *arr, size_t n)     \
            {                                                               \
                return ::zvec_extend_##Name(&v, arr, n);                    \
            }                                                               \
                                                                            \
            static inline int reserve(c_type &v, size_t n)                  \
            {                                                               \
                return ::zvec_reserve_##Name(&v, n);                        \
            }                                                               \
                                                                            \
            static inline void pop(c_type &v)                               \
            {                                                               \
                ::zvec_pop_##Name(&v);                                      \
            }                                                               \
                                                                            \
            static inline T pop_get(c_type &v)                              \
            {                                                               \
                return ::zvec_pop_get_##Name(&v);                           \
            }                                                               \
                                                                            \
            static inline void remove(c_type &v, size_t i)                  \
            {                                                               \
                ::zvec_remove_##Name(&v, i);                                \
            }                                                               \
                                                                            \
            static inline void swap_remove(c_type &v, size_t i)             \
            {                                                               \
                ::zvec_swap_remove_##Name(&v, i);                           \
            }                                                               \
                                                                            \
            static inline void clear(c_type &v)                             \
            {                                                               \
                ::zvec_clear_##Name(&v);                                    \
            }                                                               \
                                                                            \
            static inline void shrink(c_type &v)                            \
            {                                                               \
                ::zvec_shrink_to_fit_##Name(&v);                            \
            }                                                               \
                                                                            \
            static inline void reverse(c_type &v)                           \
            {                                                               \
                ::zvec_reverse_##Name(&v);                                  \
            }                                                               \
        };

    Z_ALL_VECS(ZVEC_CPP_TRAITS)
}
#endif // __cplusplus

#endif // ZVEC_H
