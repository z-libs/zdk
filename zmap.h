/*
 * GENERATED FILE - DO NOT EDIT DIRECTLY
 * Source: zmap.c
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

#ifndef ZCOMMON_H
#define ZCOMMON_H

#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

// Return Codes.
#define Z_OK     0
#define Z_ERR   -1
#define Z_FOUND  1

// Memory Macros.
// If the user hasn't defined their own allocator, use the standard one.
#ifndef Z_MALLOC
    #include <stdlib.h>
    #define Z_MALLOC(sz)       malloc(sz)
    #define Z_CALLOC(n, sz)    calloc(n, sz)
    #define Z_REALLOC(p, sz)   realloc(p, sz)
    #define Z_FREE(p)          free(p)
#endif

// Compiler Extensions (Optional).
// We check for GCC/Clang features to enable RAII-style cleanup.
// Define Z_NO_EXTENSIONS to disable this manually.
#if !defined(Z_NO_EXTENSIONS) && (defined(__GNUC__) || defined(__clang__))
        
#   define Z_HAS_CLEANUP 1
    
    // RAII Cleanup (destructors).
#   define Z_CLEANUP(func) __attribute__((cleanup(func)))
    
    // Warn if the return value (for example, an Error Result) is ignored.
    #define Z_NODISCARD     __attribute__((warn_unused_result))
    
    // Branch prediction hints.
#   define Z_LIKELY(x)     __builtin_expect(!!(x), 1)
#   define Z_UNLIKELY(x)   __builtin_expect(!!(x), 0)

#else
        
#   define Z_HAS_CLEANUP 0
#   define Z_CLEANUP(func) 
#   define Z_NODISCARD
#   define Z_LIKELY(x)     (x)
#   define Z_UNLIKELY(x)   (x)

#endif

// Metaprogramming Markers.
// These macros are used by the Z-Scanner tool to find type definitions.
// For the C compiler, they are no-ops (they compile to nothing).
#define DEFINE_VEC_TYPE(T, Name)
#define DEFINE_LIST_TYPE(T, Name)
#define DEFINE_MAP_TYPE(Key, Val, Name)
#define DEFINE_STABLE_MAP_TYPE(Key, Val, Name)

// Token concatenation macros (useful for unique variable names in defer)
#define Z_CONCAT_(a, b) a ## b
#define Z_CONCAT(a, b) Z_CONCAT_(a, b)
#define Z_UNIQUE(prefix) Z_CONCAT(prefix, __LINE__)

// Growth Strategy.
// Determines how containers expand when full.
// Default is 2.0x (Geometric Growth).
//
// Optimization Note:
// 2.0x minimizes realloc calls but can waste memory.
// 1.5x is often better for memory fragmentation and reuse.
#ifndef Z_GROWTH_FACTOR
    // Default: Double capacity (2.0x)
    #define Z_GROWTH_FACTOR(cap) ((cap) == 0 ? 32 : (cap) * 2)
    
    // Alternative: 1.5x Growth (Uncomment to use)
    // #define Z_GROWTH_FACTOR(cap) ((cap) == 0 ? 32 : (cap) + (cap) / 2)
#endif

#endif


#endif // Z_COMMON_BUNDLED
/* ============================================================================ */

#ifndef ZMAP_H
#define ZMAP_H
// [Bundled] "zcommon.h" is included inline in this same file
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
#include <stdexcept>
#include <iterator>
#include <utility>
#include <type_traits>

namespace z_map
{
    /* Forward declarations. */
    template <typename K, typename V> struct map;
    template <typename K, typename V> class map_iterator;

    /* Base traits struct (specialized by macros later). */
    template <typename K, typename V>
    struct traits
    {
        static_assert(sizeof(K) == 0, "No zmap implementation registered for this key/value pair.");
    };

    /* * Forward iterator for the map.
     * Iterates over buckets, skipping EMPTY states.
     */
    template <typename K, typename V>
    class map_iterator
    {
    public:
        // Detect const-ness for correct iterator_traits.
        static constexpr bool is_const = std::is_const<K>::value || std::is_const<V>::value;

        // Strip const for trait lookup (we always look up the raw C type).
        using KeyT = typename std::remove_const<K>::type;
        using ValT = typename std::remove_const<V>::type;
        
        using MapTraits = traits<KeyT, ValT>;
        using CMap = typename MapTraits::map_type;
        using CBucket = typename MapTraits::bucket_type;

        using iterator_category = std::forward_iterator_tag;
        using value_type = CBucket;
        using difference_type = ptrdiff_t;
        
        // Handle const references/pointers dynamically.
        using reference = typename std::conditional<is_const, const CBucket&, CBucket&>::type;
        using pointer   = typename std::conditional<is_const, const CBucket*, CBucket*>::type;

        map_iterator(CMap* m, size_t idx) : map_ptr(m), index(idx) 
        {
            // Scan to first valid item.
            if (map_ptr && index < map_ptr->capacity) 
            {
                if (map_ptr->buckets[index].state == 0) // 0 is ZMAP_EMPTY
                {
                    advance();
                }
            }
        }

        reference operator*() const { return map_ptr->buckets[index]; }
        pointer operator->() const { return &map_ptr->buckets[index]; }

GeneraCan you        bool operator==(const map_iterator& other) const 
        { 
            return map_ptr == other.map_ptr && index == other.index; 
        }
        bool operator!=(const map_iterator& other) const 
        { 
            return !(*this == other); 
        }

