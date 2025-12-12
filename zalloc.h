
#ifndef ZALLOC_H
#define ZALLOC_H

#include <stddef.h>
#include <stdint.h>

/*
 * So, this one was originally three different header files:
 * * zarena.h 
 * * zpool.h 
 * * zdebug.h 
 * After building these I thought "why can't we just unite them?
 * So this is why zalloc.h exists. Still, the file is divided in 
 * three blocks (well, three pairs of blocks), you can easily see 
 * what corresponds to what.
*/

/* * feature flags.
 * If no specific feature is requested, enable ALL of them.
 * To save binary size, define ZALLOC_ARENA_ONLY, ZALLOC_POOL_ONLY, etc.
 */
#if !defined(ZALLOC_ARENA_ONLY) && !defined(ZALLOC_POOL_ONLY) && !defined(ZALLOC_DEBUG_ONLY)
    #define ZALLOC_ENABLE_ARENA
    #define ZALLOC_ENABLE_POOL
    #define ZALLOC_ENABLE_DEBUG
#else
    #ifdef ZALLOC_ARENA_ONLY
        #define ZALLOC_ENABLE_ARENA
    #endif
    #ifdef ZALLOC_POOL_ONLY
        #define ZALLOC_ENABLE_POOL
    #endif
    #ifdef ZALLOC_DEBUG_ONLY
        #define ZALLOC_ENABLE_DEBUG
    #endif
#endif

#ifndef ZALLOC_API
    #ifdef __cplusplus
        #define ZALLOC_API extern "C"
    #else
        #define ZALLOC_API extern
    #endif
#endif

/* * Customize memory backend (defaults to stdlib).
 * We could have included zcommon.h but, well. 
*/
#ifndef Z_MALLOC
    #include <stdlib.h>
    #define Z_MALLOC(sz)    malloc(sz)
    #define Z_REALLOC(p,sz) realloc(p, sz)
    #define Z_FREE(p)       free(p)
#endif


/* * Z-ARENA (linear/region allocator)
 * Best for: temporary memory, per-frame data, fast bulk cleanups.
*/
#ifdef ZALLOC_ENABLE_ARENA

#ifndef ZARENA_MAX_ALIGN
    #define ZARENA_MAX_ALIGN 16 
#endif

#ifndef ZARENA_DEFAULT_BLOCK_SIZE
    #define ZARENA_DEFAULT_BLOCK_SIZE 4096
#endif

typedef struct zarena_block zarena_block;

typedef struct zarena 
{
    zarena_block *head;
    zarena_block *first;
    size_t total_alloc;
} zarena;

ZALLOC_API void  zarena_init(zarena *a);
ZALLOC_API void  zarena_free(zarena *a);
ZALLOC_API void  zarena_reset(zarena *a);
ZALLOC_API void* zarena_alloc(zarena *a, size_t size);
ZALLOC_API void* zarena_alloc_align(zarena *a, size_t size, size_t align);
ZALLOC_API void* zarena_alloc_zero(zarena *a, size_t size);
ZALLOC_API void* zarena_realloc(zarena *a, void *old_ptr, size_t old_size, size_t new_size);

#define zarena_alloc_macro(ctx, sz)    zarena_alloc((zarena*)ctx, sz)
#define zarena_calloc_macro(ctx, n, s) zarena_alloc_zero((zarena*)ctx, (n)*(s))
#define zarena_free_macro(ctx, p)      /* no-op */

/* "Tiny Polish": RAII Helper for GCC/Clang users */
#if defined(__GNUC__) || defined(__clang__)
    ZALLOC_API void zarena_free_ptr(zarena **a); /* Helper for cleanup */
    #define ZARENA_AUTO(name) \
        zarena name __attribute__((cleanup(zarena_free_ptr))); \
        zarena_init(&name)
#endif

#endif /* ZALLOC_ENABLE_ARENA */


/* * Z-POOL (fixed-block allocator)
 * Best for: stable maps, linked lists, graphs, millions of small objects.
 * AUTOMATICALLY ALIGNED to sizeof(void*) to prevent bus errors.
*/
#ifdef ZALLOC_ENABLE_POOL

