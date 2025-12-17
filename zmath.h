
/*
 * zmath.h — Single-header Game Math Library
 * Part of Zen Development Kit (ZDK)
 *
 * Features:
 * • Zero standard library dependencies (No <math.h> needed).
 * • Scalar math (clamp, lerp, smoothstep).
 * • Fast approximate trigonometry (polynomial).
 * • Vector math (vec2, vec3).
 *
 * License: MIT
 * Author: Zuhaitz
 * Repository: https://github.com/z-libs/zmath.h
 * Version: 1.0.0
 */

#ifndef ZMATH_H
#define ZMATH_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Constants.

#define ZMATH_PI        3.14159265358979323846f
#define ZMATH_TAU       6.28318530717958647692f
#define ZMATH_HALF_PI   1.57079632679489661923f
#define ZMATH_E         2.71828182845904523536f
#define ZMATH_LN2       0.69314718056f
#define ZMATH_EPSILON   1.19209290e-7f
#define ZMATH_SQRT2     1.41421356237f

#define ZMATH_INFINITY  (1e30f * 1e30f)
#define ZMATH_NAN       (ZMATH_INFINITY * 0.0f)

// Types.

typedef struct { float x, y; } zvec2;
typedef struct { float x, y, z; } zvec3;

// API declarations.

#ifdef ZMATH_STATIC
#   define ZMATHDEF static
#else
#   define ZMATHDEF extern
#endif

// Logic and bitwise.
ZMATHDEF bool  zmath_is_near(float a, float b, float tol);
ZMATHDEF bool  zmath_isnan(float x);
ZMATHDEF bool  zmath_isinf(float x);

// Interpolation.
ZMATHDEF float zmath_step(float edge, float x);
ZMATHDEF float zmath_smoothstep(float edge0, float edge1, float x);
ZMATHDEF float zmath_smootherstep(float edge0, float edge1, float x);
ZMATHDEF float zmath_lerp(float a, float b, float t);
ZMATHDEF float zmath_inv_lerp(float a, float b, float v);
ZMATHDEF float zmath_remap(float iMin, float iMax, float oMin, float oMax, float v);

// Basic arithmetic.
ZMATHDEF float zmath_abs(float x);
ZMATHDEF float zmath_min(float a, float b);
ZMATHDEF float zmath_max(float a, float b);
ZMATHDEF float zmath_clamp(float x, float min_val, float max_val);
ZMATHDEF float zmath_sign(float x);
ZMATHDEF float zmath_copysign(float x, float y);

// Rounding.
ZMATHDEF float zmath_floor(float x);
ZMATHDEF float zmath_ceil(float x);
ZMATHDEF float zmath_round(float x);
ZMATHDEF float zmath_fract(float x);
ZMATHDEF float zmath_fmod(float x, float y);
ZMATHDEF float zmath_mod(float x, float y);

// Power and roots.
ZMATHDEF float zmath_sqrt(float x);
ZMATHDEF float zmath_invsqrt(float x);
ZMATHDEF float zmath_hypot(float x, float y);
ZMATHDEF float zmath_log(float x);
ZMATHDEF float zmath_log2(float x);
ZMATHDEF float zmath_pow(float x, float y);
ZMATHDEF float zmath_exp(float x);

// Trigonometry.
ZMATHDEF float zmath_sin(float x);
ZMATHDEF float zmath_cos(float x);
ZMATHDEF float zmath_tan(float x);
ZMATHDEF float zmath_acos(float x);
ZMATHDEF float zmath_asin(float x);
ZMATHDEF float zmath_atan(float x);
ZMATHDEF float zmath_atan2(float y, float x);
ZMATHDEF float zmath_deg2rad(float deg);
ZMATHDEF float zmath_rad2deg(float rad);

// Vector math.
ZMATHDEF zvec2 zmath_v2_add(zvec2 a, zvec2 b);
ZMATHDEF zvec2 zmath_v2_sub(zvec2 a, zvec2 b);
ZMATHDEF zvec2 zmath_v2_scale(zvec2 v, float s);
ZMATHDEF float zmath_v2_dot(zvec2 a, zvec2 b);
ZMATHDEF float zmath_v2_len(zvec2 v);
ZMATHDEF zvec2 zmath_v2_norm(zvec2 v);

