/*
 * GENERATED FILE - DO NOT EDIT DIRECTLY
 * Source: zlist.c
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
 * zlist.h — Type-safe, zero-overhead intrusive doubly-linked lists
 * Part of Zen Development Kit (ZDK)
 *
 * This is a macro-generated, single-header intrusive doubly-linked list library.
 * It produces fully type-safe list implementations at compile time with zero
 * runtime overhead and full C11 _Generic + C++ RAII support.
 *
 * Features:
 * • O(1) push/pop front/back, insert_after, splice
 * • Full bidirectional iterators
 * • C++ z_list::list<T> with RAII and STL-compatible interface
 * • Optional short names via ZLIST_SHORT_NAMES
 * • Automatic type registration via z_registry.h
 * • Allocation failure returns Z_ENOMEM (fast path)
 * • Support for complex C++ types (constructors/destructors called)
 *
 * License: MIT
 * Author: Zuhaitz
 * Repository: https://github.com/z-libs/zlist.h
 * Version: 1.2.0
 */

#ifndef ZLIST_H
#define ZLIST_H
// [Bundled] "zcommon.h" is included inline in this same file
#include <assert.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#if defined(__has_include) && __has_include("zerror.h")
    #include "zerror.h"
    #define Z_HAS_ZERROR 1
#elif defined(ZERROR_H)
    #define Z_HAS_ZERROR 1
#else
    #define Z_HAS_ZERROR 0
#endif

// C++ interop preamble.
#ifdef __cplusplus
#include <stdexcept>
#include <initializer_list>
#include <iterator>
#include <utility>
#include <type_traits>
#include <new>

namespace z_list
{
    // Forward declarations.
    template <typename T> struct list;
    template <typename T> class list_iterator;

    // Traits struct.
    template <typename T>
    struct traits
    {
        static_assert(0 == sizeof(T), "No zlist implementation registered for this type.");
    };

    template <typename T>
    class list_iterator
    {
    public:
        using value_type = T;
        using reference = T&;
        using pointer = T*;
        using difference_type = ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;

        // Use traits to get the underlying C types.
        using Traits = traits<typename std::remove_const<T>::type>;
        using CNode = typename Traits::node_type;
        using CList = typename Traits::list_type;

        explicit list_iterator(const CList* l, CNode *p) : list_ptr(l), current(p) {}

        reference operator*() const 
        { 
            return current->value; 
        }

        pointer operator->() const 
        { 
            return &current->value; 
        }

        bool operator==(const list_iterator &other) const 
        { 
            return current == other.current; 
        }

        bool operator!=(const list_iterator &other) const 
        { 
            return current != other.current; 
        }

        list_iterator &operator++() 
        {
            if (current) 
            {
                current = current->next; 
            }
            return *this; 
        }

        list_iterator operator++(int) 
        {
            list_iterator temp = *this; 
            if (current)
            {
                current = current->next;
            }
            return temp; 
        }

        list_iterator &operator--() 
        { 
            if (nullptr == current)
            {
                current = list_ptr->tail;
            }
            else
            {
                current = current->prev; 
            }
            return *this; 
        }

        list_iterator operator--(int) 
        { 
            list_iterator temp = *this;
            if (nullptr == current)
            {
                current = list_ptr->tail;
            }
            else
            {
                current = current->prev;
            }
            return temp; 
        }

    private:
        const CList* list_ptr; 
        CNode *current;
        friend struct list<typename std::remove_const<T>::type>;
    };

    template <typename T>
    struct list
    {
        using Traits = traits<T>;
        using c_list = typename Traits::list_type;
        using c_node = typename Traits::node_type;

        using iterator = list_iterator<T>;
        using const_iterator = list_iterator<const T>;

        c_list inner;

        list() : inner(Traits::init()) {}

        list(std::initializer_list<T> init) : inner(Traits::init())
        {
            for (const auto &item : init) 
            {
                push_back(item);
            }
        }

        list(const list &other) : inner(Traits::init())
        {
            for (const auto &item : other)
            {
                push_back(item);
            }
        }

        list(list &&other) noexcept : inner(other.inner)
        {
            other.inner = Traits::init();
        }

        ~list()
        {
            Traits::clear(&inner);
        }

