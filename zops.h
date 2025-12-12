#ifndef ZOPS_H
#define ZOPS_H

#include <stddef.h>
#include <stdbool.h>

#ifndef Z_ALL_VECS
    #define Z_ALL_VECS(action)
#endif
#ifndef Z_ALL_LISTS
    #define Z_ALL_LISTS(action)
#endif
#ifndef Z_ALL_MAPS
    #define Z_ALL_MAPS(action)
#endif

/* -> Trampolines. */

// String trampolines.
static inline char* zops_at_zstr(zstr *s, size_t i)                     { return zstr_data(s) + i; }
static inline const char* zops_at_zstr_const(const zstr *s, size_t i)   { return zstr_cstr(s) + i; }

static inline char* zops_back_zstr(zstr *s)                     { return zstr_data(s) + zstr_len(s) - 1; }
static inline const char* zops_back_zstr_const(const zstr *s)   { return zstr_cstr(s) + zstr_len(s) - 1; }

static inline char* zops_front_zstr(zstr *s)                    { return zstr_data(s); }
static inline const char* zops_front_zstr_const(const zstr *s)  { return zstr_cstr(s); }

static inline int    zops_push_zstr(zstr *s, char c)            { return zstr_push(s, c); }
static inline void   zops_pop_zstr(zstr *s)                     { zstr_pop_char(s); }

// Iterator trampolines for strings
static inline char* zops_begin_zstr(zstr *s)                    { return zstr_data(s); }
static inline const char* zops_begin_zstr_const(const zstr *s)  { return zstr_cstr(s); }
static inline char* zops_end_zstr(zstr *s)                      { return zstr_data(s) + zstr_len(s); }
static inline const char* zops_end_zstr_const(const zstr *s)    { return zstr_cstr(s) + zstr_len(s); }


// Default handlers.
static inline size_t zops_ret_zero(void *p)     { (void)p; return 0; }
static inline void   zops_noop(void *p)         { (void)p; }
static inline void* zops_ret_null(void *p)      { (void)p; return NULL; }
static inline int    zops_ret_err(void *p, ...) { (void)p; return -1; } // Generic Error

// Vector trampolines.
#define _Z_TRAMP_VEC(T, Name)                                                                               \
    static inline size_t zops_len_vec_##Name(vec_##Name *v)                 { return v->length; }           \
    static inline size_t zops_len_vec_const_##Name(const vec_##Name *v)     { return v->length; }           \
    static inline size_t zops_cap_vec_##Name(vec_##Name *v)                 { return v->capacity; }         \
    static inline size_t zops_cap_vec_const_##Name(const vec_##Name *v)     { return v->capacity; }         \
    static inline bool   zops_empty_vec_##Name(vec_##Name *v)               { return v->length == 0; }      \
    static inline bool   zops_empty_vec_const_##Name(const vec_##Name *v)   { return v->length == 0; }      \
    static inline void   zops_del_vec_##Name(vec_##Name *v)                 { vec_free_##Name(v); }         \
    static inline void   zops_clear_vec_##Name(vec_##Name *v)               { vec_clear_##Name(v); }        \
    static inline T* zops_begin_vec_##Name(vec_##Name *v)                   { return v->data; }             \
    static inline const T* zops_begin_vec_const_##Name(const vec_##Name *v) { return v->data; }             \
    static inline T* zops_end_vec_##Name(vec_##Name *v)                     { return v->data + v->length; } \
    static inline const T* zops_end_vec_const_##Name(const vec_##Name *v)   { return v->data + v->length; }

// List trampolines.
#define _Z_TRAMP_LIST(T, Name)                                                                      \
    static inline T* zops_front_list_##Name(list_##Name *l) {                                       \
        zlist_node_##Name *n = list_head_##Name(l); return n ? &n->value : NULL;                    \
    }                                                                                               \
    static inline const T* zops_front_list_const_##Name(const list_##Name *l) {                     \
        zlist_node_##Name *n = l->head; return n ? &n->value : NULL;                                \
    }                                                                                               \
    static inline T* zops_back_list_##Name(list_##Name *l) {                                        \
        zlist_node_##Name *n = list_tail_##Name(l); return n ? &n->value : NULL;                    \
    }                                                                                               \
    static inline const T* zops_back_list_const_##Name(const list_##Name *l) {                      \
        zlist_node_##Name *n = l->tail; return n ? &n->value : NULL;                                \
    }                                                                                               \
    static inline T* zops_at_list_##Name(list_##Name *l, size_t i) {                                \
        zlist_node_##Name *n = list_at_##Name(l, i); return n ? &n->value : NULL;                   \
    }                                                                                               \
    static inline const T* zops_at_list_const_##Name(const list_##Name *l, size_t i) {              \
        zlist_node_##Name *n = list_at_##Name((list_##Name*)l, i); return n ? &n->value : NULL;     \
    }

