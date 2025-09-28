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

/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 */

#include "iohelpers.h"
#include <sudo_fatal.h>

int
rmdir_recursive(const char *path)
{
    char *cmd = NULL;
    int success = false;

    if (asprintf(&cmd, "rm -rf \"%s\"", path) < 0)
        return false;

    if (system(cmd) == 0)
        success = true;

    free(cmd);

    return success;
}

int
fwriteall(const char *file_path, const char *string)
{
    int success = false;

    FILE *file = fopen(file_path, "w+");
    if (file == NULL)
        goto cleanup;

    size_t size = strlen(string);
    if (fwrite(string, 1, size, file) < size) {
        goto cleanup;
    }

    success = true;

cleanup:
    if (file)
        fclose(file);

    return success;
}

int
freadall(const char *file_path, char *output, size_t max_len)
{
    int rc = false;
    FILE *file = fopen(file_path, "rb");
    if (file == NULL) {
	sudo_warn_nodebug("failed to open file '%s'", file_path);
        goto cleanup;
    }

    size_t len = fread(output, 1, max_len - 1, file);
    output[len] = '\0';

    if (ferror(file) != 0) {
        sudo_warn_nodebug("failed to read file '%s'", file_path);
        goto cleanup;
    }

    if (!feof(file)) {
        sudo_warn_nodebug("file '%s' was bigger than allocated buffer %zu",
	    file_path, max_len);
        goto cleanup;
    }

    rc = true;

cleanup:
    if (file)
        fclose(file);

    return rc;
}

int
vsnprintf_append(char * restrict output, size_t max_output_len, const char * restrict fmt, va_list args)
{
    va_list args2;
    va_copy(args2, args);

    size_t output_len = strlen(output);
    int rc = vsnprintf(output + output_len, max_output_len - output_len, fmt, args2);

    va_end(args2);
    return rc;
}

int
snprintf_append(char * restrict output, size_t max_output_len, const char * restrict fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int rc = vsnprintf_append(output, max_output_len, fmt, args);
    va_end(args);
    return rc;
}

int
str_array_count(char **str_array)
{
    int result = 0;
    for (; str_array[result] != NULL; ++result) {}
    return result;
}

void
str_array_snprint(char *out_str, size_t max_len, char **str_array, int array_len)
{
    if (array_len < 0)
        array_len = str_array_count(str_array);

    for (int pos = 0; pos < array_len; ++pos) {
        snprintf_append(out_str, max_len, "%s%s", pos > 0 ? ", " : "", str_array[pos]);
    }
}

char *
str_replaced(const char *source, size_t dest_len, const char *old, const char *new)
{
    char *result = malloc(dest_len);
    char *dest = result;
    char *pos = NULL;
    size_t old_len = strlen(old);

    if (result == NULL)
        return NULL;

    while ((pos = strstr(source, old)) != NULL) {
        size_t len = (size_t)snprintf(dest, dest_len,
            "%.*s%s", (int)(pos - source), source, new);
        if (len >= dest_len)
            goto fail;

        dest_len -= len;
        dest += len;
        source = pos + old_len;
    }

    if (strlcpy(dest, source, dest_len) >= dest_len)
        goto fail;

    return result;

fail:
    free(result);
    return strdup("str_replace_all failed, string too long");
}

void
str_replace_in_place(char *string, size_t max_length, const char *old, const char *new)
{
    char *replaced = str_replaced(string, max_length, old, new);
    if (replaced != NULL) {
        strlcpy(string, replaced, max_length);
        free(replaced);
    }
}