        list &operator=(const list &other)
        {
            if (&other != this) 
            {
                Traits::clear(&inner);
                inner = Traits::init();
                for (const auto &item : other)
                {
                    push_back(item);
                }
            }
            return *this;
        }

        list& operator=(list &&other) noexcept
        {
            if (this != &other)
            {
                Traits::clear(&inner);
                inner = other.inner;
                other.inner = Traits::init();
            }
            return *this;
        }

        size_t size() const
        { 
            return inner.length;
        }
        
        bool empty() const
        {
            return Traits::is_empty(&inner);
        }

        T &front() 
        { 
            if (empty()) throw std::out_of_range("list::front");
            return inner.head->value; 
        }

        const T &front() const 
        { 
            if (empty()) throw std::out_of_range("list::front");
            return inner.head->value; 
        }

        T &back() 
        { 
            if (empty()) throw std::out_of_range("list::back");
            return inner.tail->value; 
        }

        const T &back() const 
        { 
            if (empty()) throw std::out_of_range("list::back");
            return inner.tail->value; 
        }

        void push_back(const T &val) 
        { 
            if (Z_OK != Traits::push_back(&inner, val))
            {
                throw std::bad_alloc();
            }
        }
        
        void push_front(const T &val) 
        { 
            if (Z_OK != Traits::push_front(&inner, val))
            {
                throw std::bad_alloc();
            }
        }

        void pop_back() 
        { 
            if (empty()) throw std::out_of_range("list::pop_back");
            Traits::pop_back(&inner); 
        }

        void pop_front() 
        { 
            if (empty()) throw std::out_of_range("list::pop_front");
            Traits::pop_front(&inner); 
        }

        void clear()
        { 
            Traits::clear(&inner);
        }

        void reverse()
        {
            Traits::reverse(&inner);
        }

        iterator insert_after(iterator pos, const T &val)
        {
            c_node *prev_node = pos.current;
            if (Z_OK != Traits::insert_after(&inner, prev_node, val)) 
            {
                 throw std::bad_alloc();
            }
            return iterator(&inner, prev_node ? prev_node->next : inner.head);
        }

        iterator erase(iterator pos)
        {
            if (nullptr == pos.current)
            {
                throw std::out_of_range("list::erase on end()");
            }
            c_node *to_remove = pos.current;
            c_node *next_node = to_remove->next;
            Traits::remove_node(&inner, to_remove);
            return iterator(&inner, next_node);
        }

        void splice(list &&source)
        {
            Traits::splice(&inner, &source.inner);
        }

        iterator begin() { return iterator(&inner, inner.head); }
        const_iterator begin() const { return const_iterator(&inner, inner.head); }
        const_iterator cbegin() const { return const_iterator(&inner, inner.head); }
        iterator end() { return iterator(&inner, nullptr); }
        const_iterator end() const { return const_iterator(&inner, nullptr); }
        const_iterator cend() const { return const_iterator(&inner, nullptr); }
    };

    template <typename T>
    using list_T = list<T>;
} // namespace z_list

