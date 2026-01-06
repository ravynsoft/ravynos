/*
 * corecrypto stubs for compiling ld64 on Linux
 * Copyright (C) 2025 Zoe Knox <zoe@pixin.net>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <corecrypto/ccdigest.h>

/* ld64 only uses SHA digests so create a minimal g_crypto_funcs */
typedef void (*ccdigest_init_fn_t)(const struct ccdigest_info *di, ccdigest_ctx_t ctx);
typedef void (*ccdigest_update_fn_t)(const struct ccdigest_info *di, ccdigest_ctx_t ctx,
    unsigned long len, const void *data);
typedef void (*ccdigest_final_fn_t)(const struct ccdigest_info *di, ccdigest_ctx_t ctx,
    void *digest);
typedef void (*ccdigest_fn_t)(const struct ccdigest_info *di, unsigned long len,
    const void *data, void *digest);

typedef struct crypto_functions {
	ccdigest_init_fn_t ccdigest_init_fn;
	ccdigest_update_fn_t ccdigest_update_fn;
	ccdigest_final_fn_t ccdigest_final_fn;
	ccdigest_fn_t ccdigest_fn;
    const struct ccdigest_info *ccmd5_di;
	const struct ccdigest_info *ccsha1_di;
	const struct ccdigest_info *ccsha256_di;
	const struct ccdigest_info *ccsha384_di;
	const struct ccdigest_info *ccsha512_di;
} *crypto_functions_t;

crypto_functions_t g_crypto_funcs = NULL;

int
register_crypto_functions(const crypto_functions_t funcs)
{
	if (g_crypto_funcs) {
		return -1;
	}

	g_crypto_funcs = funcs;

	return 0;
}

/* Aaaaand a few other things we need to build on Linux */

void __assert_rtn(const char *func, const char *file, int line, const char *expr)
{
    printf("Assertion failed: %s in %s at %s:%d\n", expr, func, file, line);
    exit(1);
}

void panic(char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    printf("PANIC: ");
    vprintf(fmt, args);
    va_end(args);
    exit(2);
}