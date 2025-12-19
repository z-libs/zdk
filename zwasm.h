
/*
 * zwasm.h — Minimal WebAssembly & JS Interop Layer
 * Part of Zen Development Kit (ZDK)
 *
 * Version: 1.0.0
 *
 * Usage:
 * #define ZWASM_IMPLEMENTATION
 * #include "zwasm.h"
 * * License: MIT
 * Author: Zuhaitz
 * Repository: https://github.com/z-libs/zwasm.h
 * Version: 1.0.0
 */

#ifndef ZWASM_H
#define ZWASM_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Detect platform.
#if defined(__wasm__) && !defined(__EMSCRIPTEN__)
#   define Z_IS_BARE_WASM 1
#   define ZWASM_NO_LIBC 1
#elif defined(__EMSCRIPTEN__)
#   define Z_IS_EMSCRIPTEN 1
#else
#   define Z_IS_HOST 1
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Export/import macros.
#if defined(Z_IS_BARE_WASM)
#   define ZWASM_EXPORT __attribute__((visibility("default"))) __attribute__((used))
#   define ZWASM_IMPORT(mod, name) __attribute__((import_module(mod), import_name(name)))
#else
#   define ZWASM_EXPORT
#   define ZWASM_IMPORT(mod, name)
#endif

// Core utils.

void zwasm_log(const char *msg);
void zwasm_printf(const char *fmt, ...); // Supports %d, %f, %s.
double zwasm_time_now(void); 
float  zwasm_random(void);   

// DOM and JS.

void zwasm_eval(const char *js_code);
bool zwasm_dom_set_html(const char *element_id, const char *html);

// Canvas 2D graphics.

// Sets the active color (Hex format "#RRGGBB" or name "red").
void zwasm_fill_style(const char *color);
// Draws a filled rectangle.
void zwasm_fill_rect(float x, float y, float w, float h);
// Clears the entire canvas.
void zwasm_clear_canvas(void);

// Input system.

// Call this from JS when keys are pressed/released.
// Key codes are standard JS key codes (e.g., 37=Left, 38=Up, 39=Right, 40=Down).
ZWASM_EXPORT void zwasm_on_key(int key_code, bool is_down);

// Checks if a key is currently held down.
bool zwasm_key_down(int key_code);

// Memory management.

// Enables using zvec/zstr in bare metal mode.
// Call zwasm_mem_init() at the start of main().
void zwasm_mem_init(void *start, size_t size);
void *zwasm_malloc(size_t size);
void *zwasm_realloc(void *ptr, size_t new_size);
void zwasm_free(void *ptr);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ZWASM_H

#ifdef ZWASM_IMPLEMENTATION

#include <stdarg.h>

// Input state (global).
static bool zwasm__keys[256] = {0};

void zwasm_on_key(int key_code, bool is_down) 
{
    if (key_code >= 0 && key_code < 256) 
    {
        zwasm__keys[key_code] = is_down;
    }
}

bool zwasm_key_down(int key_code) 
{
    if (key_code >= 0 && key_code < 256) 
    {
        return zwasm__keys[key_code];
    }
    return false;
}

