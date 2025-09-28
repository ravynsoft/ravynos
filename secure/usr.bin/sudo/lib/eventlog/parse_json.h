/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2020, 2023 Todd C. Miller <Todd.Miller@sudo.ws>
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

#ifndef PARSE_JSON_H
#define PARSE_JSON_H

#include <sudo_json.h>
#include <sudo_queue.h>

TAILQ_HEAD(json_item_list, json_item);

struct eventlog_json_object {
    struct json_item *parent;
    struct json_item_list items;
};

struct json_item {
    TAILQ_ENTRY(json_item) entries;
    char *name;		/* may be NULL for first brace */
    unsigned int lineno;
    enum json_value_type type;
    union {
	struct eventlog_json_object child;
	char *string;
	long long number;
	id_t id;
	bool boolean;
    } u;
};

#endif /* PARSE_JSON_H */