extern "C" {
#endif // __cplusplus

// C implementation logic.

#ifndef ZLIST_MALLOC
    #define ZLIST_MALLOC(sz)      Z_MALLOC(sz)
#endif

#ifndef ZLIST_FREE
    #define ZLIST_FREE(p)         Z_FREE(p)
#endif

// Safe API generator (zerror.h integration).
#if Z_HAS_ZERROR && !defined(__cplusplus)

    static inline zerr zlist_err_impl(int code, const char* msg, 
                                     const char* file, int line, const char* func) 
    {
        return zerr_create_impl(code, file, line, func, "%s", msg);
    }

    #define ZLIST_GEN_SAFE_IMPL(T, Name)                                                        \
        DEFINE_RESULT(T, Res_##Name)                                                            \
                                                                                                \
        static inline zres zlist_push_back_safe_##Name(zlist_##Name *l, T val,                  \
                                                      const char* f, int ln, const char* fn)    \
        {                                                                                       \
            if (Z_OK != zlist_push_back_##Name(l, val))                                         \
            {                                                                                   \
                return zres_err(zlist_err_impl(Z_ENOMEM, "List Push OOM", f, ln, fn));          \
            }                                                                                   \
            return zres_ok();                                                                   \
        }                                                                                       \
                                                                                                \
        static inline zres zlist_push_front_safe_##Name(zlist_##Name *l, T val,                 \
                                                       const char* f, int ln, const char* fn)   \
        {                                                                                       \
            if (Z_OK != zlist_push_front_##Name(l, val))                                        \
            {                                                                                   \
                return zres_err(zlist_err_impl(Z_ENOMEM, "List Push OOM", f, ln, fn));          \
            }                                                                                   \
            return zres_ok();                                                                   \
        }                                                                                       \
                                                                                                \
        static inline Res_##Name zlist_front_safe_##Name(zlist_##Name *l,                       \
                                                        const char* f, int ln, const char* fn)  \
        {                                                                                       \
            if (!l->head)                                                                       \
            {                                                                                   \
                return Res_##Name##_err(zlist_err_impl(Z_EEMPTY, "List is empty", f, ln, fn));  \
            }                                                                                   \
            return Res_##Name##_ok(l->head->value);                                             \
        }                                                                                       \
                                                                                                \
        static inline Res_##Name zlist_back_safe_##Name(zlist_##Name *l,                        \
                                                       const char* f, int ln, const char* fn)   \
        {                                                                                       \
            if (!l->tail)                                                                       \
            {                                                                                   \
                return Res_##Name##_err(zlist_err_impl(Z_EEMPTY, "List is empty", f, ln, fn));  \
            }                                                                                   \
            return Res_##Name##_ok(l->tail->value);                                             \
        }                                                                                       \
                                                                                                \
        static inline zres zlist_pop_back_safe_##Name(zlist_##Name *l,                          \
                                                     const char* f, int ln, const char* fn)     \
        {                                                                                       \
            if (!l->tail)                                                                       \
            {                                                                                   \
                return zres_err(zlist_err_impl(Z_EEMPTY, "List is empty", f, ln, fn));          \
            }                                                                                   \
            zlist_pop_back_##Name(l);                                                           \
            return zres_ok();                                                                   \
        }                                                                                       \
                                                                                                \
        static inline zres zlist_pop_front_safe_##Name(zlist_##Name *l,                         \
                                                      const char* f, int ln, const char* fn)    \
        {                                                                                       \
            if (!l->head)                                                                       \
            {                                                                                   \
                return zres_err(zlist_err_impl(Z_EEMPTY, "List is empty", f, ln, fn));          \
            }                                                                                   \
            zlist_pop_front_##Name(l);                                                          \
            return zres_ok();                                                                   \
        }

#else
#   define ZLIST_GEN_SAFE_IMPL(T, Name)
#endif

/* * Allocation Strategy Injection.
 * Allows C++ to use new/delete (constructors/destructors) 
 * while C uses malloc/free.
 */
#ifdef __cplusplus
    #define ZLIST_IMPL_ALLOC(T, Name)                                                   \
        static inline zlist_node_##Name* zlist_create_node_##Name(T val)                \
        {                                                                               \
            try {                                                                       \
                zlist_node_##Name* n = new zlist_node_##Name;                           \
                n->prev = nullptr;                                                      \
                n->next = nullptr;                                                      \
                n->value = val; /* Invokes copy constructor/assignment */               \
                return n;                                                               \
            } catch (...) { return nullptr; }                                           \
        }                                                                               \
                                                                                        \
        static inline void zlist_free_node_##Name(zlist_node_##Name* n)                 \
        {                                                                               \
            delete n; /* Invokes destructor */                                          \
        }
