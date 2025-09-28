/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2013-2020 Todd C. Miller <Todd.Miller@sudo.ws>
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

#ifndef SUDO_JSON_H
#define SUDO_JSON_H

#include <sys/types.h>	/* for id_t */

#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# include <compat/stdbool.h>
#endif

/*
 * JSON values may be of the following types.
 */
enum json_value_type {
    JSON_STRING,
    JSON_ID,
    JSON_NUMBER,
    JSON_OBJECT,
    JSON_ARRAY,
    JSON_BOOL,
    JSON_NULL
};

/*
 * JSON value suitable for printing.
 * Note: this does not support object values.
 */
struct json_value {
    enum json_value_type type;
    union {
	const char *string;
	long long number;
	id_t id;
	bool boolean;
    } u;
};

struct json_container {
    char *buf;
    unsigned int buflen;
    unsigned int bufsize;
    unsigned int indent_level;
    unsigned int indent_increment;
    bool minimal;
    bool memfatal;
    bool need_comma;
    bool quiet;
};

sudo_dso_public bool sudo_json_init_v1(struct json_container *jsonc, unsigned int indent, bool minimal, bool memfatal);
sudo_dso_public bool sudo_json_init_v2(struct json_container *jsonc, unsigned int indent, bool minimal, bool memfatal, bool quiet);
#define sudo_json_init(_a, _b, _c, _d, _e) sudo_json_init_v2((_a), (_b), (_c), (_d), (_e))

sudo_dso_public void sudo_json_free_v1(struct json_container *jsonc);
#define sudo_json_free(_a) sudo_json_free_v1((_a))

sudo_dso_public bool sudo_json_open_object_v1(struct json_container *jsonc, const char *name);
#define sudo_json_open_object(_a, _b) sudo_json_open_object_v1((_a), (_b))

sudo_dso_public bool sudo_json_close_object_v1(struct json_container *jsonc);
#define sudo_json_close_object(_a) sudo_json_close_object_v1((_a))

sudo_dso_public bool sudo_json_open_array_v1(struct json_container *jsonc, const char *name);
#define sudo_json_open_array(_a, _b) sudo_json_open_array_v1((_a), (_b))

sudo_dso_public bool sudo_json_close_array_v1(struct json_container *jsonc);
#define sudo_json_close_array(_a) sudo_json_close_array_v1((_a))

sudo_dso_public bool sudo_json_add_value_v1(struct json_container *jsonc, const char *name, struct json_value *value);
#define sudo_json_add_value(_a, _b, _c) sudo_json_add_value_v1((_a), (_b), (_c))

sudo_dso_public bool sudo_json_add_value_as_object_v1(struct json_container *jsonc, const char *name, struct json_value *value);
#define sudo_json_add_value_as_object(_a, _b, _c) sudo_json_add_value_as_object_v1((_a), (_b), (_c))

sudo_dso_public char *sudo_json_get_buf_v1(struct json_container *jsonc);
#define sudo_json_get_buf(_a) sudo_json_get_buf_v1((_a))

sudo_dso_public unsigned int sudo_json_get_len_v1(struct json_container *jsonc);
#define sudo_json_get_len(_a) sudo_json_get_len_v1((_a))

#endif /* SUDO_JSON_H */