typedef struct zpool zpool;

ZALLOC_API void  zpool_init(zpool *p, size_t item_size, size_t items_per_block);
ZALLOC_API void  zpool_free(zpool *p);
ZALLOC_API void* zpool_alloc(zpool *p);
ZALLOC_API void  zpool_recycle(zpool *p, void *ptr);

typedef struct zpool_node 
{
    struct zpool_node *next;
} zpool_node;

struct zpool 
{
    size_t item_size;      /* Aligned size */
    size_t count_per_block;
    zpool_node *head;      /* Free list head */
    void **blocks;         /* Array of allocated pages */
    size_t block_count;
    size_t block_cap;
};

#endif /* ZALLOC_ENABLE_POOL */


/*
 * Z-DEBUG (leak detector & buffer overflow guard)
 * Best for: debugging and testing. Wraps malloc to track leaks.
*/
#ifdef ZALLOC_ENABLE_DEBUG

/* Thread safety hooks. */
#ifndef ZDEBUG_LOCK
    #define ZDEBUG_LOCK()
    #define ZDEBUG_UNLOCK()
#endif

/* Logging output customization. */
#ifndef ZDEBUG_LOG
    #include <stdio.h>
    #define ZDEBUG_LOG(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)
#endif

ZALLOC_API void* zdebug_malloc_loc(size_t size, const char *file, int line);
ZALLOC_API void* zdebug_calloc_loc(size_t count, size_t size, const char *file, int line);
ZALLOC_API void* zdebug_realloc_loc(void *ptr, size_t size, const char *file, int line);
ZALLOC_API void  zdebug_free(void *ptr);
ZALLOC_API size_t zdebug_print_leaks(void);
/* Helper to register leak report on exit ("Tiny Polish") */
ZALLOC_API void   zdebug_register_atexit(void);

#define zdebug_malloc(sz)     zdebug_malloc_loc(sz, __FILE__, __LINE__)
#define zdebug_calloc(n, sz)  zdebug_calloc_loc(n, sz, __FILE__, __LINE__)
#define zdebug_realloc(p, sz) zdebug_realloc_loc(p, sz, __FILE__, __LINE__)

#endif /* ZALLOC_ENABLE_DEBUG */

#endif // ZALLOC_H

/* Implementation. */

#ifdef ZALLOC_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

/* ZARENA. */
#ifdef ZALLOC_ENABLE_ARENA

struct zarena_block 
{
    struct zarena_block *next;
    size_t capacity;
    size_t used;
    uint8_t data[];
};

static inline uintptr_t _zarena_align_ptr(uintptr_t ptr, size_t align) 
{
    assert((align & (align - 1)) == 0);
    return (ptr + (align - 1)) & ~(align - 1);
}

static zarena_block* _zarena_new_block(size_t cap) 
{
    size_t total_sz = sizeof(zarena_block) + cap;
    zarena_block *b = (zarena_block*)Z_MALLOC(total_sz);
    if (!b) return NULL;
    b->next = NULL;
    b->capacity = cap;
    b->used = 0;
    return b;
}

ZALLOC_API void zarena_init(zarena *a) 
{
    memset(a, 0, sizeof(zarena));
}

ZALLOC_API void zarena_free(zarena *a) 
{
    zarena_block *curr = a->first;
    while (curr) 
    {
        zarena_block *next = curr->next;
        Z_FREE(curr);
        curr = next;
    }
    memset(a, 0, sizeof(zarena));
}

/* Helper for the ZARENA_AUTO macro */
#if defined(__GNUC__) || defined(__clang__)
ZALLOC_API void zarena_free_ptr(zarena **a) {
    zarena_free(*a);
}
#endif

ZALLOC_API void zarena_reset(zarena *a) 
{
    zarena_block *curr = a->first;
    while (curr) 
    {
        curr->used = 0;
        curr = curr->next;
    }
    a->head = a->first;
    a->total_alloc = 0;
}

