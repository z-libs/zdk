/*
 * GENERATED FILE - DO NOT EDIT DIRECTLY
 * Source: ztree.c
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


/* ztree.h — Type-safe Red-Black Tree (Ordered Map/Set)
 * Part of Zen Development Kit (ZDK)
 *
 * This is a macro-generated, single-header balanced binary search tree library.
 * It produces fully type-safe implementations at compile time with zero
 * runtime overhead and full C11 _Generic + C++ RAII support.
 *
 * Features:
 * • O(log n) insert, find, and remove.
 * • Keys are always sorted.
 * • Supports lower_bound (range queries).
 * • C++ z_tree::map<K, V> with RAII, iterators, and operator[].
 *
 * License: MIT
 * Author: Zuhaitz
 * Repository: https://github.com/z-libs/ztree.h
 * Version: 1.1.0
 */

#ifndef ZTREE_H
#define ZTREE_H
// [Bundled] "zcommon.h" is included inline in this same file

#ifdef __cplusplus
#include <iostream>
#include <stdexcept>
#include <iterator>
#include <utility>
#include <type_traits>

namespace z_tree {
    template <typename K, typename V> class map;
    template <typename K, typename V> class map_iterator;

    template <typename K, typename V>
    struct traits 
    {
        static_assert(0 == sizeof(K), "No ztree implementation registered for this Key/Value pair.");
    };
    
template <typename K, typename V>
    class map_iterator 
    {
     public:
        using value_type = V;
        using reference = V&;
        using pointer = V*;
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = ptrdiff_t;

        using Traits = traits<typename std::remove_const<K>::type, typename std::remove_const<V>::type>;
        using CNode = typename Traits::node_type;
        using CTree = typename Traits::tree_type;

        struct EntryProxy 
        {
            CNode *node;
            const K &key() const
            { 
                return node->key;
            }

            V &value() const
            {
                return node->value;
            }
            const K &first() const
            {
                return node->key;
            }

            V &second() const
            {
                return node->value;
            }
        };

        explicit map_iterator(CNode *p, const CTree *t) : current(p), tree(t) {}

        const K &key() const
        {
            return current->key;
        }

        V &value() const
        {
            return current->value;
        }

        EntryProxy operator*() const
        {
            return EntryProxy{current};
        }

        V *operator->() const
        {
            return &current->value;
        }

        bool operator==(const map_iterator &other) const
        {
            return current == other.current;
        }

        bool operator!=(const map_iterator &other) const
        {
            return current != other.current;
        }

        map_iterator &operator++() 
        { 
            current = Traits::next(current); 
            return *this; 
        }

        map_iterator &operator--()
        {  
            if (!current) 
            {
                current = Traits::max(const_cast<CTree*>(tree));
            } 
            else 
            {
                current = Traits::prev(current); 
            }
            return *this; 
        }

     private:
        CNode *current;
        const CTree *tree;
        friend class map<K, V>;
    };

    template <typename K, typename V>
    class map 
    {
        using Traits = traits<K, V>;
        using CTree = typename Traits::tree_type;
     public:
        using iterator = map_iterator<K, V>;
        CTree inner;

        map()
        {
            inner = Traits::init();
        }

        ~map()
        {
            Traits::clear(&inner);
        }

        map(const map&) = delete;
        map &operator=(const map&) = delete;

        map(map &&other) noexcept : inner(other.inner)
        {
            other.inner = Traits::init();
        }

        map &operator=(map &&other) noexcept
        {
            if (this != &other)
            {
                Traits::clear(&inner); 
                inner = other.inner; 
                other.inner = Traits::init(); 
            }
            return *this;
        }

        void insert(K k, V v) 
        {
            if (0 != Traits::insert(&inner, k, v))
            {
                throw std::bad_alloc();
            }
        }

        void erase(K k)
        {
            Traits::remove(&inner, k);
        }
        
        iterator erase(iterator pos)
        {
            iterator next = pos; ++next;
            Traits::remove(&inner, pos.key());
            return next;
        }