ZMATHDEF zvec3 zmath_v3_add(zvec3 a, zvec3 b);
ZMATHDEF zvec3 zmath_v3_sub(zvec3 a, zvec3 b);
ZMATHDEF zvec3 zmath_v3_scale(zvec3 v, float s);
ZMATHDEF float zmath_v3_dot(zvec3 a, zvec3 b);
ZMATHDEF zvec3 zmath_v3_cross(zvec3 a, zvec3 b);
ZMATHDEF float zmath_v3_len(zvec3 v);
ZMATHDEF zvec3 zmath_v3_norm(zvec3 v);

// Optional short names.
#ifdef ZMATH_SHORT_NAMES
    // Constants.
#   ifndef PI
#   define PI          ZMATH_PI
#   endif
#   define TAU         ZMATH_TAU
#   define EPSILON     ZMATH_EPSILON
#   define SQRT2       ZMATH_SQRT2

    // Types.
    typedef zvec2       vec2;
    typedef zvec3       vec3;

    // Logic, we undefine standard macros if they exist.
#   ifdef isnan
#   undef isnan
#   endif
#   define isnan       zmath_isnan

#   ifdef isinf
#   undef isinf
#   endif
#   define isinf       zmath_isinf

#   define is_near     zmath_is_near
    
    // Interpolation.
#   define step        zmath_step
#   define smoothstep  zmath_smoothstep
#   define smootherstep zmath_smootherstep
#   define lerp        zmath_lerp
#   define inv_lerp    zmath_inv_lerp
#   define remap       zmath_remap
    
    // Arithmetic.
#   define abs         zmath_abs
#   define min         zmath_min
#   define max         zmath_max
#   define clamp       zmath_clamp
#   define sign        zmath_sign
#   define copysign    zmath_copysign
    
    // Rounding.
#   define floor       zmath_floor
#   define ceil        zmath_ceil
#   define round       zmath_round
#   define fract       zmath_fract
#   define fmod        zmath_fmod
#   define mod         zmath_mod
    
    // Power.
#   define sqrt        zmath_sqrt
#   define invsqrt     zmath_invsqrt
#   define hypot       zmath_hypot
#   define log         zmath_log
#   define log2        zmath_log2
#   define pow         zmath_pow
#   define exp         zmath_exp
    
    // Trig.
#   ifdef sin
#   undef sin
#   endif
#   define sin         zmath_sin

#   ifdef cos
#   undef cos
#   endif
#   define cos         zmath_cos

#   ifdef tan
#   undef tan
#   endif
#   define tan         zmath_tan

#   ifdef asin
#   undef asin
#   endif
#   define asin        zmath_asin

#   ifdef acos
#   undef acos
#   endif
#   define acos        zmath_acos

#   ifdef atan
#   undef atan
#   endif
#   define atan        zmath_atan

#   ifdef atan2
#   undef atan2
#   endif
#   define atan2       zmath_atan2

#   define deg2rad     zmath_deg2rad
#   define rad2deg     zmath_rad2deg

    // Vector 2.
#   define v2_add      zmath_v2_add
#   define v2_sub      zmath_v2_sub
#   define v2_scale    zmath_v2_scale
#   define v2_dot      zmath_v2_dot
#   define v2_len      zmath_v2_len
#   define v2_norm     zmath_v2_norm

    // Vector 3.
#   define v3_add      zmath_v3_add
#   define v3_sub      zmath_v3_sub
#   define v3_scale    zmath_v3_scale
#   define v3_dot      zmath_v3_dot
#   define v3_cross    zmath_v3_cross
#   define v3_len      zmath_v3_len
#   define v3_norm     zmath_v3_norm
#endif

#ifdef __cplusplus
}
#endif

#endif // ZMATH_H

#ifdef ZMATH_IMPLEMENTATION

#ifdef __cplusplus
#   include <cstring>
#else
#   include <string.h>
#endif

static inline uint32_t zmath__float_as_uint(float f) 
{
    uint32_t u;
    memcpy(&u, &f, sizeof(float));
    return u;
}

