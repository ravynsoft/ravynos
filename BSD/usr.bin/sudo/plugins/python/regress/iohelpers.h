/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2020 Robert Manner <robert.manner@oneidentity.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef PYTHON_IO_HELPERS
#define PYTHON_IO_HELPERS

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# include <compat/stdbool.h>
#endif /* HAVE_STDBOOL_H */
#include <string.h>
#include <stdarg.h>
#include <signal.h>
#include <pwd.h>

#include <sudo_compat.h>

#define MAX_OUTPUT (2 << 16)

int rmdir_recursive(const char *path);

int fwriteall(const char *file_path, const char *string);
int freadall(const char *file_path, char *output, size_t max_len);

// allocates new string with the content of 'string' but 'old' replaced to 'new'
// The allocated array will be dest_length size and null terminated correctly.
char *str_replaced(const char *string, size_t dest_length, const char *old, const char *new);

// same, but "string" must be able to store 'max_length' number of characters including the null terminator
void str_replace_in_place(char *string, size_t max_length, const char *old, const char *new);

int vsnprintf_append(char * restrict output, size_t max_output_len, const char * restrict fmt, va_list args);
int snprintf_append(char * restrict output, size_t max_output_len, const char * restrict fmt, ...);

int str_array_count(char **str_array);
void str_array_snprint(char *out_str, size_t max_len, char **str_array, int array_len);

#endif
