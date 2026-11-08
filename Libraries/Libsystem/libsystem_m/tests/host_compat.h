/*
 * The host-side regression harness compiles math.c against glibc headers.
 * These aliases keep unrelated Darwin-specific declarations in that source
 * from colliding with glibc declarations. The tested fmod symbols are not
 * renamed, so the harness links directly to the ravynOS implementation.
 */
#define _GNU_SOURCE
#include <stdbool.h>
#include <math.h>
#include <float.h>

#undef fpclassify
#undef signbit
#undef isfinite
#undef isinf
#undef isnan
#undef isnormal
#undef isgreater
#undef isgreaterequal
#undef isless
#undef islessequal
#undef islessgreater
#undef isunordered

#define signbit ravynos_host_signbit
#define isfinite ravynos_host_isfinite
#define isinf ravynos_host_isinf
#define isnan ravynos_host_isnan
#define isnormal ravynos_host_isnormal
#define isgreater ravynos_host_isgreater
#define isgreaterequal ravynos_host_isgreaterequal
#define isless ravynos_host_isless
#define islessequal ravynos_host_islessequal
#define islessgreater ravynos_host_islessgreater
#define isunordered ravynos_host_isunordered
#define copysignf ravynos_host_copysignf
#define ilogb ravynos_host_ilogb

typedef struct {
    double sin;
    double cos;
} __sincos_stret_f64_t;

typedef float __v4sf __attribute__((vector_size(16)));
