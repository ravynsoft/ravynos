/*
 * Copyright © 2021 Adrian Johnson
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Author: Adrian Johnson <ajohnson@redneon.com>
 */


#include "cairo-test.h"

#include <cairo.h>

/* Uncomment to enable faulty test data in order to force a
 * failure. This allows the error logging path to be tested.
 */
/* #define FORCE_FAILURE 1 */

struct test_data {
    uint64_t a;
    uint64_t b;
    uint64_t result;
    cairo_bool_t overflow;
};

#if SIZEOF_SIZE_T == 4
static const struct test_data add_32bit_test_data[] = {
    { 0x00000000, 0x00000000, 0x00000000, 0 },
    { 0x00000001, 0x00000000, 0x00000001, 0 },
    { 0x00000000, 0x00000001, 0x00000001, 0 },
    { 0xffffffff, 0x00000001, 0x00000000, 1 },
    { 0x00000001, 0xffffffff, 0x00000000, 1 },
    { 0xfffffffe, 0x00000001, 0xffffffff, 0 },
    { 0x00000001, 0xfffffffe, 0xffffffff, 0 },
    { 0x12345678, 0x98765432, 0xaaaaaaaa, 0 },
    { 0x80000000, 0x80000000, 0x00000000, 1 },

#if FORCE_FAILURE
    { 0x00000001, 0x00000002, 0x00000004, 1 },
#endif
};

static const struct test_data mul_32bit_test_data[] = {
    { 0x00000000, 0x00000000, 0x00000000, 0 },
    { 0x0000ffff, 0x0000ffff, 0xfffe0001, 0 },
    { 0x00010000, 0x00010000, 0x00000000, 1 },
    { 0x00000002, 0x80000000, 0x00000000, 1 },
    { 0x80000000, 0x00000002, 0x00000000, 1 },
    { 0xffffffff, 0x00000001, 0xffffffff, 0 },
};
#endif

#if SIZEOF_SIZE_T == 8
static const struct test_data add_64bit_test_data[] = {
    { 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0 },
    { 0x0000000000000001, 0x0000000000000000, 0x0000000000000001, 0 },
    { 0x0000000000000000, 0x0000000000000001, 0x0000000000000001, 0 },
    { 0xffffffffffffffff, 0x0000000000000001, 0x0000000000000000, 1 },
    { 0x0000000000000001, 0xffffffffffffffff, 0x0000000000000000, 1 },
    { 0x0000000000000001, 0xfffffffffffffffe, 0xffffffffffffffff, 0 },
    { 0xfffffffffffffffe, 0x0000000000000001, 0xffffffffffffffff, 0 },
    { 0x123456789abcdef0, 0x987654320fedcbba, 0xaaaaaaaaaaaaaaaa, 0 },
    { 0x8000000000000000, 0x8000000000000000, 0x0000000000000000, 1 },

#if FORCE_FAILURE
    { 0x0000000000000001, 0x0000000000000002, 0x0000000000000004, 1 },
#endif
};

static const struct test_data mul_64bit_test_data[] = {
    { 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0 },
    { 0x00000000ffffffff, 0x00000000ffffffff, 0xfffffffe00000001, 0 },
    { 0x0000000100000000, 0x0000000100000000, 0x0000000000000000, 1 },
    { 0x0000000000000002, 0x8000000000000000, 0x0000000000000000, 1 },
    { 0x8000000000000000, 0x0000000000000002, 0x0000000000000000, 1 },
    { 0xffffffffffffffff, 0x0000000000000001, 0xffffffffffffffff, 0 },
};
#endif

static cairo_bool_t
check_if_result_fail (cairo_test_context_t *ctx,
                      size_t result,
                      cairo_bool_t overflow,
                      const struct test_data *data,
                      const char *func_name)
{
    int hex_digits = SIZEOF_SIZE_T * 2;
    if (overflow != data->overflow || (!data->overflow && result != data->result)) {
        cairo_test_log (ctx, "%s a = 0x%0*llx b = 0x%0*llx result = 0x%0*llx overflow = %d\n",
                        func_name,
                        hex_digits,
                        (unsigned long long)data->a,
                        hex_digits,
                        (unsigned long long)data->b,
                        hex_digits,
                        (unsigned long long)result,
                        overflow);
        if (data->overflow)
            cairo_test_log (ctx, "EXPECTED overflow = 1\n");
        else
            cairo_test_log (ctx, "EXPECTED result = 0x%0*llx overflow = 0\n",
                            hex_digits,
                            (unsigned long long)data->result);
        return TRUE;
    }
    return FALSE;
}

static cairo_test_status_t
preamble (cairo_test_context_t *ctx)
{
    int i;
    cairo_bool_t overflow;
    size_t result;

#if SIZEOF_SIZE_T == 4
    const struct test_data *add_data = add_32bit_test_data;
    int num_add_tests = ARRAY_LENGTH(add_32bit_test_data);
    const struct test_data *mul_data = mul_32bit_test_data;
    int num_mul_tests = ARRAY_LENGTH(mul_32bit_test_data);
#elif SIZEOF_SIZE_T == 8
    const struct test_data *add_data = add_64bit_test_data;
    int num_add_tests = ARRAY_LENGTH(add_64bit_test_data);
    const struct test_data *mul_data = mul_64bit_test_data;
    int num_mul_tests = ARRAY_LENGTH(mul_64bit_test_data);
#else
    cairo_test_log (ctx, "sizeof(size_t) = %lld is not supported by this test\n",
                    (unsigned long long)sizeof(size_t));
    return CAIRO_TEST_UNTESTED;
#endif

    /* First check the fallback versions of the overflow functions. */
    for (i = 0; i < num_add_tests; i++) {
        const struct test_data *data = &add_data[i];
        overflow = _cairo_fallback_add_size_t_overflow (data->a, data->b, &result);
        if (check_if_result_fail (ctx,
                                  result,
                                  overflow,
                                  data,
                                  "_cairo_fallback_add_size_t_overflow"))
        {
            return CAIRO_TEST_FAILURE;
        }
    }

    for (i = 0; i < num_mul_tests; i++) {
        const struct test_data *data = &mul_data[i];
        overflow = _cairo_fallback_mul_size_t_overflow (data->a, data->b, &result);
        if (check_if_result_fail (ctx,
                                  result,
                                  overflow,
                                  data,
                                  "_cairo_fallback_mul_size_t_overflow"))
        {
            return CAIRO_TEST_FAILURE;
        }
    }
    /* Next check the compiler builtins (if available, otherwise the
     * fallback versions are tested again). This is to ensure the fallback version
     * produces identical results to the compiler builtins.
     */
    for (i = 0; i < num_add_tests; i++) {
        const struct test_data *data = &add_data[i];
        overflow = _cairo_add_size_t_overflow (data->a, data->b, &result);
        if (check_if_result_fail (ctx,
                                  result,
                                  overflow,
                                  data,
                                  "_cairo_add_size_t_overflow"))
        {
            return CAIRO_TEST_FAILURE;
        }
    }

    for (i = 0; i < num_mul_tests; i++) {
        const struct test_data *data = &mul_data[i];
        overflow = _cairo_mul_size_t_overflow (data->a, data->b, &result);
        if (check_if_result_fail (ctx,
                                  result,
                                  overflow,
                                  data,
                                  "_cairo_mul_size_t_overflow"))
        {
            return CAIRO_TEST_FAILURE;
        }
    }

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (overflow,
	    "Test the _cairo_*_size_t_overflow functions.",
	    "memory", /* keywords */
	    NULL, /* requirements */
	    0, 0,
	    preamble, NULL)