ZALLOC_API void* zarena_alloc_align(zarena *a, size_t size, size_t align) 
{
    if (size == 0) return NULL;

    if (a->head) 
    {
        uintptr_t base = (uintptr_t)a->head->data;
        uintptr_t curr = base + a->head->used;
        uintptr_t next = _zarena_align_ptr(curr, align);
        size_t padding = next - curr;
        size_t needed  = size + padding;

        if (a->head->used + needed <= a->head->capacity) 
        {
            a->head->used  += needed;
            a->total_alloc += size;
            return (void*)next;
        }
    }

    if (a->head && a->head->next) 
    {
        zarena_block *next_blk = a->head->next;
        uintptr_t base  = (uintptr_t)next_blk->data;
        uintptr_t start = _zarena_align_ptr(base, align);
        size_t padding  = start - base;
        
        if (size + padding <= next_blk->capacity) 
        {
            a->head         = next_blk;
            a->head->used   = size + padding;
            a->total_alloc += size;
            return (void*)start;
        }
    }

    size_t next_cap = (a->head ? a->head->capacity * 2 : ZARENA_DEFAULT_BLOCK_SIZE);
    if (next_cap < size + align) next_cap = size + align;

    zarena_block *b = _zarena_new_block(next_cap);
    if (!b) return NULL;

    if (a->head) 
    {
        b->next = a->head->next;
        a->head->next = b;
    } 
    else 
    {
        a->first = b;
    }
    a->head = b;

    uintptr_t base  = (uintptr_t)b->data;
    uintptr_t start = _zarena_align_ptr(base, align);
    size_t padding  = start - base;

    b->used = size + padding;
    a->total_alloc += size;
    return (void*)start;
}

ZALLOC_API void* zarena_alloc(zarena *a, size_t size) 
{
    return zarena_alloc_align(a, size, ZARENA_MAX_ALIGN);
}

ZALLOC_API void* zarena_alloc_zero(zarena *a, size_t size) 
{
    void *p = zarena_alloc_align(a, size, ZARENA_MAX_ALIGN);
    if (p) memset(p, 0, size);
    return p;
}

ZALLOC_API void* zarena_realloc(zarena *a, void *old_ptr, size_t old_size, size_t new_size) 
{
    if (!old_ptr) return zarena_alloc(a, new_size);
    if (new_size == 0) return NULL;
    
    if (new_size <= old_size) return old_ptr;

    if (a->head) 
    {
        uintptr_t old_p = (uintptr_t)old_ptr;
        uintptr_t data_end = (uintptr_t)a->head->data + a->head->used;
   
        if (old_p + old_size == data_end) 
        {
            size_t diff = new_size - old_size;
            if (a->head->used + diff <= a->head->capacity) 
            {
                a->head->used  += diff;
                a->total_alloc += diff;
                return old_ptr;
            }
        }
    }

    void *new_ptr = zarena_alloc(a, new_size);
    if (new_ptr) memcpy(new_ptr, old_ptr, old_size);
    return new_ptr;
}
#endif // ZALLOC_ENABLE_ARENA


/* ZPOOL. */
#ifdef ZALLOC_ENABLE_POOL

ZALLOC_API void zpool_init(zpool *p, size_t item_size, size_t items_per_block) 
{
    memset(p, 0, sizeof(zpool));
    
    size_t align = sizeof(void*);
    if (item_size < sizeof(zpool_node*)) item_size = sizeof(zpool_node*);
    
    p->item_size = (item_size + (align - 1)) & ~(align - 1);
    p->count_per_block = (items_per_block < 1) ? 64 : items_per_block;
}

ZALLOC_API void zpool_free(zpool *p) 
{
    for (size_t i = 0; i < p->block_count; i++) 
    {
        Z_FREE(p->blocks[i]);
    }
    if (p->blocks) Z_FREE(p->blocks);
    memset(p, 0, sizeof(zpool));
}