#if defined(Z_IS_BARE_WASM)

    // Minimal libc.

    size_t strlen(const char *s) 
    { 
        const char *p = s; 
        while (*p) 
        {
            p++; 
        }
        return p - s; 
    }
    void *memset(void *d, int v, size_t n) 
    { 
        unsigned char *p = (unsigned char *)d; 
        while(n--) 
        {
            *p++=v; 
        }
        return d; 
    }

    void *memcpy(void *d, const void *s, size_t n) 
    {
        char *dp = (char *)d;
        const char *sp = (const char *)s; 
        while(n--) 
        {
            *dp++=*sp++; 
        }
        return d;
    }

    // Helpers.
    static char* zwasm__itoa(int i, char *p) 
    {
        if (i < 0) 
        { 
            *p++ = '-'; 
            i = -i; 
        }
        char t[16]; int k = 0;
        if (0 == i) 
        {
            t[k++] = '0';
        }
        while (i) 
        { 
            t[k++] = (i % 10) + '0'; 
            i /= 10; 
        }
        while (k--) 
        {
            *p++ = t[k];
        }
        return p;
    }

    // Imports.
    ZWASM_IMPORT("env", "js_log")   void js_log(const char *s, int l);
    ZWASM_IMPORT("env", "js_time")  double js_time(void);
    ZWASM_IMPORT("env", "js_rand")  float js_rand(void);
    ZWASM_IMPORT("env", "js_eval")  void js_eval(const char *s, int l);
    
    // Canvas imports.
    ZWASM_IMPORT("env", "js_canvas_rect")  void js_canvas_rect(float x, float y, float w, float h);
    ZWASM_IMPORT("env", "js_canvas_style") void js_canvas_style(const char *c, int l);
    ZWASM_IMPORT("env", "js_canvas_clear") void js_canvas_clear(void);

    // Wrappers.
    void zwasm_log(const char *msg)   
    { 
        js_log(msg, strlen(msg)); 
    }

    double zwasm_time_now(void)
    { 
        return js_time(); 
    }

    float  zwasm_random(void)
    { 
        return js_rand(); 
    }

    void   zwasm_eval(const char *js) 
    { 
        js_eval(js, strlen(js)); 
    }

    void zwasm_fill_rect(float x, float y, float w, float h) 
    { 
        js_canvas_rect(x, y, w, h); 
    }

    void zwasm_fill_style(const char *c) 
    { 
        js_canvas_style(c, strlen(c)); 
    }

    void zwasm_clear_canvas(void) 
    { 
        js_canvas_clear(); 
    }

    // Allocator (bump pointer).
    // Note: In a real app, use the symbol &__heap_base provided by linker
    extern unsigned char __heap_base; 
    static unsigned char *zwasm__heap_ptr = &__heap_base;

    void zwasm_mem_init(void *s, size_t sz) 
    { 
        (void)s; 
        (void)sz; // Auto-init via linker symbol.
    }
    
    void *zwasm_malloc(size_t size) 
    {
        void *ptr = zwasm__heap_ptr;
        zwasm__heap_ptr += size; // Should align to 8/16 bytes in real usage.
        return ptr;
    }
    void zwasm_free(void *ptr) 
    { 
        // No-op in bump allocator.
    }

    void *zwasm_realloc(void *ptr, size_t new_size) 
    {
        // Very dumb realloc: just alloc new and copy. 
        // Real implementation requires size tracking.
        void *new_ptr = zwasm_malloc(new_size);
        memcpy(new_ptr, ptr, new_size); // Potentially unsafe read if shrinking.
        return new_ptr;
    }

    // Printf and DOM.
    bool zwasm_dom_set_html(const char *id, const char *html) 
    {
        char buf[4096]; 
        char *p = buf; 
        char *end = buf + 4095;
        const char *s1="var e=document.getElementById('", *s2="');if(e)e.innerHTML=`", *s3="`;";
        while (*s1 && p<end) 
        {
            *p++ = *s1++; 
        }
        while (*id && p<end) 
        {
            *p++ = *id++;
        }
        while (*s2 && p<end) 
        {
            *p++ = *s2++; 
        }
        while (*html && p<end) 
        {
            *p++ = *html++;
        }
        while (*s3 && p<end) 
        {
            *p++ = *s3++; 
        }
        *p = 0;
        zwasm_eval(buf); return true;
    }

    void zwasm_printf(const char *fmt, ...) 
    {
        char buf[1024]; 
        char *p = buf; 
        va_list args; 
        va_start(args, fmt);
        while (*fmt && p < buf + 1000) 
        {
            if ('%' == *fmt) 
            {
                fmt++;
                if ('d' == *fmt) 
                {
                    p = zwasm__itoa(va_arg(args,int), p);
                }
                else if ('s' == *fmt) 
                { 
                    char *s=va_arg(args,char*); 
                    while (*s)
                    {
                        *p++ = *s++; 
                    }
                }
                else if('f' == *fmt) 
                { 
                    double f = va_arg(args,double); 
                    int i = (int)f; 
                    p = zwasm__itoa(i,p); 
                    *p++ = '.';
                    int fr = (int)((f-i)*1000); 
                    if (fr < 0)
                    {
                        fr = -fr; 
                    }
                    if (fr < 10)
                    {
                        *p++ = '0'; 
                    }
                    if (fr < 100)
                    {
                        *p++ = '0';
                    }
                    p=zwasm__itoa(fr,p);
                }
            } 
            else 
            {
                *p++ = *fmt;
            }
            fmt++;
        }
        *p = 0; 
        va_end(args); 
        zwasm_log(buf);
    }

#else 

#   include <stdio.h>
#   include <stdlib.h>
#   include <string.h>
#   include <time.h>

    void zwasm_log(const char *msg) 
    {
        printf("[LOG] %s\n", msg);
    }

    void zwasm_printf(const char *fmt, ...) 
    {
        va_list args;
        va_start(args, fmt);
        printf("[PRINTF] ");
        vprintf(fmt, args);
        printf("\n");
        va_end(args);
    }

    double zwasm_time_now(void) 
    {
        return (double)clock() / CLOCKS_PER_SEC;
    }

    float zwasm_random(void) 
    {
        return (float)rand() / (float)RAND_MAX;
    }

    void zwasm_eval(const char *js_code) 
    {
        printf("[JS EVAL] %s\n", js_code);
    }

    bool zwasm_dom_set_html(const char *element_id, const char *html) 
    {
        printf("[DOM] Set #%s HTML to: %s\n", element_id, html);
        return true;
    }

    // Canvas stubs.

    void zwasm_fill_style(const char *color) 
    {
        printf("[CANVAS] Fill Style: %s\n", color);
    }

    void zwasm_fill_rect(float x, float y, float w, float h) 
    {
        printf("[CANVAS] Rect: %.2f, %.2f (%.2fx%.2f)\n", x, y, w, h);
    }

    void zwasm_clear_canvas(void) 
    {
        printf("[CANVAS] Clear\n");
    }

    // Standard memory management.
    void zwasm_mem_init(void *start, size_t size) 
    { 
        (void)start; 
        (void)size; 
        // Not needed on host.
    }

    void *zwasm_malloc(size_t size) 
    { 
        return malloc(size); 
    }

    void *zwasm_realloc(void *ptr, size_t new_size) 
    { 
        return realloc(ptr, new_size); 
    }

    void zwasm_free(void *ptr) 
    { 
        free(ptr); 
    }

#endif // Platform

#endif // ZWASM_IMPLEMENTATION