        map_iterator& operator++() 
        {
            if (index < map_ptr->capacity) 
            {
                index++;
                advance();
            }
            return *this;
        }

        map_iterator operator++(int) 
        {
            map_iterator temp = *this;
            ++(*this);
            return temp;
        }

    private:
        void advance() 
        {
            while (index < map_ptr->capacity && map_ptr->buckets[index].state == 0) 
            {
                index++;
            }
        }

        CMap* map_ptr;
        size_t index;
    };

    template <typename K, typename V>
    struct map
    {
        using Traits = traits<K, V>;
        using c_map = typename Traits::map_type;
        using c_bucket = typename Traits::bucket_type;

        using iterator = map_iterator<K, V>;
        using const_iterator = map_iterator<const K, const V>;

        c_map inner;

        using HashFunc = uint32_t (*)(K, uint32_t);
        using CmpFunc = int (*)(K, K);

        // Constructor.
        map(HashFunc h, CmpFunc c, uint32_t seed = 0xCAFEBABE, float load_factor = 0.85f) 
            : inner(Traits::init(h, c, load_factor)) 
        {
            Traits::set_seed(&inner, seed);
        }

        // Move constructor.
        map(map&& other) noexcept : inner(other.inner) 
        {
            other.inner = (c_map){0};
        }

        // Destructor.
        ~map() { Traits::free(&inner); }

        // Move assignment.
        map& operator=(map&& other) noexcept 
        {
            if (this != &other) 
            {
                Traits::free(&inner);
                inner = other.inner;
                other.inner = (c_map){0};
            }
            return *this;
        }

        // Deleted copy (maps are heavy yk bruh).
        map(const map&) = delete;
        map& operator=(const map&) = delete;

        /* Modifiers. */

        void put(const K& key, const V& val) 
        {
            if (Traits::put(&inner, key, val) != 0) 
            {
                // In embedded/no-exception environments, this logic would need adaption...
                throw std::bad_alloc();
            }
        }

        void insert_or_assign(const K& key, const V& val) { put(key, val); }

        V* get(const K& key) { return Traits::get(&inner, key); }
        const V* get(const K& key) const { return Traits::get((c_map*)&inner, key); }

        bool contains(const K& key) const { return Traits::get((c_map*)&inner, key) != NULL; }

        void erase(const K& key) { Traits::remove(&inner, key); }

        void clear() { Traits::clear(&inner); }
        
        void set_seed(uint32_t seed) { Traits::set_seed(&inner, seed); }

        /* Capacity. */
        size_t size() const { return inner.count; }
        bool empty() const { return inner.count == 0; }

        /* Iterators. */
        iterator begin() { return iterator(&inner, 0); }
        iterator end() { return iterator(&inner, inner.capacity); }
        
        const_iterator begin() const { return const_iterator((c_map*)&inner, 0); }
        const_iterator end() const { return const_iterator((c_map*)&inner, inner.capacity); }
        const_iterator cbegin() const { return begin(); }
        const_iterator cend() const { return end(); }
    };
}