// Map trampolines.
#define _Z_TRAMP_MAP(K, V, Name)                                                                            \
    static inline size_t zops_len_map_##Name(map_##Name *m)             { return map_size_##Name(m); }      \
    static inline size_t zops_len_map_const_##Name(const map_##Name *m) { return m->count; }                \
    static inline bool zops_empty_map_##Name(map_##Name *m)             { return m->count == 0; }           \
    static inline bool zops_empty_map_const_##Name(const map_##Name *m) { return m->count == 0; }           \
    static inline void zops_del_map_##Name(map_##Name *m)               { map_free_##Name(m); }             \
    static inline void zops_clear_map_##Name(map_##Name *m)             { map_clear_##Name(m); }            \
    static inline V* zops_at_map_##Name(map_##Name *m, K k)             { return map_get_##Name(m, k); }    \
    static inline const V* zops_at_map_const_##Name(const map_##Name *m, K k) {                             \
            return (const V*)map_get_##Name((map_##Name*)m, k);                                             \
    }

/* -> Generator Execution */
Z_ALL_VECS(_Z_TRAMP_VEC)
Z_ALL_LISTS(_Z_TRAMP_LIST)
Z_ALL_MAPS(_Z_TRAMP_MAP)


/* -> Association lists for _Generic. */

// Properties.
#define _Z_MAP_LEN(T, N)   vec_##N*: zops_len_vec_##N, const vec_##N*: zops_len_vec_const_##N,
#define _Z_MAP_CAP(T, N)   vec_##N*: zops_cap_vec_##N, const vec_##N*: zops_cap_vec_const_##N,
#define _Z_MAP_EMPTY(T, N) vec_##N*: zops_empty_vec_##N, const vec_##N*: zops_empty_vec_const_##N,
#define _Z_MAP_DEL(T, N)   vec_##N*: zops_del_vec_##N,
#define _Z_MAP_CLEAR(T, N) vec_##N*: zops_clear_vec_##N,

// Vector access.
#define _Z_MAP_AT(T, N)    vec_##N*: vec_at_##N, const vec_##N*: vec_at_##N,
#define _Z_MAP_BACK(T, N)  vec_##N*: vec_last_##N, const vec_##N*: vec_last_##N,
#define _Z_MAP_FRONT(T, N) vec_##N*: vec_data_##N, const vec_##N*: vec_data_##N,
#define _Z_MAP_PUSH(T, N)  vec_##N*: vec_push_##N,
#define _Z_MAP_POP(T, N)   vec_##N*: vec_pop_##N,

// Iterator support.
#define _Z_MAP_BEGIN(T, N) vec_##N*: zops_begin_vec_##N, const vec_##N*: zops_begin_vec_const_##N,
#define _Z_MAP_END(T, N)   vec_##N*: zops_end_vec_##N,   const vec_##N*: zops_end_vec_const_##N,

// List support.
#define _Z_LIST_PUSH(T, N)  list_##N*: list_push_back_##N,
#define _Z_LIST_POP(T, N)   list_##N*: list_pop_back_##N,
#define _Z_LIST_FRONT(T, N) list_##N*: zops_front_list_##N, const list_##N*: zops_front_list_const_##N,
#define _Z_LIST_BACK(T, N)  list_##N*: zops_back_list_##N, const list_##N*: zops_back_list_const_##N,
#define _Z_LIST_AT(T, N)    list_##N*: zops_at_list_##N, const list_##N*: zops_at_list_const_##N,

// Map support.
#define _Z_HASH_LEN(K, V, N)   map_##N*: zops_len_map_##N, const map_##N*: zops_len_map_const_##N,
#define _Z_HASH_EMPTY(K, V, N) map_##N*: zops_empty_map_##N, const map_##N*: zops_empty_map_const_##N,
#define _Z_HASH_DEL(K, V, N)   map_##N*: zops_del_map_##N,
#define _Z_HASH_CLEAR(K, V, N) map_##N*: zops_clear_map_##N,
#define _Z_HASH_AT(K, V, N)    map_##N*: zops_at_map_##N, const map_##N*: zops_at_map_const_##N,


/* Our cute API. */