static inline float zmath__uint_as_float(uint32_t u) 
{
    float f;
    memcpy(&f, &u, sizeof(uint32_t));
    return f;
}

// Logic and comparison.

ZMATHDEF bool zmath_is_near(float a, float b, float tol) 
{
    return zmath_abs(a - b) <= tol;
}

ZMATHDEF bool zmath_isnan(float x) 
{
    uint32_t u = zmath__float_as_uint(x);
    return (u & 0x7F800000) == 0x7F800000 && (u & 0x007FFFFF) != 0;
}

ZMATHDEF bool zmath_isinf(float x) 
{
    uint32_t u = zmath__float_as_uint(x);
    return (u & 0x7FFFFFFF) == 0x7F800000;
}

// Interpolation.

ZMATHDEF float zmath_step(float edge, float x) 
{
    return (x < edge) ? 0.0f : 1.0f;
}

ZMATHDEF float zmath_smoothstep(float edge0, float edge1, float x) 
{
    float t = zmath_clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f); 
    return t * t * (3.0f - 2.0f * t);
}

ZMATHDEF float zmath_smootherstep(float edge0, float edge1, float x) 
{
    float t = zmath_clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

ZMATHDEF float zmath_lerp(float a, float b, float t) 
{
    return (1.0f - t) * a + t * b;
}

ZMATHDEF float zmath_inv_lerp(float a, float b, float v) 
{
    return (v - a) / (b - a);
}

ZMATHDEF float zmath_remap(float iMin, float iMax, float oMin, float oMax, float v) 
{
    float t = zmath_inv_lerp(iMin, iMax, v);
    return zmath_lerp(oMin, oMax, t);
}

// Basic arithmetic.

ZMATHDEF float zmath_abs(float x) 
{

    uint32_t u = zmath__float_as_uint(x);
    u &= 0x7FFFFFFF;
    return zmath__uint_as_float(u);
}

ZMATHDEF float zmath_min(float a, float b) 
{
    return (a < b) ? a : b;
}

ZMATHDEF float zmath_max(float a, float b) 
{
    return (a > b) ? a : b;
}

ZMATHDEF float zmath_clamp(float x, float min_val, float max_val) 
{
    if (x < min_val) 
    {
        return min_val;
    }
    if (x > max_val) 
    {
        return max_val;
    }
    return x;
}

ZMATHDEF float zmath_sign(float x) 
{
    if (x > 0.0f) 
    {
        return 1.0f;
    }
    if (x < 0.0f) 
    {
        return -1.0f;
    }
    return 0.0f;
}

ZMATHDEF float zmath_copysign(float x, float y) 
{
    uint32_t ix = zmath__float_as_uint(x);
    uint32_t iy = zmath__float_as_uint(y);
    ix = (ix & 0x7FFFFFFF) | (iy & 0x80000000);
    return zmath__uint_as_float(ix);
}

// Rounding.

#define ZMATH_NO_FRACT_LIMIT 8388608.0f

ZMATHDEF float zmath_floor(float x) 
{
    if (zmath_abs(x) >= ZMATH_NO_FRACT_LIMIT) 
    {
        return x;
    }
    int i = (int)x;
    if (x < 0.0f && x != (float)i) 
    {
        return (float)(i - 1);
    }
    return (float)i;
}

ZMATHDEF float zmath_ceil(float x) 
{
    if (zmath_abs(x) >= ZMATH_NO_FRACT_LIMIT) 
    {
        return x;
    }
    int i = (int)x;
    if (x > 0.0f && x != (float)i) 
    {
        return (float)(i + 1);
    }
    return (float)i;
}

ZMATHDEF float zmath_round(float x) 
{
    return (x >= 0.0f) ? zmath_floor(x + 0.5f) : zmath_ceil(x - 0.5f);
}

ZMATHDEF float zmath_fract(float x) 
{

    return x - zmath_floor(x); 
}

ZMATHDEF float zmath_fmod(float x, float y) 
{ 
    if (zmath_abs(y) < ZMATH_EPSILON) 
    {
        return 0.0f;
    }
    return x - y * (float)((int)(x / y)); 
}

ZMATHDEF float zmath_mod(float x, float y) 
{ 
    if (zmath_abs(y) < ZMATH_EPSILON)
    {
        return 0.0f;
    }
    return x - y * zmath_floor(x / y); 
}

// Power and roots.

ZMATHDEF float zmath_invsqrt(float x) 
{
    float xhalf = 0.5f * x;
    uint32_t i = zmath__float_as_uint(x);
    i = 0x5f3759df - (i >> 1);
    x = zmath__uint_as_float(i);
    x = x * (1.5f - xhalf * x * x);
    return x;
}

ZMATHDEF float zmath_sqrt(float x) 
{
    if (x <= 0.0f) 
    {
        return 0.0f;
    }
    float guess = x * zmath_invsqrt(x); 
    return 0.5f * (guess + x / guess);
}

ZMATHDEF float zmath_hypot(float x, float y) 
{
    x = zmath_abs(x);
    y = zmath_abs(y);
    float min_val = zmath_min(x, y);
    float max_val = zmath_max(x, y);
    if (max_val == 0.0f) 
    {
        return 0.0f;
    }
    float r = min_val / max_val;
    return max_val * zmath_sqrt(1.0f + r * r);
}

ZMATHDEF float zmath_log(float x) 
{
    if (x <= 0.0f) 
    {
        return -ZMATH_INFINITY;
    }
    uint32_t ix = zmath__float_as_uint(x);
    int exponent = ((int)((ix >> 23) & 0xFF)) - 127;
    ix = (ix & 0x007FFFFF) | 0x3F800000;
    float m = zmath__uint_as_float(ix);
    float z = (m - 1.0f) / (m + 1.0f);
    float z2 = z * z;
    float y = z * (2.0f + z2 * (0.66666666f + z2 * (0.4f + z2 * 0.28571428f)));
    return (float)exponent * ZMATH_LN2 + y;
}

ZMATHDEF float zmath_log2(float x) 
{ 
    return zmath_log(x) * 1.44269504088f; 
}

ZMATHDEF float zmath_exp(float x) 
{
    float px = x * 1.44269504088f; 
    float n = zmath_round(px);
    float r = (px - n) * 0.69314718056f; 
    float r2 = r * r;
    float f = 1.0f + r + (r2 * 0.5f) + (r * r2 * 0.16666666f);
    int int_part = (int)n;
    uint32_t bits = zmath__float_as_uint(f);
    bits += (int_part << 23);
    return zmath__uint_as_float(bits);
}

ZMATHDEF float zmath_pow(float x, float y) 
{
    if (x <= 0.0f) 
    {
        return 0.0f;
    }
    if (y == 0.0f) 
    {
        return 1.0f;
    }
    return zmath_exp(y * zmath_log(x));
}

// Trigonometry.

#define ZMATH_SIN_C0 -0.1666666664f
#define ZMATH_SIN_C1  0.0083333315f
#define ZMATH_SIN_C2 -0.0001984090f
#define ZMATH_SIN_C3  0.0000027526f

ZMATHDEF float zmath_sin(float x) 
{
    float q = x * (1.0f / ZMATH_TAU);
    q = zmath_round(q);
    x -= q * ZMATH_TAU;

    if (x > ZMATH_HALF_PI) 
    {
        x = ZMATH_PI - x;
    }
    else if (x < -ZMATH_HALF_PI) 
    {
        x = -ZMATH_PI - x;
    }

    float x2 = x * x;
    return x * (1.0f + x2 * (ZMATH_SIN_C0 + x2 * (ZMATH_SIN_C1 + 
               x2 * (ZMATH_SIN_C2 + x2 * ZMATH_SIN_C3))));
}

ZMATHDEF float zmath_cos(float x)
{ 
    return zmath_sin(x + ZMATH_HALF_PI); 
}

ZMATHDEF float zmath_tan(float x) 
{
    float c = zmath_cos(x);
    if (zmath_abs(c) < 1e-5f) 
    {
        return 0.0f;
    }
    return zmath_sin(x) / c;
}

ZMATHDEF float zmath_atan(float x) 
{
    float sign = 1.0f;
    if (x < 0.0f) 
    { 
        x = -x; 
        sign = -1.0f; 
    }
    bool complement = (x > 1.0f);
    if (complement) 
    {
        x = 1.0f / x;
    }
    float x2 = x * x;
    float y = x * (0.99997726f + x2 * (-0.33262347f + x2 * (0.19354346f + 
              x2 * (-0.11643287f + x2 * (0.05265332f - x2 * 0.01172120f)))));
    if (complement) 
    {
        y = ZMATH_HALF_PI - y;
    }
    return sign * y;
}

ZMATHDEF float zmath_atan2(float y, float x) 
{
    if (x == 0.0f) 
    {
        return (y > 0.0f) ? ZMATH_HALF_PI : ((y < 0.0f) ? -ZMATH_HALF_PI : 0.0f);
    }
    float res = zmath_atan(y / x);
    if (x < 0.0f) 
    {
        res += (y >= 0.0f) ? ZMATH_PI : -ZMATH_PI;
    }
    return res;
}

ZMATHDEF float zmath_asin(float x) 
{
    x = zmath_clamp(x, -1.0f, 1.0f);
    return zmath_atan(x / zmath_sqrt(1.0f - x * x));
}

ZMATHDEF float zmath_acos(float x) 
{ 
    return ZMATH_HALF_PI - zmath_asin(x); 
}

ZMATHDEF float zmath_deg2rad(float deg) 
{ 
    return deg * (ZMATH_PI / 180.0f); 
}

ZMATHDEF float zmath_rad2deg(float rad) 
{ 
    return rad * (180.0f / ZMATH_PI); 
}

// Vector math.

ZMATHDEF zvec2 zmath_v2_add(zvec2 a, zvec2 b) 
{ 
    zvec2 r = {a.x+b.x, a.y+b.y}; 
    return r; 
}

ZMATHDEF zvec2 zmath_v2_sub(zvec2 a, zvec2 b) 
{
    zvec2 r = {a.x-b.x, a.y-b.y}; 
    return r; 
}

ZMATHDEF zvec2 zmath_v2_scale(zvec2 v, float s) 
{ 
    zvec2 r = {v.x*s, v.y*s}; 
    return r; 
}

ZMATHDEF float zmath_v2_dot(zvec2 a, zvec2 b) 
{ 
    return a.x*b.x + a.y*b.y; 
}

ZMATHDEF float zmath_v2_len(zvec2 v) 
{ 
    return zmath_sqrt(zmath_v2_dot(v, v)); 
}

ZMATHDEF zvec2 zmath_v2_norm(zvec2 v) 
{ 
    float len = zmath_v2_len(v);
    return (len > ZMATH_EPSILON) ? zmath_v2_scale(v, 1.0f/len) : v;
}

ZMATHDEF zvec3 zmath_v3_add(zvec3 a, zvec3 b) 
{ 
    zvec3 r = {a.x+b.x, a.y+b.y, a.z+b.z}; 
    return r; 
}

ZMATHDEF zvec3 zmath_v3_sub(zvec3 a, zvec3 b) 
{ 
    zvec3 r = {a.x-b.x, a.y-b.y, a.z-b.z}; 
    return r; 
}

ZMATHDEF zvec3 zmath_v3_scale(zvec3 v, float s) 
{ 
    zvec3 r = {v.x*s, v.y*s, v.z*s}; 
    return r; 
}

ZMATHDEF float zmath_v3_dot(zvec3 a, zvec3 b) 
{ 
    return a.x*b.x + a.y*b.y + a.z*b.z; 
}

ZMATHDEF zvec3 zmath_v3_cross(zvec3 a, zvec3 b) 
{
    zvec3 r = { a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x };
    return r;
}

ZMATHDEF float zmath_v3_len(zvec3 v) 
{ 
    return zmath_sqrt(zmath_v3_dot(v, v)); 
}

ZMATHDEF zvec3 zmath_v3_norm(zvec3 v) 
{
    float len = zmath_v3_len(v);
    return (len > ZMATH_EPSILON) ? zmath_v3_scale(v, 1.0f/len) : v;
}

#endif // ZMATH_IMPLEMENTATION
