// SPDX-License-Identifier: GPL-2.0-only
/*
 * Parse xbm token to create pixmap
 *
 * Copyright Johan Malm 2020
 */

#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xbm/parse.h"

static uint32_t color;

static uint32_t
u32(float *rgba)
{
	uint32_t r[4] = { 0 };
	for (int i = 0; i < 4; i++) {
		r[i] = rgba[i] * 255;
	}
	return ((r[3] & 0xff) << 24) | ((r[0] & 0xff) << 16) |
	       ((r[1] & 0xff) << 8) | (r[2] & 0xff);
}

void
parse_set_color(float *rgba)
{
	color = u32(rgba);
}

static void
process_bytes(struct pixmap *pixmap, struct token *tokens)
{
	pixmap->data = (uint32_t *)calloc(pixmap->width * pixmap->height,
					  sizeof(uint32_t));
	struct token *t = tokens;
	for (int row = 0; row < pixmap->height; row++) {
		int byte = 1;
		for (int col = 0; col < pixmap->width; col++) {
			if (col == byte * 8) {
				++byte;
				++t;
			}
			if (!t->type) {
				return;
			}
			if (t->type != TOKEN_INT) {
				return;
			}
			int bit = 1 << (col % 8);
			if (t->value & bit) {
				pixmap->data[row * pixmap->width + col] = color;
			}
		}
		++t;
	}
}

struct pixmap
parse_xbm_tokens(struct token *tokens)
{
	struct pixmap pixmap = { 0 };

	for (struct token *t = tokens; t->type; t++) {
		if (pixmap.width && pixmap.height) {
			if (t->type != TOKEN_INT) {
				continue;
			}
			process_bytes(&pixmap, t);
			goto out;
		}
		if (strstr(t->name, "width")) {
			pixmap.width = atoi((++t)->name);
		} else if (strstr(t->name, "height")) {
			pixmap.height = atoi((++t)->name);
		}
	}
out:
	return pixmap;
}

/*
 * Openbox built-in icons are not bigger than 8x8, so have only written this
 * function to cope wit that max size
 */
#define LABWC_BUILTIN_ICON_MAX_SIZE (8)
struct pixmap
parse_xbm_builtin(const char *button, int size)
{
	struct pixmap pixmap = { 0 };

	assert(size <= LABWC_BUILTIN_ICON_MAX_SIZE);
	pixmap.width = size;
	pixmap.height = size;

	struct token t[LABWC_BUILTIN_ICON_MAX_SIZE + 1];
	for (int i = 0; i < size; i++) {
		t[i].value = button[i];
		t[i].type = TOKEN_INT;
	}
	t[size].type = 0;
	process_bytes(&pixmap, t);
	return pixmap;
}
