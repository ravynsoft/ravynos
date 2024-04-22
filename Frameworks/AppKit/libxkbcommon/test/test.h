/*
 * Copyright © 2012 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Author: Daniel Stone <daniel@fooishbar.org>
 */

#include <assert.h>

/* Don't use compat names in internal code. */
#define _XKBCOMMON_COMPAT_H
#include "xkbcommon/xkbcommon.h"
#include "xkbcommon/xkbcommon-compose.h"
#include "utils.h"

/* Automake test exit code to signify SKIP (à la PASS, FAIL, etc). */
#define SKIP_TEST 77

/* The offset between KEY_* numbering, and keycodes in the XKB evdev
 * dataset. */
#define EVDEV_OFFSET 8

enum key_seq_state {
    DOWN,
    REPEAT,
    UP,
    BOTH,
    NEXT,
    FINISH,
};

int
test_key_seq(struct xkb_keymap *keymap, ...);

int
test_key_seq_va(struct xkb_keymap *keymap, va_list args);

char *
test_makedir(const char *parent, const char *path);

char *
test_maketempdir(const char *template);

char *
test_get_path(const char *path_rel);

char *
test_read_file(const char *path_rel);

enum test_context_flags {
    CONTEXT_NO_FLAG = 0,
    CONTEXT_ALLOW_ENVIRONMENT_NAMES = (1 << 0),
};

struct xkb_context *
test_get_context(enum test_context_flags flags);

struct xkb_keymap *
test_compile_file(struct xkb_context *context, const char *path_rel);

struct xkb_keymap *
test_compile_string(struct xkb_context *context, const char *string);

struct xkb_keymap *
test_compile_buffer(struct xkb_context *context, const char *buf, size_t len);

struct xkb_keymap *
test_compile_rules(struct xkb_context *context, const char *rules,
                   const char *model, const char *layout, const char *variant,
                   const char *options);


#ifdef _WIN32
#define setenv(varname, value, overwrite) _putenv_s((varname), (value))
#define unsetenv(varname) _putenv_s(varname, "")
#endif