static void _zpool_grow(zpool *p) 
{
    size_t block_mem_size = p->item_size * p->count_per_block;
    uint8_t *block = (uint8_t*)Z_MALLOC(block_mem_size);
    if (!block) return;

    if (p->block_count == p->block_cap) 
    {
        size_t new_cap = (p->block_cap == 0) ? 8 : p->block_cap * 2;
        void **new_list = (void**)Z_REALLOC(p->blocks, new_cap * sizeof(void*));
        if (!new_list) 
        { 
            Z_FREE(block); 
            return; 
        }
        p->blocks = new_list;
        p->block_cap = new_cap;
    }
    p->blocks[p->block_count++] = block;

    for (size_t i = 0; i < p->count_per_block - 1; i++) 
    {
        zpool_node *node = (zpool_node*)(block + (i * p->item_size));
        node->next = (zpool_node*)(block + ((i + 1) * p->item_size));
    }
    
    zpool_node *last = (zpool_node*)(block + ((p->count_per_block - 1) * p->item_size));
    last->next = p->head;
    p->head = (zpool_node*)block;
}

ZALLOC_API void* zpool_alloc(zpool *p) 
{
    if (!p->head) 
    {
        _zpool_grow(p);
        if (!p->head) return NULL;
    }
    zpool_node *node = p->head;
    p->head = node->next;
    return (void*)node;
}

ZALLOC_API void zpool_recycle(zpool *p, void *ptr) 
{
    if (!ptr) return;
    zpool_node *node = (zpool_node*)ptr;
    node->next = p->head;
    p->head = node;
}
#endif // ZALLOC_ENABLE_POOL


/* ZDEBUG. */
#ifdef ZALLOC_ENABLE_DEBUG

#define ZDEBUG_MAGIC_ALIVE 0x11223344
#define ZDEBUG_MAGIC_FREED 0xDEADDEAD
#define ZDEBUG_CANARY_VAL  0xBB
#define ZDEBUG_CANARY_SIZE 16

typedef struct zdebug_header 
{
    struct zdebug_header *prev;
    struct zdebug_header *next;
    const char *file;
    size_t size;
    int line;
    uint32_t magic; 
} zdebug_header;

static zdebug_header *g_zdebug_head = NULL;

static void _zdebug_add(zdebug_header *h) 
{
    ZDEBUG_LOCK();
    h->next = g_zdebug_head;
    h->prev = NULL;
    if (g_zdebug_head) g_zdebug_head->prev = h;
    g_zdebug_head = h;
    ZDEBUG_UNLOCK();
}

static void _zdebug_remove(zdebug_header *h) 
{
    ZDEBUG_LOCK();
    if (h->prev) h->prev->next = h->next;
    else g_zdebug_head = h->next;
    if (h->next) h->next->prev = h->prev;
    ZDEBUG_UNLOCK();
}

static void _zdebug_set_canary(zdebug_header *h)
{
    uint8_t *user_mem = (uint8_t*)(h + 1);
    uint8_t *footer = user_mem + h->size;
    memset(footer, ZDEBUG_CANARY_VAL, ZDEBUG_CANARY_SIZE);
}

static int _zdebug_check_canary(zdebug_header *h)
{
    uint8_t *user_mem = (uint8_t*)(h + 1);
    uint8_t *footer = user_mem + h->size;
    for (int i = 0; i < ZDEBUG_CANARY_SIZE; i++)
    {
        if (footer[i] != ZDEBUG_CANARY_VAL) return 0;
    }
    return 1;
}

ZALLOC_API void* zdebug_malloc_loc(size_t size, const char *file, int line) 
{
    if (size == 0) return NULL;
    
    size_t total_size = sizeof(zdebug_header) + size + ZDEBUG_CANARY_SIZE;
    
    zdebug_header *h = (zdebug_header*)malloc(total_size);
    if (!h)
    {
        ZDEBUG_LOG("[ZDEBUG] Out of memory at %s:%d\n", file, line);
        return NULL;
    }
    
    h->file = file; 
    h->line = line; 
    h->size = size; 
    h->magic = ZDEBUG_MAGIC_ALIVE;
    
    _zdebug_set_canary(h);
    _zdebug_add(h);
    
    return (void*)(h + 1);
}

