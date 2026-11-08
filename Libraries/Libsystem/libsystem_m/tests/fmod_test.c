#include <errno.h>
#include <fenv.h>
#include <math.h>
#include <stdio.h>

static int failures;

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, __LINE__, #condition); \
        ++failures; \
    } \
} while (0)

#define DEFINE_FMOD_TESTS(suffix, type, call, infinity, quiet_nan, signaling_nan, negative_zero, huge, remainder) \
static void test_fmod_##suffix(void) \
{ \
    volatile type x = (type)5.25; \
    volatile type y = (type)2.0; \
    volatile type inf = (type)(infinity); \
    volatile type zero = (type)0.0; \
    volatile type qnan = (type)(quiet_nan); \
    volatile type snan = (type)(signaling_nan); \
    volatile type huge_input = (type)(huge); \
    volatile type negative_zero_value = (type)(negative_zero); \
    type result; \
    \
    feclearexcept(FE_ALL_EXCEPT); \
    errno = EINTR; \
    result = call(x, y); \
    CHECK(result == (type)1.25); \
    CHECK(fetestexcept(FE_ALL_EXCEPT) == 0); \
    CHECK(errno == EINTR); \
    \
    feclearexcept(FE_ALL_EXCEPT); \
    result = call(-x, y); \
    CHECK(result == (type)-1.25); \
    CHECK(fetestexcept(FE_ALL_EXCEPT) == 0); \
    \
    feclearexcept(FE_ALL_EXCEPT); \
    result = call(x, -y); \
    CHECK(result == (type)1.25); \
    CHECK(fetestexcept(FE_ALL_EXCEPT) == 0); \
    \
    feclearexcept(FE_ALL_EXCEPT); \
    errno = 0; \
    result = call(inf, y); \
    CHECK(__builtin_isnan(result)); \
    CHECK((fetestexcept(FE_ALL_EXCEPT) & FE_INVALID) != 0); \
    CHECK((fetestexcept(FE_ALL_EXCEPT) & FE_DIVBYZERO) == 0); \
    CHECK(errno == EDOM); \
    \
    feclearexcept(FE_ALL_EXCEPT); \
    errno = 0; \
    result = call(-inf, y); \
    CHECK(__builtin_isnan(result)); \
    CHECK((fetestexcept(FE_ALL_EXCEPT) & FE_INVALID) != 0); \
    CHECK(errno == EDOM); \
    \
    feclearexcept(FE_ALL_EXCEPT); \
    errno = 0; \
    result = call(x, zero); \
    CHECK(__builtin_isnan(result)); \
    CHECK((fetestexcept(FE_ALL_EXCEPT) & FE_INVALID) != 0); \
    CHECK((fetestexcept(FE_ALL_EXCEPT) & FE_DIVBYZERO) == 0); \
    CHECK(errno == EDOM); \
    \
    feclearexcept(FE_ALL_EXCEPT); \
    errno = 0; \
    result = call(x, negative_zero_value); \
    CHECK(__builtin_isnan(result)); \
    CHECK((fetestexcept(FE_ALL_EXCEPT) & FE_INVALID) != 0); \
    CHECK(errno == EDOM); \
    \
    feclearexcept(FE_ALL_EXCEPT); \
    result = call(huge_input, (type)3.0); \
    CHECK(result == (type)(remainder)); \
    CHECK(fetestexcept(FE_ALL_EXCEPT) == 0); \
    \
    feclearexcept(FE_ALL_EXCEPT); \
    result = call((type)(infinity), (type)(infinity)); \
    CHECK(__builtin_isnan(result)); \
    CHECK((fetestexcept(FE_ALL_EXCEPT) & FE_INVALID) != 0); \
    CHECK(errno == EDOM); \
    \
    feclearexcept(FE_ALL_EXCEPT); \
    errno = EINTR; \
    result = call((type)5.0, inf); \
    CHECK(result == (type)5.0); \
    CHECK(fetestexcept(FE_ALL_EXCEPT) == 0); \
    CHECK(errno == EINTR); \
    \
    feclearexcept(FE_ALL_EXCEPT); \
    result = call(negative_zero_value, (type)3.0); \
    CHECK(result == (type)0.0); \
    CHECK(__builtin_signbit(result)); \
    CHECK(fetestexcept(FE_ALL_EXCEPT) == 0); \
    \
    feclearexcept(FE_ALL_EXCEPT); \
    errno = EINTR; \
    result = call(qnan, (type)2.0); \
    CHECK(__builtin_isnan(result)); \
    CHECK((fetestexcept(FE_ALL_EXCEPT) & FE_INVALID) == 0); \
    CHECK(errno == EINTR); \
    \
    feclearexcept(FE_ALL_EXCEPT); \
    errno = EINTR; \
    result = call(snan, (type)2.0); \
    CHECK(__builtin_isnan(result)); \
    CHECK((fetestexcept(FE_ALL_EXCEPT) & FE_INVALID) != 0); \
    CHECK(errno == EINTR); \
}

DEFINE_FMOD_TESTS(f, float, fmodf, __builtin_inff(), __builtin_nanf(""), __builtin_nansf(""), \
    __builtin_copysignf(0.0f, -1.0f), 0x1p40f, 1.0f)
DEFINE_FMOD_TESTS(d, double, fmod, __builtin_inf(), __builtin_nan(""), __builtin_nans(""), \
    __builtin_copysign(0.0, -1.0), 0x1p100, 1.0)
DEFINE_FMOD_TESTS(l, long double, fmodl, __builtin_infl(), __builtin_nanl(""), __builtin_nansl(""), \
    __builtin_copysignl(0.0L, -1.0L), 0x1p1000L, 1.0L)

int main(void)
{
    test_fmod_f();
    test_fmod_d();
    test_fmod_l();
    if (failures != 0)
        return 1;
    puts("fmod tests passed");
    return 0;
}
