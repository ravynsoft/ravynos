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

#pragma once

#include "config.h"

#include <assert.h>

/* Don't use compat names in internal code. */
#define _XKBCOMMON_COMPAT_H
#include "xkbcommon/xkbcommon.h"
#include "xkbcommon/xkbcommon-compose.h"

#define ARRAY_SIZE(arr) ((sizeof(arr) / sizeof(*(arr))))

/* Fields that are printed in the interactive tools. */
enum print_state_fields {
#ifdef ENABLE_PRIVATE_APIS
    PRINT_MODMAPS = (1u << 1),
#endif
    PRINT_LAYOUT = (1u << 2),
    PRINT_UNICODE = (1u << 3),
    PRINT_ALL_FIELDS = ((PRINT_UNICODE << 1) - 1),
    /*
     * Fields that can be hidden with the option --short.
     * NOTE: If this value is modified, remember to update the documentation of
     *       the --short option in the corresponding tools.
     */
    PRINT_VERBOSE_FIELDS = (PRINT_LAYOUT | PRINT_UNICODE)
};
typedef uint32_t print_state_fields_mask_t;

#ifdef ENABLE_PRIVATE_APIS
void
print_keymap_modmaps(struct xkb_keymap *keymap);
void
print_keys_modmaps(struct xkb_keymap *keymap);
#endif

void
tools_print_keycode_state(struct xkb_state *state,
                          struct xkb_compose_state *compose_state,
                          xkb_keycode_t keycode,
                          enum xkb_consumed_mode consumed_mode,
                          print_state_fields_mask_t fields);

void
tools_print_state_changes(enum xkb_state_component changed);

void
tools_disable_stdin_echo(void);

void
tools_enable_stdin_echo(void);

int
tools_exec_command(const char *prefix, int argc, char **argv);

#ifdef _WIN32
#define setenv(varname, value, overwrite) _putenv_s((varname), (value))
#define unsetenv(varname) _putenv_s(varname, "")
#endif