ZALLOC_API void* zdebug_calloc_loc(size_t count, size_t size, const char *file, int line) 
{
    void *ptr = zdebug_malloc_loc(count * size, file, line);
    if (ptr) memset(ptr, 0, count * size);
    return ptr;
}

ZALLOC_API void* zdebug_realloc_loc(void *ptr, size_t size, const char *file, int line) 
{
    if (!ptr) return zdebug_malloc_loc(size, file, line);
    if (size == 0) { 
        zdebug_free(ptr); 
        return NULL; 
    }
    
    zdebug_header *h = ((zdebug_header*)ptr) - 1;
    
    if (h->magic != ZDEBUG_MAGIC_ALIVE) 
    {
        ZDEBUG_LOG("[ZDEBUG] Bad realloc pointer (%p) at %s:%d\n", ptr, file, line);
        abort();
    }
    if (!_zdebug_check_canary(h)) 
    {
        ZDEBUG_LOG("[ZDEBUG] CORRUPTION detected during realloc (%p) alloc: %s:%d\n", ptr, h->file, h->line);
        abort();
    }

    _zdebug_remove(h);

    size_t total_size = sizeof(zdebug_header) + size + ZDEBUG_CANARY_SIZE;
    zdebug_header *new_h = (zdebug_header*)realloc(h, total_size);
    
    if (!new_h) 
    {
        _zdebug_add(h); 
        return NULL; 
    }

    new_h->size = size;
    new_h->file = file; 
    new_h->line = line;
    _zdebug_set_canary(new_h);
    _zdebug_add(new_h);
    
    return (void*)(new_h + 1);
}

ZALLOC_API void zdebug_free(void *ptr) 
{
    if (!ptr) return;
    zdebug_header *h = ((zdebug_header*)ptr) - 1;

    if (h->magic != ZDEBUG_MAGIC_ALIVE) 
    {
        if (h->magic == ZDEBUG_MAGIC_FREED) 
        {
             ZDEBUG_LOG("[ZDEBUG] DOUBLE FREE detected! (%p) originally from %s:%d\n", ptr, h->file, h->line);
        } 
        else 
        {
             ZDEBUG_LOG("[ZDEBUG] INVALID FREE detected! (%p) Unknown pointer.\n", ptr);
        }
        abort();
    }

    if (!_zdebug_check_canary(h)) 
    {
        ZDEBUG_LOG("[ZDEBUG] BUFFER OVERFLOW detected! (%p) allocated at %s:%d\n", ptr, h->file, h->line);
        abort();
    }

    h->magic = ZDEBUG_MAGIC_FREED;
    _zdebug_remove(h);
    free(h);
}

/* Wrapper to conform to atexit signature (void->void) */
static void _zdebug_atexit_wrapper(void) 
{
    zdebug_print_leaks();
}

ZALLOC_API void zdebug_register_atexit(void) 
{
    atexit(_zdebug_atexit_wrapper);
}

ZALLOC_API size_t zdebug_print_leaks(void) 
{
    ZDEBUG_LOCK();
    zdebug_header *curr = g_zdebug_head;
    size_t count = 0, bytes = 0;
    
    if (curr) ZDEBUG_LOG("=> ZDEBUG DETECTED LEAKS:\n");
    while (curr) 
    {
        ZDEBUG_LOG("   [Leak] %zu bytes at %p (alloc: %s:%d)\n", curr->size, (void*)(curr+1), curr->file, curr->line);
        bytes += curr->size; count++;
        curr = curr->next;
    }
    if (count > 0) 
    {
        ZDEBUG_LOG("=> Total: %zu bytes in %zu blocks.\n", bytes, count);
    }
    ZDEBUG_UNLOCK();
    return count;
}
#endif // ZALLOC_ENABLE_DEBUG

#endif // ZALLOC_IMPLEMENTATION