extern "C" {
#endif // __cplusplus

/* Configuration and allocators. */

#ifndef Z_MAP_MALLOC
    #define Z_MAP_MALLOC(sz)      Z_MALLOC(sz)
#endif

#ifndef Z_MAP_CALLOC
    #define Z_MAP_CALLOC(n, sz)   Z_CALLOC(n, sz)
#endif

#ifndef Z_MAP_REALLOC
    #define Z_MAP_REALLOC(p, sz)  Z_REALLOC(p, sz)
#endif

#ifndef Z_MAP_FREE
    #define Z_MAP_FREE(p)         Z_FREE(p)
#endif

/* Load factor configuration. */
#define ZMAP_DEFAULT_LOAD 0.85f

/* Hashing strategy and helpers */

#if defined(__has_include) && __has_include("zhash.h")
    #include "zhash.h"
    #define ZMAP_HAS_ZHASH 1
#elif defined(ZHASH_H)
    #define ZMAP_HAS_ZHASH 1
#else
    #define ZMAP_HAS_ZHASH 0
#endif

#ifndef ZMAP_HASH_FUNC
    #if ZMAP_HAS_ZHASH
        /* Use WyHash. */
        #define ZMAP_HASH_FUNC(key, len, seed) zhash_fast(key, len, seed)
    #else
        /* FNV-1a. */
        static inline uint32_t zmap_default_hash(const void *key, size_t len, uint32_t seed) 
        {
            uint32_t hash = 2166136261u ^ seed;
            const uint8_t *data = (const uint8_t *)key;
            for (size_t i = 0; i < len; i++) {
                hash ^= data[i];
                hash *= 16777619;
            }
            return hash;
        }
        #define ZMAP_HASH_FUNC(key, len, seed) zmap_default_hash(key, len, seed)
    #endif
#endif

#define ZMAP_HASH_SCALAR(k, s) ZMAP_HASH_FUNC(&(k), sizeof(k), s)
#define ZMAP_HASH_STR(k, s)    ZMAP_HASH_FUNC((k), strlen(k), s)

static inline size_t zmap_next_pow2(size_t n) 
{
    if (n == 0) return 16;
    n--;
    n |= n >> 1; n |= n >> 2; n |= n >> 4; n |= n >> 8; n |= n >> 16;
    #if UINTPTR_MAX > 0xFFFFFFFF
    n |= n >> 32;
    #endif
    return n + 1;
}

/* Fibonacci Hashing Constants & Helpers. */
#define ZMAP_FIB_CONST 0x9E3779B9U

static inline size_t zmap_fib_index(uint32_t hash, uint32_t bits)
{
    return (size_t)((hash * ZMAP_FIB_CONST) >> (32 - bits));
}

static inline size_t zmap_dist(size_t index, size_t capacity, uint32_t hash, uint32_t bits)
{
    size_t home = zmap_fib_index(hash, bits);
    if (index >= home) return index - home;
    return (index + capacity) - home;
}

/* Bucket States.
 * ZMAP_DELETED is removed. We use backward shift deletion.
 */
typedef enum 
{
    ZMAP_EMPTY = 0,
    ZMAP_OCCUPIED
} zmap_state;

/* STANDARD MAP GENERATOR.
 * Stores Key and Value inline.
 */
#define Z_MAP_GENERATE_IMPL(KeyT, ValT, Name)                                                                               \
                                                                                                                            \
    /* Individual bucket structure. */                                                                                      \
    typedef struct                                                                                                          \
    {                                                                                                                       \
        KeyT key;                                                                                                           \
        ValT value;                                                                                                         \
        uint32_t stored_hash;                                                                                               \
        zmap_state state;                                                                                                   \
    } zmap_bucket_##Name;                                                                                                   \
                                                                                                                            \
    /* The main map container. */                                                                                           \
    typedef struct                                                                                                          \
    {                                                                                                                       \
        zmap_bucket_##Name *buckets;                                                                                        \
        size_t capacity;                                                                                                    \
        size_t count;                                                                                                       \
        size_t threshold;                                                                                                   \
        uint32_t bits;                                                                                                      \
        float  load_factor;                                                                                                 \
        uint32_t seed;                                                                                                      \
        uint32_t (*hash_func)(KeyT, uint32_t);                                                                              \
        int      (*cmp_func)(KeyT, KeyT);                                                                                   \
    } map_##Name;                                                                                                           \
                                                                                                                            \
    /* Iterator state. */                                                                                                   \
    typedef struct                                                                                                          \
    {                                                                                                                       \
        map_##Name *map;                                                                                                    \
        size_t index;                                                                                                       \
    } zmap_iter_##Name;                                                                                                     \
                                                                                                                            \
    static inline map_##Name map_init_ext_##Name(uint32_t (*h)(KeyT, uint32_t), int (*c)(KeyT, KeyT), float load)           \
    {                                                                                                                       \
        return (map_##Name){                                                                                                \
            .bits = 0, .load_factor = (load <= 0.1f || load > 0.95f) ? ZMAP_DEFAULT_LOAD : load,                            \
            .seed = 0xCAFEBABE, .hash_func = h, .cmp_func = c                                                               \
        };                                                                                                                  \
    }                                                                                                                       \
                                                                                                                            \
    static inline map_##Name map_init_##Name(uint32_t (*h)(KeyT, uint32_t), int (*c)(KeyT, KeyT))                           \
    {                                                                                                                       \
        return map_init_ext_##Name(h, c, ZMAP_DEFAULT_LOAD);                                                                \
    }                                                                                                                       \
                                                                                                                            \
    static inline void map_set_seed_##Name(map_##Name *m, uint32_t s) { m->seed = s; }                                      \
                                                                                                                            \
    static inline void map_free_##Name(map_##Name *m)                                                                       \
    {                                                                                                                       \
        Z_MAP_FREE(m->buckets);                                                                                             \
        *m = (map_##Name){0};                                                                                               \
    }                                                                                                                       \
                                                                                                                            \
    /* Resizes the bucket array and rehashes all items. */                                                                  \
    static inline int map_resize_##Name(map_##Name *m, size_t new_cap) {                                                    \
        zmap_bucket_##Name *new_buckets = (zmap_bucket_##Name*)Z_MAP_CALLOC(new_cap, sizeof(zmap_bucket_##Name));           \
        if (!new_buckets) return Z_ERR;                                                                                     \
        uint32_t new_bits = 0; size_t temp = new_cap; while(temp >>= 1) new_bits++;                                         \
                                                                                                                            \
        for (size_t i = 0; i < m->capacity; i++)                                                                            \
        {                                                                                                                   \
            if (m->buckets[i].state == ZMAP_OCCUPIED)                                                                       \
            {                                                                                                               \
                zmap_bucket_##Name entry = m->buckets[i];                                                                   \
                size_t idx = zmap_fib_index(entry.stored_hash, new_bits);                                                   \
                size_t dist = 0;                                                                                            \
                for (;;)                                                                                                    \
                {                                                                                                           \
                    if (new_buckets[idx].state == ZMAP_EMPTY)                                                               \
                    {                                                                                                       \
                        new_buckets[idx] = entry; new_buckets[idx].state = ZMAP_OCCUPIED; break;                            \
                    }                                                                                                       \
                    size_t existing_dist = zmap_dist(idx, new_cap, new_buckets[idx].stored_hash, new_bits);                 \
                    if (dist > existing_dist)                                                                               \
                    {                                                                                                       \
                        zmap_bucket_##Name swap_tmp = new_buckets[idx];                                                     \
                        new_buckets[idx] = entry; entry = swap_tmp;                                                         \
                        dist = existing_dist;                                                                               \
                    }                                                                                                       \
                    idx = (idx + 1) & (new_cap - 1); dist++;                                                                \
                }                                                                                                           \
            }                                                                                                               \
        }                                                                                                                   \
        Z_MAP_FREE(m->buckets);                                                                                             \
        m->buckets = new_buckets; m->capacity = new_cap; m->bits = new_bits;                                                \
        m->threshold = (size_t)(new_cap * m->load_factor);                                                                  \
        return Z_OK;                                                                                                        \
    }                                                                                                                       \
                                                                                                                            \
    /* Inserts or updates a value. Clean Robin Hood (No Tombstones). */                                                     \
    static inline int map_put_##Name(map_##Name *m, KeyT key, ValT val)                                                     \
    {                                                                                                                       \
        if (m->count >= m->threshold)                                                                                       \
        {                                                                                                                   \
            size_t new_cap = zmap_next_pow2(Z_GROWTH_FACTOR(m->capacity));                                                  \
            if (map_resize_##Name(m, new_cap) != Z_OK) return Z_ERR;                                                        \
        }                                                                                                                   \
        uint32_t hash = m->hash_func(key, m->seed);                                                                         \
        size_t idx = zmap_fib_index(hash, m->bits);                                                                         \
        size_t dist = 0;                                                                                                    \
        zmap_bucket_##Name entry = (zmap_bucket_##Name){                                                                    \
            .key = key, .value = val, .stored_hash = hash, .state = ZMAP_OCCUPIED };                                        \
                                                                                                                            \
        for (;;)                                                                                                            \
        {                                                                                                                   \
            if (m->buckets[idx].state == ZMAP_EMPTY)                                                                        \
            {                                                                                                               \
                m->buckets[idx] = entry;                                                                                    \
                m->count++; return Z_OK;                                                                                    \
            }                                                                                                               \
            if (m->buckets[idx].stored_hash == hash && m->cmp_func(m->buckets[idx].key, key) == 0)                          \
            {                                                                                                               \
                m->buckets[idx].value = val;                                                                                \
                return Z_OK;                                                                                                \
            }                                                                                                               \
            /* Robin Hood swap check. */                                                                                    \
            size_t existing_dist = zmap_dist(idx, m->capacity, m->buckets[idx].stored_hash, m->bits);                       \
            if (dist > existing_dist)                                                                                       \
            {                                                                                                               \
                zmap_bucket_##Name swap_tmp = m->buckets[idx];                                                              \
                m->buckets[idx] = entry;                                                                                    \
                entry = swap_tmp;                                                                                           \
                dist = existing_dist;                                                                                       \
            }                                                                                                               \
            idx = (idx + 1) & (m->capacity - 1);                                                                            \
            dist++;                                                                                                         \
        }                                                                                                                   \
    }                                                                                                                       \
                                                                                                                            \
    static inline ValT* map_get_##Name(map_##Name *m, KeyT key)                                                             \
    {                                                                                                                       \
        if (m->count == 0) return NULL;                                                                                     \
        uint32_t hash = m->hash_func(key, m->seed);                                                                         \
        size_t idx = zmap_fib_index(hash, m->bits);                                                                         \
        size_t dist = 0;                                                                                                    \
        for (;;)                                                                                                            \
        {                                                                                                                   \
            if (m->buckets[idx].state == ZMAP_EMPTY) return NULL;                                                           \
            size_t existing_dist = zmap_dist(idx, m->capacity, m->buckets[idx].stored_hash, m->bits);                       \
            if (dist > existing_dist) return NULL; /* Early Exit */                                                         \
            if (m->buckets[idx].stored_hash == hash && m->cmp_func(m->buckets[idx].key, key) == 0)                          \
                return &m->buckets[idx].value;                                                                              \
            idx = (idx + 1) & (m->capacity - 1);                                                                            \
            dist++;                                                                                                         \
        }                                                                                                                   \
    }                                                                                                                       \
                                                                                                                            \
    /* Removes an item using Backward Shift Deletion. */                                                                      \
    static inline void map_remove_##Name(map_##Name *m, KeyT key)                                                           \
    {                                                                                                                       \
        if (m->count == 0) return;                                                                                          \
        uint32_t hash = m->hash_func(key, m->seed);                                                                         \
        size_t idx = zmap_fib_index(hash, m->bits);                                                                         \
        size_t dist = 0;                                                                                                    \
        for (;;)                                                                                                            \
        {                                                                                                                   \
            if (m->buckets[idx].state == ZMAP_EMPTY) return;                                                                \
            size_t existing_dist = zmap_dist(idx, m->capacity, m->buckets[idx].stored_hash, m->bits);                       \
            if (dist > existing_dist) return;                                                                               \
            if (m->buckets[idx].stored_hash == hash && m->cmp_func(m->buckets[idx].key, key) == 0)                          \
            {                                                                                                               \
                /* Item found. Perform Backward Shift. */                                                                   \
                m->count--;                                                                                                 \
                for (;;)                                                                                                    \
                {                                                                                                           \
                    size_t next = (idx + 1) & (m->capacity - 1);                                                            \
                    if (m->buckets[next].state == ZMAP_EMPTY)                                                               \
                    {                                                                                                       \
                        m->buckets[idx].state = ZMAP_EMPTY;                                                                 \
                        return;                                                                                             \
                    }                                                                                                       \
                    size_t next_dist = zmap_dist(next, m->capacity, m->buckets[next].stored_hash, m->bits);                 \
                    if (next_dist == 0)                                                                                     \
                    {                                                                                                       \
                        m->buckets[idx].state = ZMAP_EMPTY;                                                                 \
                        return;                                                                                             \
                    }                                                                                                       \
                    m->buckets[idx] = m->buckets[next];                                                                     \
                    idx = next;                                                                                             \
                }                                                                                                           \
            }                                                                                                               \
            idx = (idx + 1) & (m->capacity - 1);                                                                            \
            dist++;                                                                                                         \
        }                                                                                                                   \
    }                                                                                                                       \
                                                                                                                            \
    static inline zmap_iter_##Name map_iter_init_##Name(map_##Name *m)                                                      \
    {                                                                                                                       \
        return (zmap_iter_##Name){ .map = m, .index = 0 };                                                                  \
    }                                                                                                                       \
                                                                                                                            \
    static inline bool map_iter_next_##Name(zmap_iter_##Name *it, KeyT *out_k, ValT *out_v)                                 \
    {                                                                                                                       \
        if (!it->map || !it->map->buckets) return false;                                                                    \
        while (it->index < it->map->capacity)                                                                               \
        {                                                                                                                   \
            size_t i = it->index++;                                                                                         \
            if (it->map->buckets[i].state == ZMAP_OCCUPIED)                                                                 \
            {                                                                                                               \
                if (out_k) *out_k = it->map->buckets[i].key;                                                                \
                if (out_v) *out_v = it->map->buckets[i].value;                                                              \
                return true;                                                                                                \
            }                                                                                                               \
        }                                                                                                                   \
        return false;                                                                                                       \
    }                                                                                                                       \
                                                                                                                            \
    static inline size_t map_size_##Name(map_##Name *m) { return m->count; }                                                \
                                                                                                                            \
    static inline void map_clear_##Name(map_##Name *m)                                                                      \
    {                                                                                                                       \
        if (m->capacity > 0) memset(m->buckets, 0, m->capacity * sizeof(zmap_bucket_##Name));                               \
        m->count = 0;                                                                                                       \
    }

/* STABLE MAP GENERATOR (pointer optimized / uthash-like)
 * Stores a POINTER to the value (ValT*). The bucket owns the memory.
 */
#define Z_MAP_GENERATE_STABLE_IMPL(KeyT, ValT, Name)                                                                                    \
                                                                                                                                        \
    typedef struct                                                                                                                      \
    {                                                                                                                                   \
        KeyT key;                                                                                                                       \
        ValT *value;                                                                                                                    \
        uint32_t stored_hash;                                                                                                           \
        zmap_state state;                                                                                                               \
    } zmap_bucket_stable_##Name;                                                                                                        \
                                                                                                                                        \
    typedef struct                                                                                                                      \
    {                                                                                                                                   \
        zmap_bucket_stable_##Name *buckets;                                                                                             \
        size_t capacity; size_t count; size_t threshold;                                                                                \
        uint32_t bits;                                                                                                                  \
        float  load_factor; uint32_t seed;                                                                                              \
        uint32_t (*hash_func)(KeyT, uint32_t);                                                                                          \
        int      (*cmp_func)(KeyT, KeyT);                                                                                               \
    } map_stable_##Name;                                                                                                                \
                                                                                                                                        \
    typedef struct                                                                                                                      \
    {                                                                                                                                   \
        map_stable_##Name *map;                                                                                                         \
        size_t index;                                                                                                                   \
    } zmap_iter_stable_##Name;                                                                                                          \
                                                                                                                                        \
    static inline map_stable_##Name map_init_ext_stable_##Name(uint32_t (*h)(KeyT, uint32_t), int (*c)(KeyT, KeyT), float load)         \
    {                                                                                                                                   \
        return (map_stable_##Name){ .bits = 0, .load_factor = (load <= 0.1f || load > 0.95f) ? ZMAP_DEFAULT_LOAD : load,                \
                                    .seed = 0xCAFEBABE, .hash_func = h, .cmp_func = c };                                                \
    }                                                                                                                                   \
                                                                                                                                        \
    static inline map_stable_##Name map_init_stable_##Name(uint32_t (*h)(KeyT, uint32_t), int (*c)(KeyT, KeyT))                         \
    {                                                                                                                                   \
        return map_init_ext_stable_##Name(h, c, ZMAP_DEFAULT_LOAD);                                                                     \
    }                                                                                                                                   \
                                                                                                                                        \
    static inline void map_set_seed_stable_##Name(map_stable_##Name *m, uint32_t s) { m->seed = s; }                                    \
                                                                                                                                        \
    static inline void map_free_stable_##Name(map_stable_##Name *m)                                                                     \
    {                                                                                                                                   \
        if (m->buckets)                                                                                                                 \
        {                                                                                                                               \
            for (size_t i = 0; i < m->capacity; i++)                                                                                    \
                if (m->buckets[i].state == ZMAP_OCCUPIED) Z_MAP_FREE(m->buckets[i].value);                                              \
            Z_MAP_FREE(m->buckets);                                                                                                     \
        }                                                                                                                               \
        *m = (map_stable_##Name){0};                                                                                                    \
    }                                                                                                                                   \
                                                                                                                                        \
    static inline int map_resize_stable_##Name(map_stable_##Name *m, size_t new_cap)                                                    \
    {                                                                                                                                   \
        zmap_bucket_stable_##Name *new_buckets = (zmap_bucket_stable_##Name*)Z_MAP_CALLOC(new_cap, sizeof(zmap_bucket_stable_##Name));  \
        if (!new_buckets) return Z_ERR;                                                                                                 \
        uint32_t new_bits = 0; size_t temp = new_cap; while(temp >>= 1) new_bits++;                                                     \
                                                                                                                                        \
        for (size_t i = 0; i < m->capacity; i++)                                                                                        \
        {                                                                                                                               \
            if (m->buckets[i].state == ZMAP_OCCUPIED)                                                                                   \
            {                                                                                                                           \
                zmap_bucket_stable_##Name entry = m->buckets[i];                                                                        \
                size_t idx = zmap_fib_index(entry.stored_hash, new_bits);                                                               \
                size_t dist = 0;                                                                                                        \
                for (;;) {                                                                                                              \
                    if (new_buckets[idx].state == ZMAP_EMPTY) {                                                                         \
                        new_buckets[idx] = entry; new_buckets[idx].state = ZMAP_OCCUPIED; break;                                        \
                    }                                                                                                                   \
                    size_t existing_dist = zmap_dist(idx, new_cap, new_buckets[idx].stored_hash, new_bits);                             \
                    if (dist > existing_dist) {                                                                                         \
                        zmap_bucket_stable_##Name tmp = new_buckets[idx];                                                               \
                        new_buckets[idx] = entry; entry = tmp;                                                                          \
                        dist = existing_dist;                                                                                           \
                    }                                                                                                                   \
                    idx = (idx + 1) & (new_cap - 1); dist++;                                                                            \
                }                                                                                                                       \
            }                                                                                                                           \
        }                                                                                                                               \
        Z_MAP_FREE(m->buckets); m->buckets = new_buckets; m->capacity = new_cap; m->bits = new_bits;                                    \
        m->threshold = (size_t)(new_cap * m->load_factor);                                                                              \
        return Z_OK;                                                                                                                    \
    }                                                                                                                                   \
                                                                                                                                        \
    static inline int map_put_stable_##Name(map_stable_##Name *m, KeyT key, ValT val)                                                   \
    {                                                                                                                                   \
        if (m->count >= m->threshold)                                                                                                   \
        {                                                                                                                               \
            size_t new_cap = zmap_next_pow2(Z_GROWTH_FACTOR(m->capacity));                                                              \
            if (map_resize_stable_##Name(m, new_cap) != Z_OK) return Z_ERR;                                                             \
        }                                                                                                                               \
        uint32_t hash = m->hash_func(key, m->seed);                                                                                     \
        size_t idx = zmap_fib_index(hash, m->bits);                                                                                     \
        size_t dist = 0;                                                                                                                \
        zmap_bucket_stable_##Name entry = (zmap_bucket_stable_##Name){                                                                  \
            .key = key, .value = NULL, .stored_hash = hash, .state = ZMAP_OCCUPIED };                                                   \
                                                                                                                                        \
        for (;;)                                                                                                                        \
        {                                                                                                                               \
            if (m->buckets[idx].state == ZMAP_EMPTY)                                                                                    \
            {                                                                                                                           \
                if (!entry.value)                                                                                                       \
                {                                                                                                                       \
                    entry.value = (ValT*)Z_MAP_MALLOC(sizeof(ValT));                                                                    \
                    if (!entry.value) return Z_ERR;                                                                                     \
                    *entry.value = val;                                                                                                 \
                }                                                                                                                       \
                m->buckets[idx] = entry; m->count++; return Z_OK;                                                                       \
            }                                                                                                                           \
            if (m->buckets[idx].stored_hash == hash && m->cmp_func(m->buckets[idx].key, key) == 0)                                      \
            {                                                                                                                           \
                 *m->buckets[idx].value = val;                                                                                          \
                 if (entry.value) Z_MAP_FREE(entry.value);                                                                              \
                 return Z_OK;                                                                                                           \
            }                                                                                                                           \
            size_t existing_dist = zmap_dist(idx, m->capacity, m->buckets[idx].stored_hash, m->bits);                                   \
            if (dist > existing_dist)                                                                                                   \
            {                                                                                                                           \
                if (!entry.value)                                                                                                       \
                {                                                                                                                       \
                    entry.value = (ValT*)Z_MAP_MALLOC(sizeof(ValT));                                                                    \
                    if (!entry.value) return Z_ERR;                                                                                     \
                    *entry.value = val;                                                                                                 \
                }                                                                                                                       \
                zmap_bucket_stable_##Name temp = m->buckets[idx];                                                                       \
                m->buckets[idx] = entry;                                                                                                \
                entry = temp;                                                                                                           \
                dist = existing_dist;                                                                                                   \
            }                                                                                                                           \
            idx = (idx + 1) & (m->capacity - 1); dist++;                                                                                \
        }                                                                                                                               \
    }                                                                                                                                   \
                                                                                                                                        \
    static inline ValT* map_get_stable_##Name(map_stable_##Name *m, KeyT key)                                                           \
    {                                                                                                                                   \
        if (m->count == 0) return NULL;                                                                                                 \
        uint32_t hash = m->hash_func(key, m->seed);                                                                                     \
        size_t idx = zmap_fib_index(hash, m->bits);                                                                                     \
        size_t dist = 0;                                                                                                                \
        for (;;)                                                                                                                        \
        {                                                                                                                               \
            if (m->buckets[idx].state == ZMAP_EMPTY) return NULL;                                                                       \
            size_t existing_dist = zmap_dist(idx, m->capacity, m->buckets[idx].stored_hash, m->bits);                                   \
            if (dist > existing_dist) return NULL;                                                                                      \
            if (m->buckets[idx].stored_hash == hash && m->cmp_func(m->buckets[idx].key, key) == 0) return m->buckets[idx].value;        \
            idx = (idx + 1) & (m->capacity - 1); dist++;                                                                                \
        }                                                                                                                               \
    }                                                                                                                                   \
                                                                                                                                        \
    static inline void map_remove_stable_##Name(map_stable_##Name *m, KeyT key)                                                         \
    {                                                                                                                                   \
        if (m->count == 0) return;                                                                                                      \
        uint32_t hash = m->hash_func(key, m->seed);                                                                                     \
        size_t idx = zmap_fib_index(hash, m->bits);                                                                                     \
        size_t dist = 0;                                                                                                                \
        for (;;)                                                                                                                        \
        {                                                                                                                               \
            if (m->buckets[idx].state == ZMAP_EMPTY) return;                                                                            \
            size_t existing_dist = zmap_dist(idx, m->capacity, m->buckets[idx].stored_hash, m->bits);                                   \
            if (dist > existing_dist) return;                                                                                           \
            if (m->buckets[idx].stored_hash == hash && m->cmp_func(m->buckets[idx].key, key) == 0)                                      \
            {                                                                                                                           \
                Z_MAP_FREE(m->buckets[idx].value);                                                                                      \
                m->count--;                                                                                                             \
                for (;;)                                                                                                                \
                {                                                                                                                       \
                    size_t next = (idx + 1) & (m->capacity - 1);                                                                        \
                    if (m->buckets[next].state == ZMAP_EMPTY)                                                                           \
                    {                                                                                                                   \
                         m->buckets[idx].state = ZMAP_EMPTY; return;                                                                    \
                    }                                                                                                                   \
                    size_t next_dist = zmap_dist(next, m->capacity, m->buckets[next].stored_hash, m->bits);                             \
                    if (next_dist == 0)                                                                                                 \
                    {                                                                                                                   \
                         m->buckets[idx].state = ZMAP_EMPTY; return;                                                                    \
                    }                                                                                                                   \
                    m->buckets[idx] = m->buckets[next];                                                                                 \
                    idx = next;                                                                                                         \
                }                                                                                                                       \
            }                                                                                                                           \
            idx = (idx + 1) & (m->capacity - 1); dist++;                                                                                \
        }                                                                                                                               \
    }                                                                                                                                   \
                                                                                                                                        \
    static inline size_t map_size_stable_##Name(map_stable_##Name *m) { return m->count; }                                              \
    static inline void map_clear_stable_##Name(map_stable_##Name *m)                                                                    \
    {                                                                                                                                   \
        if (m->capacity > 0)                                                                                                            \
        {                                                                                                                               \
            for (size_t i = 0; i < m->capacity; i++) if (m->buckets[i].state == ZMAP_OCCUPIED) Z_MAP_FREE(m->buckets[i].value);         \
            memset(m->buckets, 0, m->capacity * sizeof(zmap_bucket_stable_##Name));                                                     \
        }                                                                                                                               \
        m->count = 0;                                                                                                                   \
    }                                                                                                                                   \
                                                                                                                                        \
    static inline zmap_iter_stable_##Name map_iter_init_stable_##Name(map_stable_##Name *m) { return (zmap_iter_stable_##Name){m, 0}; } \
                                                                                                                                        \
    static inline bool map_iter_next_stable_##Name(zmap_iter_stable_##Name *it, KeyT *k, ValT *v)                                       \
    {                                                                                                                                   \
        if(!it->map || !it->map->buckets) return false;                                                                                 \
        while(it->index < it->map->capacity)                                                                                            \
        {                                                                                                                               \
            size_t i = it->index++;                                                                                                     \
            if(it->map->buckets[i].state == ZMAP_OCCUPIED)                                                                              \
            {                                                                                                                           \
                if(k) *k = it->map->buckets[i].key;                                                                                     \
                if(v) *v = *it->map->buckets[i].value;                                                                                  \
                return true;                                                                                                            \
            }                                                                                                                           \
        } return false;                                                                                                                 \
    }

/* Registry and dispatch. */

/* Registry Hooks. */
#if defined(__has_include) && __has_include("z_registry.h")
    #include "z_registry.h"
#endif

#ifndef REGISTER_MAP_TYPES
    #define REGISTER_MAP_TYPES(X)
#endif
#ifndef REGISTER_STABLE_MAPS
    #define REGISTER_STABLE_MAPS(X)
#endif
#ifndef Z_AUTOGEN_MAPS
    #define Z_AUTOGEN_MAPS(X)
#endif
#ifndef Z_AUTOGEN_STABLE_MAPS
    #define Z_AUTOGEN_STABLE_MAPS(X)
#endif

// Master Lists.
#define Z_ALL_MAPS(X)        Z_AUTOGEN_MAPS(X)        REGISTER_MAP_TYPES(X)
#define Z_ALL_STABLE_MAPS(X) Z_AUTOGEN_STABLE_MAPS(X) REGISTER_STABLE_MAPS(X)

// Generate defs.
Z_ALL_MAPS(Z_MAP_GENERATE_IMPL)
Z_ALL_STABLE_MAPS(Z_MAP_GENERATE_STABLE_IMPL)

// Dispatch entries (Standard).
#define M_PUT_ENTRY(K, V, N)     map_##N*: map_put_##N,
#define M_GET_ENTRY(K, V, N)     map_##N*: map_get_##N,
#define M_REM_ENTRY(K, V, N)     map_##N*: map_remove_##N,
#define M_FREE_ENTRY(K, V, N)    map_##N*: map_free_##N,
#define M_SIZE_ENTRY(K, V, N)    map_##N*: map_size_##N,
#define M_CLEAR_ENTRY(K, V, N)   map_##N*: map_clear_##N,
#define M_SEED_ENTRY(K, V, N)    map_##N*: map_set_seed_##Name,
#define M_ITER_INIT(K, V, N)     map_##N*: map_iter_init_##Name,
#define M_ITER_NEXT(K, V, N)     zmap_iter_##N*: map_iter_next_##Name,

// Dispatch entries (Stable).
#define S_PUT_ENTRY(K, V, N)     map_stable_##N*: map_put_stable_##N,
#define S_GET_ENTRY(K, V, N)     map_stable_##N*: map_get_stable_##N,
#define S_REM_ENTRY(K, V, N)     map_stable_##N*: map_remove_stable_##Name,
#define S_FREE_ENTRY(K, V, N)    map_stable_##N*: map_free_stable_##Name,
#define S_SIZE_ENTRY(K, V, N)    map_stable_##N*: map_size_stable_##Name,
#define S_CLEAR_ENTRY(K, V, N)   map_stable_##N*: map_clear_stable_##Name,
#define S_SEED_ENTRY(K, V, N)    map_stable_##N*: map_set_seed_stable_##Name,
#define S_ITER_INIT(K, V, N)     map_stable_##N*: map_iter_init_stable_##Name,
#define S_ITER_NEXT(K, V, N)     zmap_iter_stable_##N*: map_iter_next_stable_##Name,

/* API Macros - auto-detects variant via _Generic. */
#define map_init(Name, h, c)        map_init_##Name(h, c)
#define map_init_stable(Name, h, c) map_init_stable_##Name(h, c)

#if defined(Z_HAS_CLEANUP) && Z_HAS_CLEANUP
    #define map_autofree(Name)  Z_CLEANUP(map_free_##Name) map_##Name
#endif

// Generic API Methods.
#define map_put(m, k, v)   _Generic((m), Z_ALL_MAPS(M_PUT_ENTRY)  Z_ALL_STABLE_MAPS(S_PUT_ENTRY)  default: 0)(m, k, v)
#define map_get(m, k)      _Generic((m), Z_ALL_MAPS(M_GET_ENTRY)  Z_ALL_STABLE_MAPS(S_GET_ENTRY)  default: (void*)0)(m, k)
#define map_remove(m, k)   _Generic((m), Z_ALL_MAPS(M_REM_ENTRY)  Z_ALL_STABLE_MAPS(S_REM_ENTRY)  default: (void)0)(m, k)
#define map_free(m)        _Generic((m), Z_ALL_MAPS(M_FREE_ENTRY) Z_ALL_STABLE_MAPS(S_FREE_ENTRY) default: (void)0)(m)
#define map_size(m)        _Generic((m), Z_ALL_MAPS(M_SIZE_ENTRY) Z_ALL_STABLE_MAPS(S_SIZE_ENTRY) default: 0)(m)
#define map_clear(m)       _Generic((m), Z_ALL_MAPS(M_CLEAR_ENTRY)Z_ALL_STABLE_MAPS(S_CLEAR_ENTRY)default: (void)0)(m)
#define map_set_seed(m, s) _Generic((m), Z_ALL_MAPS(M_SEED_ENTRY) Z_ALL_STABLE_MAPS(S_SEED_ENTRY) default: (void)0)(m, s)

// Iterators.
#define map_iter_init(Name, m) _Generic((m),        \
    map_##Name*: map_iter_init_##Name,              \
    map_stable_##Name*: map_iter_init_stable_##Name \
)(m)

#define map_iter_next(it, k, v) _Generic((it),      \
    Z_ALL_MAPS(M_ITER_NEXT)                         \
    Z_ALL_STABLE_MAPS(S_ITER_NEXT)                  \
    default: false)(it, k, v)

#ifdef __cplusplus
} // extern "C"

// C++ Traits Generation.
namespace z_map
{
    // C++ traits for standard maps.
    #define ZMAP_CPP_TRAITS(Key, Val, Name)                                 \
        template<> struct traits<Key, Val>                                  \
        {                                                                   \
            using map_type = map_##Name;                                    \
            using bucket_type = zmap_bucket_##Name;                         \
            static constexpr auto init = map_init_ext_##Name;               \
            static constexpr auto put = map_put_##Name;                     \
            static constexpr auto get = map_get_##Name;                     \
            static constexpr auto remove = map_remove_##Name;               \
            static constexpr auto clear = map_clear_##Name;                 \
            static constexpr auto free = map_free_##Name;                   \
            static constexpr auto set_seed = map_set_seed_##Name;           \
        };

    Z_ALL_MAPS(ZMAP_CPP_TRAITS)
}
#endif // __cplusplus

#endif // ZMAP_H