#define z_len(x) _Generic((x),  \
    zstr*:      zstr_len,       \
    const zstr*: zstr_len,      \
    Z_ALL_VECS(_Z_MAP_LEN)      \
    Z_ALL_MAPS(_Z_HASH_LEN)     \
    default:    zops_ret_zero   \
)(x)

#define z_is_empty(x) _Generic((x), \
    zstr*:      zstr_is_empty,      \
    const zstr*: zstr_is_empty,     \
    Z_ALL_VECS(_Z_MAP_EMPTY)        \
    Z_ALL_MAPS(_Z_HASH_EMPTY)       \
    default:    zops_ret_zero       \
)(x)

#define z_cap(x) _Generic((x),  \
    Z_ALL_VECS(_Z_MAP_CAP)      \
    default:    zops_ret_zero   \
)(x)

#define z_del(x) _Generic((x),  \
    zstr*:      zstr_free,      \
    Z_ALL_VECS(_Z_MAP_DEL)      \
    Z_ALL_MAPS(_Z_HASH_DEL)     \
    default:    zops_noop       \
)(x)

#define z_clear(x) _Generic((x),    \
    zstr*:      zstr_clear,         \
    Z_ALL_VECS(_Z_MAP_CLEAR)        \
    Z_ALL_MAPS(_Z_HASH_CLEAR)       \
    default:    zops_noop           \
)(x)

/* Iterators. */

#define z_begin(x) _Generic((x),        \
    zstr*:       zops_begin_zstr,       \
    const zstr*: zops_begin_zstr_const, \
    Z_ALL_VECS(_Z_MAP_BEGIN)            \
    default:     zops_ret_null          \
)(x)

#define z_end(x) _Generic((x),          \
    zstr*:       zops_end_zstr,         \
    const zstr*: zops_end_zstr_const,   \
    Z_ALL_VECS(_Z_MAP_END)              \
    default:     zops_ret_null          \
)(x)


/* Accessors. */

#define z_at(x, i) _Generic((x),        \
    zstr*:      zops_at_zstr,           \
    const zstr*: zops_at_zstr_const,    \
    Z_ALL_VECS(_Z_MAP_AT)               \
    Z_ALL_LISTS(_Z_LIST_AT)             \
    Z_ALL_MAPS(_Z_HASH_AT)              \
    default:    zops_ret_null           \
)(x, i)

#define z_back(x) _Generic((x),     \
    zstr*:      zops_back_zstr,     \
    const zstr*: zops_back_zstr_const,\
    Z_ALL_VECS(_Z_MAP_BACK)             \
    Z_ALL_LISTS(_Z_LIST_BACK)           \
    default:    zops_ret_null           \
)(x)

#define z_front(x) _Generic((x),        \
    zstr*:      zops_front_zstr,        \
    const zstr*: zops_front_zstr_const, \
    Z_ALL_VECS(_Z_MAP_FRONT)            \
    Z_ALL_LISTS(_Z_LIST_FRONT)          \
    default:    zops_ret_null           \
)(x)

/* Modifiers */

#define z_push(x, val) _Generic((x),    \
    zstr*:      zops_push_zstr,         \
    Z_ALL_VECS(_Z_MAP_PUSH)             \
    Z_ALL_LISTS(_Z_LIST_PUSH)           \
    default:    zops_ret_err            \
)(x, val)

#define z_pop(x) _Generic((x),  \
    zstr*:      zops_pop_zstr,  \
    Z_ALL_VECS(_Z_MAP_POP)      \
    Z_ALL_LISTS(_Z_LIST_POP)    \
    default:    zops_noop       \
)(x)

// Detect if we can use type inference.
#if defined(__GNUC__) || defined(__clang__)
#   define Z_AUTO_TYPE __auto_type
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ > 201710L
#   define Z_AUTO_TYPE auto
#endif

#ifdef Z_AUTO_TYPE
#   define z_foreach(iter, container)               \
        for (Z_AUTO_TYPE iter = z_begin(container); \
             iter != z_end(container);              \
             ++iter)
#endif

/* Short aliases. */
#ifdef Z_SHORT_OPS
#   define len      z_len
#   define cap      z_cap
#   define is_empty z_is_empty
#   define del      z_del
#   define clear    z_clear
    
#   define push     z_push
#   define pop      z_pop
#   define at       z_at
#   define back     z_back
#   define front    z_front
    
#   define begin    z_begin
#   define end      z_end
#   ifdef z_foreach
#       define foreach  z_foreach
#   endif
#endif

#endif // ZOPS_H