#else
    #define ZLIST_IMPL_ALLOC(T, Name)                                                   \
        static inline zlist_node_##Name* zlist_create_node_##Name(T val)                \
        {                                                                               \
            zlist_node_##Name* n = (zlist_node_##Name*)                                 \
                                   ZLIST_MALLOC(sizeof(zlist_node_##Name));             \
            if (n) {                                                                    \
                n->value = val;                                                         \
                n->prev = NULL;                                                         \
                n->next = NULL;                                                         \
            }                                                                           \
            return n;                                                                   \
        }                                                                               \
                                                                                        \
        static inline void zlist_free_node_##Name(zlist_node_##Name* n)                 \
        {                                                                               \
            ZLIST_FREE(n);                                                              \
        }
#endif


/*
 * ZLIST_GENERATE_IMPL(T, Name)
 * Generates the complete list implementation.
 */
#define ZLIST_GENERATE_IMPL(T, Name)                                                \
                                                                                    \
/* Node structure. */                                                               \
typedef struct zlist_node_##Name                                                    \
{                                                                                   \
    struct zlist_node_##Name *prev;                                                 \
    struct zlist_node_##Name *next;                                                 \
    T value;                                                                        \
} zlist_node_##Name;                                                                \
                                                                                    \
/* List structure (container). */                                                   \
typedef struct                                                                      \
{                                                                                   \
    zlist_node_##Name *head;                                                        \
    zlist_node_##Name *tail;                                                        \
    size_t length;                                                                  \
} zlist_##Name;                                                                     \
                                                                                    \
/* Inject Allocation Logic (C vs C++). */                                           \
ZLIST_IMPL_ALLOC(T, Name)                                                           \
                                                                                    \
static inline zlist_##Name zlist_init_##Name(void)                                  \
{                                                                                   \
    zlist_##Name l = { NULL, NULL, 0 };                                             \
    return l;                                                                       \
}                                                                                   \
                                                                                    \
static inline bool zlist_is_empty_##Name(const zlist_##Name *l)                     \
{                                                                                   \
    return l->head == NULL;                                                         \
}                                                                                   \
                                                                                    \
static inline void zlist_reverse_##Name(zlist_##Name *l)                            \
{                                                                                   \
    zlist_node_##Name *curr = l->head;                                              \
    zlist_node_##Name *temp = NULL;                                                 \
    while (curr)                                                                    \
    {                                                                               \
        temp = curr->prev;                                                          \
        curr->prev = curr->next;                                                    \
        curr->next = temp;                                                          \
        curr = curr->prev;                                                          \
    }                                                                               \
    if (temp)                                                                       \
    {                                                                               \
        l->tail = l->head;                                                          \
        l->head = temp->prev;                                                       \
    }                                                                               \
}                                                                                   \
                                                                                    \
static inline zlist_node_##Name* zlist_detach_node_##Name(zlist_##Name *l,          \
                                                          zlist_node_##Name *n)     \
{                                                                                   \
    if (!n) return NULL;                                                            \
    if (n->prev) n->prev->next = n->next;                                           \
    else l->head = n->next;                                                         \
    if (n->next) n->next->prev = n->prev;                                           \
    else l->tail = n->prev;                                                         \
    n->prev = n->next = NULL;                                                       \
    l->length--;                                                                    \
    return n;                                                                       \
}                                                                                   \
                                                                                    \
static inline int zlist_push_back_##Name(zlist_##Name *l, T val)                    \
{                                                                                   \
    zlist_node_##Name *n = zlist_create_node_##Name(val);                           \
    if (!n) return Z_ENOMEM;                                                        \
                                                                                    \
    n->prev = l->tail;                                                              \
    if (l->tail) l->tail->next = n;                                                 \
    l->tail = n;                                                                    \
    if (!l->head) l->head = n;                                                      \
    l->length++;                                                                    \
    return Z_OK;                                                                    \
}                                                                                   \
                                                                                    \
static inline int zlist_push_front_##Name(zlist_##Name *l, T val)                   \
{                                                                                   \
    zlist_node_##Name *n = zlist_create_node_##Name(val);                           \
    if (!n) return Z_ENOMEM;                                                        \
                                                                                    \
    n->next = l->head;                                                              \
    if (l->head) l->head->prev = n;                                                 \
    l->head = n;                                                                    \
    if (!l->tail) l->tail = n;                                                      \
    l->length++;                                                                    \
    return Z_OK;                                                                    \
}                                                                                   \
                                                                                    \
static inline int zlist_insert_after_##Name(zlist_##Name *l,                        \
    zlist_node_##Name *prev_node, T val)                                            \
{                                                                                   \
    if (!prev_node) return zlist_push_front_##Name(l, val);                         \
    zlist_node_##Name *n = zlist_create_node_##Name(val);                           \
    if (!n) return Z_ENOMEM;                                                        \
                                                                                    \
    n->prev = prev_node;                                                            \
    n->next = prev_node->next;                                                      \
    if (prev_node->next) prev_node->next->prev = n;                                 \
    else l->tail = n;                                                               \
    prev_node->next = n;                                                            \
    l->length++;                                                                    \
    return Z_OK;                                                                    \
}                                                                                   \
                                                                                    \
static inline void zlist_pop_back_##Name(zlist_##Name *l)                           \
{                                                                                   \
    if (!l->tail) return;                                                           \
    zlist_node_##Name *old_tail = l->tail;                                          \
    l->tail = old_tail->prev;                                                       \
    if (l->tail) l->tail->next = NULL;                                              \
    else l->head = NULL;                                                            \
    zlist_free_node_##Name(old_tail);                                               \
    l->length--;                                                                    \
}                                                                                   \
                                                                                    \
static inline void zlist_pop_front_##Name(zlist_##Name *l)                          \
{                                                                                   \
    if (!l->head) return;                                                           \
    zlist_node_##Name *old_head = l->head;                                          \
    l->head = old_head->next;                                                       \
    if (l->head) l->head->prev = NULL;                                              \
    else l->tail = NULL;                                                            \
    zlist_free_node_##Name(old_head);                                               \
    l->length--;                                                                    \
}                                                                                   \
                                                                                    \
static inline void zlist_remove_node_##Name(zlist_##Name *l, zlist_node_##Name *n)  \
{                                                                                   \
    if (!n) return;                                                                 \
    if (n->prev) n->prev->next = n->next;                                           \
    else l->head = n->next;                                                         \
    if (n->next) n->next->prev = n->prev;                                           \
    else l->tail = n->prev;                                                         \
    zlist_free_node_##Name(n);                                                      \
    l->length--;                                                                    \
}                                                                                   \
                                                                                    \
static inline void zlist_clear_##Name(zlist_##Name *l)                              \
{                                                                                   \
    zlist_node_##Name *curr = l->head;                                              \
    while (curr)                                                                    \
    {                                                                               \
        zlist_node_##Name *next = curr->next;                                       \
        zlist_free_node_##Name(curr);                                               \
        curr = next;                                                                \
    }                                                                               \
    l->head = l->tail = NULL;                                                       \
    l->length = 0;                                                                  \
}                                                                                   \
                                                                                    \
static inline void zlist_splice_##Name(zlist_##Name *dest, zlist_##Name *src)       \
{                                                                                   \
    if (dest == src || !src->head) return;                                          \
    if (!dest->head)                                                                \
    {                                                                               \
        *dest = *src;                                                               \
    }                                                                               \
    else                                                                            \
    {                                                                               \
        dest->tail->next = src->head;                                               \
        src->head->prev = dest->tail;                                               \
        dest->tail = src->tail;                                                     \
        dest->length += src->length;                                                \
    }                                                                               \
    src->head = src->tail = NULL;                                                   \
    src->length = 0;                                                                \
}                                                                                   \
                                                                                    \
static inline zlist_node_##Name *zlist_at_##Name(zlist_##Name *l, size_t index)     \
{                                                                                   \
    if (index >= l->length) return NULL;                                            \
    zlist_node_##Name *curr = l->head;                                              \
    while (index-- > 0) curr = curr->next;                                          \
    return curr;                                                                    \
}                                                                                   \
                                                                                    \
static inline zlist_node_##Name *zlist_head_##Name(zlist_##Name *l)                 \
{                                                                                   \
    return l->head;                                                                 \
}                                                                                   \
                                                                                    \
static inline zlist_node_##Name *zlist_tail_##Name(zlist_##Name *l)                 \
{                                                                                   \
    return l->tail;                                                                 \
}                                                                                   \
                                                                                    \
ZLIST_GEN_SAFE_IMPL(T, Name)

// C Generic dispatch entries.
#define L_IS_EMPTY_ENTRY(T, Name)               zlist_##Name*: zlist_is_empty_##Name,
#define L_CONST_IS_EMPTY_ENTRY(T, Name) const   zlist_##Name*: zlist_is_empty_##Name,
#define L_REVERSE_ENTRY(T, Name)                zlist_##Name*: zlist_reverse_##Name,
#define L_DETACH_ENTRY(T, Name)                 zlist_##Name*: zlist_detach_node_##Name,
#define L_PUSH_B_ENTRY(T, Name)                 zlist_##Name*: zlist_push_back_##Name,
#define L_PUSH_F_ENTRY(T, Name)                 zlist_##Name*: zlist_push_front_##Name,
#define L_INS_A_ENTRY(T, Name)                  zlist_##Name*: zlist_insert_after_##Name,
#define L_POP_B_ENTRY(T, Name)                  zlist_##Name*: zlist_pop_back_##Name,
#define L_POP_F_ENTRY(T, Name)                  zlist_##Name*: zlist_pop_front_##Name,
#define L_REM_N_ENTRY(T, Name)                  zlist_##Name*: zlist_remove_node_##Name,
#define L_CLEAR_ENTRY(T, Name)                  zlist_##Name*: zlist_clear_##Name,
#define L_SPLICE_ENTRY(T, Name)                 zlist_##Name*: zlist_splice_##Name,
#define L_HEAD_ENTRY(T, Name)                   zlist_##Name*: zlist_head_##Name,
#define L_TAIL_ENTRY(T, Name)                   zlist_##Name*: zlist_tail_##Name,
#define L_AT_ENTRY(T, Name)                     zlist_##Name*: zlist_at_##Name,

#if Z_HAS_ZERROR
#   define L_PUSH_B_SAFE_ENTRY(T, Name)  zlist_##Name*: zlist_push_back_safe_##Name,
#   define L_PUSH_F_SAFE_ENTRY(T, Name)  zlist_##Name*: zlist_push_front_safe_##Name,
#   define L_POP_B_SAFE_ENTRY(T, Name)   zlist_##Name*: zlist_pop_back_safe_##Name,
#   define L_POP_F_SAFE_ENTRY(T, Name)   zlist_##Name*: zlist_pop_front_safe_##Name,
#   define L_FRONT_SAFE_ENTRY(T, Name)   zlist_##Name*: zlist_front_safe_##Name,
#   define L_BACK_SAFE_ENTRY(T, Name)    zlist_##Name*: zlist_back_safe_##Name,
#endif

// Registry Loading.
#ifndef REGISTER_ZLIST_TYPES
#   if defined(__has_include) && __has_include("z_registry.h")
#       include "z_registry.h"
#   endif
#endif

#ifndef REGISTER_ZLIST_TYPES
#   define REGISTER_ZLIST_TYPES(X)
#endif

#ifndef Z_AUTOGEN_LISTS
    #define Z_AUTOGEN_LISTS(X)
#endif

// Combine all sources of types.
#define Z_ALL_LISTS(X)      \
    Z_AUTOGEN_LISTS(X)      \
    REGISTER_ZLIST_TYPES(X)

// Execute the generator for all registered types.
Z_ALL_LISTS(ZLIST_GENERATE_IMPL)

// C API macros (using _Generic).
#define zlist_init(Name)         zlist_init_##Name()

#if Z_HAS_CLEANUP
#   define zlist_autofree(Name)  Z_CLEANUP(zlist_clear_##Name) zlist_##Name
#endif

#define zlist_is_empty(l)  _Generic((l),    \
    Z_ALL_LISTS(L_IS_EMPTY_ENTRY)           \
    Z_ALL_LISTS(L_CONST_IS_EMPTY_ENTRY)     \
    default: false) (l)

#define zlist_reverse(l)            _Generic((l),    Z_ALL_LISTS(L_REVERSE_ENTRY) default: (void)0) (l)
#define zlist_detach_node(l, n)     _Generic((l),    Z_ALL_LISTS(L_DETACH_ENTRY)  default: (void*)0) (l, n)
#define zlist_push_back(l, val)     _Generic((l),    Z_ALL_LISTS(L_PUSH_B_ENTRY)  default: 0)         (l, val)
#define zlist_push_front(l, val)    _Generic((l),    Z_ALL_LISTS(L_PUSH_F_ENTRY)  default: 0)         (l, val)
#define zlist_insert_after(l, n, v) _Generic((l),    Z_ALL_LISTS(L_INS_A_ENTRY)   default: 0)         (l, n, v)
#define zlist_pop_back(l)           _Generic((l),    Z_ALL_LISTS(L_POP_B_ENTRY)   default: (void)0)   (l)
#define zlist_pop_front(l)          _Generic((l),    Z_ALL_LISTS(L_POP_F_ENTRY)   default: (void)0)   (l)
#define zlist_remove_node(l, n)     _Generic((l),    Z_ALL_LISTS(L_REM_N_ENTRY)   default: (void)0)   (l, n)
#define zlist_clear(l)              _Generic((l),    Z_ALL_LISTS(L_CLEAR_ENTRY)   default: (void)0)   (l)
#define zlist_splice(dst, src)      _Generic((dst),  Z_ALL_LISTS(L_SPLICE_ENTRY)  default: (void)0)   (dst, src)
#define zlist_head(l)               _Generic((l),    Z_ALL_LISTS(L_HEAD_ENTRY)    default: (void*)0)  (l)
#define zlist_tail(l)               _Generic((l),    Z_ALL_LISTS(L_TAIL_ENTRY)    default: (void*)0)  (l)
#define zlist_at(l, idx)            _Generic((l),    Z_ALL_LISTS(L_AT_ENTRY)      default: (void*)0)  (l, idx)

// Explicit declaration macros
#define zlist_foreach_decl(Name, l, iter) \
    for (zlist_node_##Name *iter = (l)->head; iter != NULL; iter = iter->next)

#define zlist_foreach_safe_decl(Name, l, iter, safe)                            \
    for (zlist_node_##Name *iter = (l)->head, *safe = iter ? iter->next : NULL; \
         iter != NULL;                                                          \
         iter = safe, safe = iter ? iter->next : NULL)

#define zlist_foreach_rev_decl(Name, l, iter) \
    for (zlist_node_##Name *iter = (l)->tail; iter != NULL; iter = iter->prev)

#define zlist_foreach_rev_safe_decl(Name, l, iter, safe)                        \
    for (zlist_node_##Name *iter = (l)->tail, *safe = iter ? iter->prev : NULL; \
         iter != NULL;                                                          \
         iter = safe, safe = iter ? iter->prev : NULL)

// Smart iteration helpers
#if defined(__GNUC__) || defined(__clang__)

#   define zlist_foreach(l, iter) \
        for (__typeof__((l)->head) iter = (l)->head; (iter) != NULL; (iter) = (iter)->next)

#   define zlist_foreach_safe(l, iter, safe_iter)                                               \
        for (__typeof__((l)->head) iter = (l)->head, safe_iter = (iter) ? (iter)->next : NULL;  \
             (iter) != NULL;                                                                    \
             (iter) = (safe_iter), (safe_iter) = (iter) ? (iter)->next : NULL)

#   define zlist_foreach_rev(l, iter) \
        for (__typeof__((l)->tail) iter = (l)->tail; (iter) != NULL; (iter) = (iter)->prev)

#   define zlist_foreach_rev_safe(l, iter, safe_iter)                                           \
        for (__typeof__((l)->tail) iter = (l)->tail, safe_iter = (iter) ? (iter)->prev : NULL;  \
             (iter) != NULL;                                                                    \
             (iter) = (safe_iter), (safe_iter) = (iter) ? (iter)->prev : NULL)

#else
#   define zlist_foreach(l, iter) \
        for ((iter) = (l)->head; (iter) != NULL; (iter) = (iter)->next)

#   define zlist_foreach_safe(l, iter, safe_iter)                               \
        for ((iter) = (l)->head, (safe_iter) = (iter) ? (iter)->next : NULL;    \
             (iter) != NULL;                                                    \
             (iter) = (safe_iter), (safe_iter) = (iter) ? (iter)->next : NULL)

#   define zlist_foreach_rev(l, iter) \
        for ((iter) = (l)->tail; (iter) != NULL; (iter) = (iter)->prev)

#   define zlist_foreach_rev_safe(l, iter, safe_iter)                           \
        for ((iter) = (l)->tail, (safe_iter) = (iter) ? (iter)->prev : NULL;    \
             (iter) != NULL;                                                    \
             (iter) = (safe_iter), (safe_iter) = (iter) ? (iter)->prev : NULL)

#endif

// Safe API macros (conditioned on zerror.h).
#if Z_HAS_ZERROR && !defined(__cplusplus)
    static inline zres zlist_err_dummy(void* v, ...) 
    {
        (void)v;
        return zres_err(zerr_create(-1, "Unknown List Type")); 
    }

#   define zlist_push_back_safe(l, val) \
        _Generic((l), Z_ALL_LISTS(L_PUSH_B_SAFE_ENTRY) default: zlist_err_dummy)(l, val, __FILE__, __LINE__, __func__)

#   define zlist_push_front_safe(l, val) \
        _Generic((l), Z_ALL_LISTS(L_PUSH_F_SAFE_ENTRY) default: zlist_err_dummy)(l, val, __FILE__, __LINE__, __func__)

#   define zlist_pop_back_safe(l) \
        _Generic((l), Z_ALL_LISTS(L_POP_B_SAFE_ENTRY)  default: zlist_err_dummy)(l, __FILE__, __LINE__, __func__)

#   define zlist_pop_front_safe(l) \
        _Generic((l), Z_ALL_LISTS(L_POP_F_SAFE_ENTRY)  default: zlist_err_dummy)(l, __FILE__, __LINE__, __func__)

#   define zlist_front_safe(l) \
        _Generic((l), Z_ALL_LISTS(L_FRONT_SAFE_ENTRY)  default: zlist_err_dummy)(l, __FILE__, __LINE__, __func__)

#   define zlist_back_safe(l) \
        _Generic((l), Z_ALL_LISTS(L_BACK_SAFE_ENTRY)   default: zlist_err_dummy)(l, __FILE__, __LINE__, __func__)
#endif

// Optional short names.
#ifdef ZLIST_SHORT_NAMES
#   define list(Name)                   zlist_##Name
#   define list_node(Name)              zlist_node_##Name
#   define list_init                    zlist_init
#   define list_autofree                zlist_autofree
#   define list_push_back               zlist_push_back
#   define list_push_front              zlist_push_front
#   define list_insert_after            zlist_insert_after
#   define list_pop_back                zlist_pop_back
#   define list_pop_front               zlist_pop_front
#   define list_remove_node             zlist_remove_node
#   define list_clear                   zlist_clear
#   define list_splice                  zlist_splice
#   define list_head                    zlist_head
#   define list_tail                    zlist_tail
#   define list_at                      zlist_at
#   define list_reverse                 zlist_reverse
#   define list_detach_node             zlist_detach_node
#   define list_is_empty                zlist_is_empty
#   define list_foreach_decl            zlist_foreach_decl 
#   define list_foreach_safe_decl       zlist_foreach_safe_decl
#   define list_foreach_rev_decl        zlist_foreach_rev_decl
#   define list_foreach_rev_safe_decl   zlist_foreach_rev_safe_decl
#   define list_foreach                 zlist_foreach
#   define list_foreach_safe            zlist_foreach_safe
#   define list_foreach_rev             zlist_foreach_rev
#   define list_foreach_rev_safe        zlist_foreach_rev_safe

#   if Z_HAS_ZERROR && !defined(__cplusplus)
#       define list_push_back_safe   zlist_push_back_safe
#       define list_push_front_safe  zlist_push_front_safe
#       define list_pop_back_safe    zlist_pop_back_safe
#       define list_pop_front_safe   zlist_pop_front_safe
#       define list_front_safe       zlist_front_safe
#       define list_back_safe        zlist_back_safe
#   endif
#endif

// C++ trait specialization.
#ifdef __cplusplus
} // extern "C"

namespace z_list
{
    #define ZLIST_CPP_TRAITS(T, Name)                                           \
        template<> struct traits<T>                                             \
        {                                                                       \
            using list_type = ::zlist_##Name;                                   \
            using node_type = ::zlist_node_##Name;                              \
            static constexpr auto init = ::zlist_init_##Name;                   \
            static constexpr auto is_empty = ::zlist_is_empty_##Name;           \
            static constexpr auto reverse = ::zlist_reverse_##Name;             \
            static constexpr auto detach = ::zlist_detach_node_##Name;          \
            static constexpr auto push_back = ::zlist_push_back_##Name;         \
            static constexpr auto push_front = ::zlist_push_front_##Name;       \
            static constexpr auto insert_after = ::zlist_insert_after_##Name;   \
            static constexpr auto pop_back = ::zlist_pop_back_##Name;           \
            static constexpr auto pop_front = ::zlist_pop_front_##Name;         \
            static constexpr auto remove_node = ::zlist_remove_node_##Name;     \
            static constexpr auto clear = ::zlist_clear_##Name;                 \
            static constexpr auto splice = ::zlist_splice_##Name;               \
            static constexpr auto head = ::zlist_head_##Name;                   \
            static constexpr auto tail = ::zlist_tail_##Name;                   \
        };

    Z_ALL_LISTS(ZLIST_CPP_TRAITS)
} // namespace z_list
#endif // __cplusplus

#endif // ZLIST_H