        V *find(K k)
        {
            auto *n = Traits::find(&inner, k);
            return n ? &n->value : nullptr; 
        }
        
        V &operator[](const K &k)
        {
            auto *n = Traits::find(&inner, k);
            if (!n)
            { 
                Traits::insert(&inner, k, V{}); 
                n = Traits::find(&inner, k); 
            }
            return n->value;
        }

        iterator lower_bound(const K &k)
        {
            return iterator(Traits::lower_bound(&inner, k), &inner);
        }

        iterator begin()
        {
            return iterator(Traits::min(&inner), &inner);
        }

        iterator end()
        {
            return iterator(nullptr, &inner);
        }
        
        size_t size() const
        {
            return inner.size;
        }

        bool empty() const
        {
            return 0 == inner.size;
        }

        void clear()
        {
            Traits::clear(&inner);
        }
    };
}
extern "C" {
#endif

// Allocator overrides.
#ifndef ZTREE_MALLOC
#   define ZTREE_MALLOC(sz)      Z_MALLOC(sz)
#endif

#ifndef ZTREE_CALLOC
#   define ZTREE_CALLOC(n, sz)   Z_CALLOC(n, sz)
#endif

#ifndef ZTREE_REALLOC
#   define ZTREE_REALLOC(p, sz)  Z_REALLOC(p, sz)
#endif

#ifndef ZTREE_FREE
#   define ZTREE_FREE(p)         Z_FREE(p)
#endif

typedef enum 
{ 
    ZTREE_RED, 
    ZTREE_BLACK 
} ztree_color;

#ifdef __cplusplus
#   define ZTREE_NEW_NODE(Type, n)  Type *n = new Type()
#   define ZTREE_FREE_NODE(n)       delete n
#else
    
#   define ZTREE_NEW_NODE(Type, n)                      \
        Type *n = (Type*)ZTREE_MALLOC(sizeof(Type));    \
        if (n)                                          \
        {                                               \
            n->parent = n->left = n->right = NULL;      \
        }

#   define ZTREE_FREE_NODE(n)       ZTREE_FREE(n)
#endif

#define ZTREE_GENERATE_IMPL(Key, Val, Name, Cmp)                                                                \
                                                                                                                \
    typedef struct ztree_node_##Name                                                                            \
    {                                                                                                           \
        Key key;                                                                                                \
        Val value;                                                                                              \
        ztree_color color;                                                                                      \
        struct ztree_node_##Name *parent, *left, *right;                                                        \
    } ztree_node_##Name;                                                                                        \
                                                                                                                \
    typedef struct                                                                                              \
    {                                                                                                           \
        ztree_node_##Name *root;                                                                                \
        size_t size;                                                                                            \
    } ztree_##Name;                                                                                             \
                                                                                                                \
    static inline ztree_##Name ztree_init_##Name(void)                                                          \
    {                                                                                                           \
        ztree_##Name t = {NULL, 0};                                                                             \
        return t;                                                                                               \
    }                                                                                                           \
                                                                                                                \
    static inline ztree_node_##Name *ztree__new_##Name(Key k, Val v)                                            \
    {                                                                                                           \
        ZTREE_NEW_NODE(ztree_node_##Name, n);                                                                   \
        if(n)                                                                                                   \
        {                                                                                                       \
            n->key = k;                                                                                         \
            n->value = v;                                                                                       \
            n->color = ZTREE_RED;                                                                               \
            n->parent = n->left = n->right = NULL;                                                              \
        }                                                                                                       \
        return n;                                                                                               \
    }                                                                                                           \
                                                                                                                \
    static inline void ztree__free_rec_##Name(ztree_node_##Name *n)                                             \
    {                                                                                                           \
        if (!n)                                                                                                 \
        {                                                                                                       \
            return;                                                                                             \
        }                                                                                                       \
        ztree__free_rec_##Name(n->left);                                                                        \
        ztree__free_rec_##Name(n->right);                                                                       \
        ZTREE_FREE_NODE(n);                                                                                     \
    }                                                                                                           \
                                                                                                                \
    static inline void ztree_clear_##Name(ztree_##Name *t)                                                      \
    {                                                                                                           \
        ztree__free_rec_##Name(t->root);                                                                        \
        t->root = NULL;                                                                                         \
        t->size = 0;                                                                                            \
    }                                                                                                           \
                                                                                                                \
    static inline void ztree__rot_l_##Name(ztree_##Name *t, ztree_node_##Name *x)                               \
    {                                                                                                           \
        ztree_node_##Name *y = x->right;                                                                        \
        x->right = y->left;                                                                                     \
        if (y->left)                                                                                            \
        {                                                                                                       \
            y->left->parent = x;                                                                                \
            y->parent = x->parent;                                                                              \
        }                                                                                                       \
        y->parent = x->parent;                                                                                  \
        if (!x->parent)                                                                                         \
        {                                                                                                       \
            t->root = y;                                                                                        \
        }                                                                                                       \
        else if (x == x->parent->left)                                                                          \
        {                                                                                                       \
            x->parent->left = y;                                                                                \
        }                                                                                                       \
        else                                                                                                    \
        {                                                                                                       \
            x->parent->right = y;                                                                               \
        }                                                                                                       \
        y->left = x;                                                                                            \
        x->parent = y;                                                                                          \
    }                                                                                                           \
                                                                                                                \
    static inline void ztree__rot_r_##Name(ztree_##Name *t, ztree_node_##Name *y)                               \
    {                                                                                                           \
        ztree_node_##Name *x = y->left;                                                                         \
        y->left = x->right;                                                                                     \
        if (x->right)                                                                                           \
        {                                                                                                       \
            x->right->parent = y;                                                                               \
            x->parent = y->parent;                                                                              \
        }                                                                                                       \
        x->parent = y->parent;                                                                                  \
        if (!y->parent)                                                                                         \
        {                                                                                                       \
            t->root = x;                                                                                        \
        }                                                                                                       \
        else if (y == y->parent->left)                                                                          \
        {                                                                                                       \
            y->parent->left = x;                                                                                \
        }                                                                                                       \
        else                                                                                                    \
        {                                                                                                       \
            y->parent->right = x;                                                                               \
        }                                                                                                       \
        x->right = y;                                                                                           \
        y->parent = x;                                                                                          \
    }                                                                                                           \
                                                                                                                \
    static inline void ztree__fix_ins_##Name(ztree_##Name *t, ztree_node_##Name *z)                             \
    {                                                                                                           \
        while (z->parent && ZTREE_RED == z->parent->color)                                                      \
        {                                                                                                       \
            if (z->parent == z->parent->parent->left)                                                           \
            {                                                                                                   \
                ztree_node_##Name *y = z->parent->parent->right;                                                \
                if (y && ZTREE_RED == y->color)                                                                 \
                {                                                                                               \
                    z->parent->color = ZTREE_BLACK;                                                             \
                    y->color = ZTREE_BLACK;                                                                     \
                    z->parent->parent->color = ZTREE_RED;                                                       \
                    z = z->parent->parent;                                                                      \
                }                                                                                               \
                else                                                                                            \
                {                                                                                               \
                    if (z == z->parent->right)                                                                  \
                    {                                                                                           \
                        z = z->parent;                                                                          \
                        ztree__rot_l_##Name(t, z);                                                              \
                    }                                                                                           \
                    z->parent->color = ZTREE_BLACK;                                                             \
                    z->parent->parent->color = ZTREE_RED;                                                       \
                    ztree__rot_r_##Name(t, z->parent->parent);                                                  \
                }                                                                                               \
            }                                                                                                   \
            else                                                                                                \
            {                                                                                                   \
                ztree_node_##Name *y = z->parent->parent->left;                                                 \
                if (y && ZTREE_RED == y->color)                                                                 \
                {                                                                                               \
                    z->parent->color = ZTREE_BLACK;                                                             \
                    y->color = ZTREE_BLACK;                                                                     \
                    z->parent->parent->color = ZTREE_RED;                                                       \
                    z = z->parent->parent;                                                                      \
                }                                                                                               \
                else                                                                                            \
                {                                                                                               \
                    if (z == z->parent->left)                                                                   \
                    {                                                                                           \
                        z = z->parent;                                                                          \
                        ztree__rot_r_##Name(t, z);                                                              \
                    }                                                                                           \
                    z->parent->color = ZTREE_BLACK;                                                             \
                    z->parent->parent->color = ZTREE_RED;                                                       \
                    ztree__rot_l_##Name(t, z->parent->parent);                                                  \
                }                                                                                               \
            }                                                                                                   \
        }                                                                                                       \
        t->root->color = ZTREE_BLACK;                                                                           \
    }                                                                                                           \
                                                                                                                \
    static inline void ztree__fix_del_##Name(ztree_##Name *t, ztree_node_##Name *x, ztree_node_##Name *p)       \
    {                                                                                                           \
        while (x != t->root && (!x || x->color == ZTREE_BLACK))                                                 \
        {                                                                                                       \
            if (x == p->left)                                                                                   \
            {                                                                                                   \
                ztree_node_##Name *w = p->right;                                                                \
                if (ZTREE_RED == w->color)                                                                      \
                {                                                                                               \
                    w->color = ZTREE_BLACK;                                                                     \
                    p->color = ZTREE_RED;                                                                       \
                    ztree__rot_l_##Name(t, p);                                                                  \
                    w = p->right;                                                                               \
                }                                                                                               \
                if ((!w->left || ZTREE_BLACK == w->left->color) &&                                              \
                    (!w->right || ZTREE_BLACK == w->right->color))                                              \
                    {                                                                                           \
                    w->color = ZTREE_RED;                                                                       \
                    x = p;                                                                                      \
                    p = x->parent;                                                                              \
                }                                                                                               \
                else                                                                                            \
                {                                                                                               \
                    if (!w->right || ZTREE_BLACK == w->right->color)                                            \
                    {                                                                                           \
                        if (w->left)                                                                            \
                        {                                                                                       \
                            w->left->color = ZTREE_BLACK;                                                       \
                        }                                                                                       \
                        w->color = ZTREE_RED;                                                                   \
                        ztree__rot_r_##Name(t, w);                                                              \
                        w = p->right;                                                                           \
                    }                                                                                           \
                    w->color = p->color;                                                                        \
                    p->color = ZTREE_BLACK;                                                                     \
                    if (w->right)                                                                               \
                    {                                                                                           \
                        w->right->color = ZTREE_BLACK;                                                          \
                    }                                                                                           \
                    ztree__rot_l_##Name(t, p);                                                                  \
                    x = t->root;                                                                                \
                }                                                                                               \
            }                                                                                                   \
            else                                                                                                \
            {                                                                                                   \
                ztree_node_##Name *w = p->left;                                                                 \
                if (ZTREE_RED == w->color)                                                                      \
                {                                                                                               \
                    w->color = ZTREE_BLACK;                                                                     \
                    p->color = ZTREE_RED;                                                                       \
                    ztree__rot_r_##Name(t, p);                                                                  \
                    w = p->left;                                                                                \
                }                                                                                               \
                if ((!w->right || ZTREE_BLACK == w->right->color) &&                                            \
                    (!w->left || ZTREE_BLACK == w->left->color))                                                \
                {                                                                                               \
                    w->color = ZTREE_RED;                                                                       \
                    x = p;                                                                                      \
                    p = x->parent;                                                                              \
                }                                                                                               \
                else                                                                                            \
                {                                                                                               \
                    if (!w->left || ZTREE_BLACK == w->left->color)                                              \
                    {                                                                                           \
                        if (w->right)                                                                           \
                        {                                                                                       \
                            w->right->color = ZTREE_BLACK;                                                      \
                        }                                                                                       \
                        w->color = ZTREE_RED;                                                                   \
                        ztree__rot_l_##Name(t, w);                                                              \
                        w = p->left;                                                                            \
                    }                                                                                           \
                    w->color = p->color;                                                                        \
                    p->color = ZTREE_BLACK;                                                                     \
                    if (w->left)                                                                                \
                    {                                                                                           \
                        w->left->color = ZTREE_BLACK;                                                           \
                    }                                                                                           \
                    ztree__rot_r_##Name(t, p);                                                                  \
                    x = t->root;                                                                                \
                }                                                                                               \
            }                                                                                                   \
        }                                                                                                       \
        if (x)                                                                                                  \
        {                                                                                                       \
            x->color = ZTREE_BLACK;                                                                             \
        }                                                                                                       \
    }                                                                                                           \
                                                                                                                \
    static inline void ztree__transplant_##Name(ztree_##Name *t, ztree_node_##Name *u, ztree_node_##Name *v)    \
    {                                                                                                           \
        if (!u->parent)                                                                                         \
        {                                                                                                       \
            t->root = v;                                                                                        \
        }                                                                                                       \
        else if (u == u->parent->left)                                                                          \
        {                                                                                                       \
            u->parent->left = v;                                                                                \
        }                                                                                                       \
        else                                                                                                    \
        {                                                                                                       \
            u->parent->right = v;                                                                               \
        }                                                                                                       \
        if (v)                                                                                                  \
        {                                                                                                       \
            v->parent = u->parent;                                                                              \
        }                                                                                                       \
    }                                                                                                           \
                                                                                                                \
    static inline ztree_node_##Name* ztree_find_##Name(ztree_##Name *t, Key k)                                  \
    {                                                                                                           \
        ztree_node_##Name *x = t->root;                                                                         \
        while (x)                                                                                               \
        {                                                                                                       \
            int cmp = Cmp(&k, &x->key);                                                                         \
            if (0 == cmp)                                                                                       \
            {                                                                                                   \
                return x;                                                                                       \
            }                                                                                                   \
            x = (cmp < 0) ? x->left : x->right;                                                                 \
        }                                                                                                       \
        return NULL;                                                                                            \
    }                                                                                                           \
                                                                                                                \
    static inline void ztree_remove_##Name(ztree_##Name *t, Key k)                                              \
    {                                                                                                           \
        ztree_node_##Name *z = ztree_find_##Name(t, k);                                                         \
        if (!z)                                                                                                 \
        {                                                                                                       \
            return;                                                                                             \
        }                                                                                                       \
        ztree_node_##Name *y = z, *x;                                                                           \
        ztree_node_##Name *x_parent = NULL;                                                                     \
        ztree_color y_orig_color = y->color;                                                                    \
        if (!z->left)                                                                                           \
        {                                                                                                       \
            x = z->right;                                                                                       \
            x_parent = z->parent;                                                                               \
            ztree__transplant_##Name(t, z, z->right);                                                           \
        }                                                                                                       \
        else if (!z->right)                                                                                     \
        {                                                                                                       \
            x = z->left;                                                                                        \
            x_parent = z->parent;                                                                               \
            ztree__transplant_##Name(t, z, z->left);                                                            \
        }                                                                                                       \
        else                                                                                                    \
        {                                                                                                       \
            y = z->right;                                                                                       \
            while (y->left)                                                                                     \
            {                                                                                                   \
                y = y->left;                                                                                    \
            }                                                                                                   \
            y_orig_color = y->color;                                                                            \
            x = y->right;                                                                                       \
            if (y->parent == z)                                                                                 \
            {                                                                                                   \
                x_parent = y;                                                                                   \
            }                                                                                                   \
            else                                                                                                \
            {                                                                                                   \
                x_parent = y->parent;                                                                           \
                ztree__transplant_##Name(t, y, y->right);                                                       \
                y->right = z->right;                                                                            \
                y->right->parent = y;                                                                           \
            }                                                                                                   \
            ztree__transplant_##Name(t, z, y);                                                                  \
            y->left = z->left;                                                                                  \
            y->left->parent = y;                                                                                \
            y->color = z->color;                                                                                \
        }                                                                                                       \
        if (ZTREE_BLACK == y_orig_color)                                                                        \
        {                                                                                                       \
            ztree__fix_del_##Name(t, x, x_parent);                                                              \
        }                                                                                                       \
        ZTREE_FREE_NODE(z);                                                                                     \
        t->size--;                                                                                              \
    }                                                                                                           \
                                                                                                                \
    static inline int ztree_insert_##Name(ztree_##Name *t, Key k, Val v)                                        \
    {                                                                                                           \
        ztree_node_##Name *y = NULL, *x = t->root;                                                              \
        while (x)                                                                                               \
        {                                                                                                       \
            y = x;                                                                                              \
            int cmp = Cmp(&k, &x->key);                                                                         \
            if (0 == cmp)                                                                                       \
            {                                                                                                   \
                x->value = v;                                                                                   \
                return Z_OK;                                                                                    \
            }                                                                                                   \
            x = (cmp < 0) ? x->left : x->right;                                                                 \
        }                                                                                                       \
        ztree_node_##Name *z = ztree__new_##Name(k, v);                                                         \
        if (!z)                                                                                                 \
        {                                                                                                       \
            return Z_ENOMEM;                                                                                    \
        }                                                                                                       \
        z->parent = y;                                                                                          \
        if (!y)                                                                                                 \
        {                                                                                                       \
            t->root = z;                                                                                        \
        }                                                                                                       \
        else if (Cmp(&k, &y->key) < 0)                                                                          \
        {                                                                                                       \
            y->left = z;                                                                                        \
        }                                                                                                       \
        else                                                                                                    \
        {                                                                                                       \
            y->right = z;                                                                                       \
        }                                                                                                       \
        ztree__fix_ins_##Name(t, z);                                                                            \
        t->size++;                                                                                              \
        return Z_OK;                                                                                            \
    }                                                                                                           \
                                                                                                                \
    static inline ztree_node_##Name* ztree_lower_bound_##Name(ztree_##Name *t, Key k)                           \
    {                                                                                                           \
        ztree_node_##Name *curr = t->root, *res = NULL;                                                         \
        while (curr)                                                                                            \
        {                                                                                                       \
            int cmp = Cmp(&k, &curr->key);                                                                      \
            if (0 == cmp)                                                                                       \
            {                                                                                                   \
                return curr;                                                                                    \
            }                                                                                                   \
            if (cmp < 0)                                                                                        \
            {                                                                                                   \
                res = curr;                                                                                     \
                curr = curr->left;                                                                              \
            }                                                                                                   \
            else                                                                                                \
            {                                                                                                   \
                curr = curr->right;                                                                             \
            }                                                                                                   \
        } return res;                                                                                           \
    }                                                                                                           \
                                                                                                                \
    static inline ztree_node_##Name* ztree_min_##Name(ztree_##Name *t)                                          \
    {                                                                                                           \
        ztree_node_##Name *n = t->root;                                                                         \
        while (n && n->left)                                                                                    \
        {                                                                                                       \
            n = n->left;                                                                                        \
        }                                                                                                       \
        return n;                                                                                               \
    }                                                                                                           \
                                                                                                                \
    static inline ztree_node_##Name* ztree_max_##Name(ztree_##Name *t)                                          \
    {                                                                                                           \
        ztree_node_##Name *n = t->root;                                                                         \
        while (n && n->right)                                                                                   \
        {                                                                                                       \
            n = n->right;                                                                                       \
        }                                                                                                       \
        return n;                                                                                               \
    }                                                                                                           \
                                                                                                                \
    static inline ztree_node_##Name* ztree_next_##Name(ztree_node_##Name *n)                                    \
    {                                                                                                           \
        if (!n)                                                                                                 \
        {                                                                                                       \
            return NULL;                                                                                        \
        }                                                                                                       \
        if (n->right)                                                                                           \
        {                                                                                                       \
            n = n->right;                                                                                       \
            while (n->left)                                                                                     \
            {                                                                                                   \
                n = n->left;                                                                                    \
            }                                                                                                   \
            return n;                                                                                           \
        }                                                                                                       \
        ztree_node_##Name *p = n->parent;                                                                       \
        while (p && n == p->right)                                                                              \
        {                                                                                                       \
            n = p;                                                                                              \
            p = p->parent;                                                                                      \
        }                                                                                                       \
        return p;                                                                                               \
    }                                                                                                           \
                                                                                                                \
    static inline ztree_node_##Name* ztree_prev_##Name(ztree_node_##Name *n)                                    \
    {                                                                                                           \
        if (!n)                                                                                                 \
        {                                                                                                       \
            return NULL;                                                                                        \
        }                                                                                                       \
        if (n->left)                                                                                            \
        {                                                                                                       \
            n = n->left;                                                                                        \
            while (n->right)                                                                                    \
            {                                                                                                   \
                n = n->right;                                                                                   \
            }                                                                                                   \
            return n;                                                                                           \
        }                                                                                                       \
        ztree_node_##Name *p = n->parent;                                                                       \
        while (p && n == p->left)                                                                               \
        {                                                                                                       \
            n = p;                                                                                              \
            p = p->parent;                                                                                      \
        }                                                                                                       \
        return p;                                                                                               \
    }

#ifndef REGISTER_ZTREE_TYPES
#   if defined(__has_include) && __has_include("z_registry.h")
#       include "z_registry.h"
#   endif
#endif

#ifndef REGISTER_ZTREE_TYPES
#   define REGISTER_ZTREE_TYPES(X)
#endif

#ifndef Z_AUTOGEN_TREES
#   define Z_AUTOGEN_TREES(X)
#endif

#define Z_ALL_TREES(X) Z_AUTOGEN_TREES(X) REGISTER_ZTREE_TYPES(X)

Z_ALL_TREES(ZTREE_GENERATE_IMPL)

#define T_INSERT_ENTRY(K, V, Name, Cmp)   ztree_##Name*: ztree_insert_##Name,
#define T_FIND_ENTRY(K, V, Name, Cmp)     ztree_##Name*: ztree_find_##Name,
#define T_LB_ENTRY(K, V, Name, Cmp)       ztree_##Name*: ztree_lower_bound_##Name,
#define T_REM_ENTRY(K, V, Name, Cmp)      ztree_##Name*: ztree_remove_##Name,
#define T_CLEAR_ENTRY(K, V, Name, Cmp)    ztree_##Name*: ztree_clear_##Name,
#define T_MIN_ENTRY(K, V, Name, Cmp)      ztree_##Name*: ztree_min_##Name,
#define T_MAX_ENTRY(K, V, Name, Cmp)      ztree_##Name*: ztree_max_##Name,
#define T_NEXT_ENTRY(K, V, Name, Cmp)     ztree_node_##Name*: ztree_next_##Name,
#define T_PREV_ENTRY(K, V, Name, Cmp)     ztree_node_##Name*: ztree_prev_##Name,

#define ztree_init(Name)             ztree_init_##Name()

#if defined(__GNUC__) || defined(__clang__)
#   define ztree_autofree(Name)     __attribute__((cleanup(ztree_clear_##Name))) ztree_##Name
#endif

#define ztree_insert(t, k, v)   _Generic((t), Z_ALL_TREES(T_INSERT_ENTRY) default: 0)       (t, k, v)
#define ztree_remove(t, k)      _Generic((t), Z_ALL_TREES(T_REM_ENTRY)    default: (void)0) (t, k)
#define ztree_find(t, k)        _Generic((t), Z_ALL_TREES(T_FIND_ENTRY)   default: NULL)    (t, k)
#define ztree_lower_bound(t,k)  _Generic((t), Z_ALL_TREES(T_LB_ENTRY)     default: NULL)    (t, k)
#define ztree_clear(t)          _Generic((t), Z_ALL_TREES(T_CLEAR_ENTRY)  default: (void)0) (t)
#define ztree_min(t)            _Generic((t), Z_ALL_TREES(T_MIN_ENTRY)    default: NULL)    (t)
#define ztree_max(t)            _Generic((t), Z_ALL_TREES(T_MAX_ENTRY)    default: NULL)    (t)
#define ztree_next(n)           _Generic((n), Z_ALL_TREES(T_NEXT_ENTRY)   default: NULL)    (n)
#define ztree_prev(n)           _Generic((n), Z_ALL_TREES(T_PREV_ENTRY)   default: NULL)    (n)

// Iteration macros.
#if defined(__GNUC__) || defined(__clang__)

#   define ztree_foreach(t, iter) \
        for (__typeof__(ztree_min(t)) iter = ztree_min(t); (iter) != NULL; (iter) = ztree_next(iter))

#   define ztree_foreach_safe(t, iter, safe)                                        \
        for (__typeof__(ztree_min(t)) iter = ztree_min(t), safe = ztree_next(iter); \
             (iter) != NULL;                                                        \
             (iter) = (safe), (safe) = (safe) ? ztree_next(safe) : NULL)

#   define ztree_foreach_reverse(t, iter) \
        for (__typeof__(ztree_max(t)) iter = ztree_max(t); (iter) != NULL; (iter) = ztree_prev(iter))
#else

#   define ztree_foreach(t, iter) \
        for ((iter) = ztree_min(t); (iter) != NULL; (iter) = ztree_next(iter))

#   define ztree_foreach_safe(t, iter, safe)                    \
        for ((iter) = ztree_min(t), (safe) = ztree_next(iter);  \
             (iter) != NULL;                                    \
             (iter) = (safe), (safe) = (safe) ? ztree_next(safe) : NULL)

#   define ztree_foreach_reverse(t, iter) \
        for ((iter) = ztree_max(t); (iter) != NULL; (iter) = ztree_prev(iter))
#endif

#ifdef ZTREE_SHORT_NAMES
#   define tree(Name)              ztree_##Name
#   define tree_init        ztree_init
#   define tree_insert      ztree_insert
#   define tree_remove      ztree_remove
#   define tree_find        ztree_find
#   define tree_lower_bound ztree_lower_bound
#   define tree_clear       ztree_clear
#   define tree_min         ztree_min
#   define tree_max         ztree_max
#   define tree_next        ztree_next
#   define tree_prev        ztree_prev
#   define tree_foreach     ztree_foreach
#   define tree_foreach_safe ztree_foreach_safe
#endif

#ifdef __cplusplus
} // extern "C"
namespace z_tree 
{
#   define ZTREE_CPP_TRAITS(Key, Val, Name, Cmp)                            \
        template<> struct traits<Key, Val>                                  \
        {                                                                   \
            using tree_type = ::ztree_##Name;                               \
            using node_type = ::ztree_node_##Name;                          \
            static constexpr auto init = ::ztree_init_##Name;               \
            static constexpr auto insert = ::ztree_insert_##Name;           \
            static constexpr auto remove = ::ztree_remove_##Name;           \
            static constexpr auto find = ::ztree_find_##Name;               \
            static constexpr auto lower_bound = ::ztree_lower_bound_##Name; \
            static constexpr auto clear = ::ztree_clear_##Name;             \
            static constexpr auto min = ::ztree_min_##Name;                 \
            static constexpr auto max = ::ztree_max_##Name;                 \
            static constexpr auto next = ::ztree_next_##Name;               \
            static constexpr auto prev = ::ztree_prev_##Name;               \
        };
    Z_ALL_TREES(ZTREE_CPP_TRAITS)
}
#endif
#endif // ZTREE_H
