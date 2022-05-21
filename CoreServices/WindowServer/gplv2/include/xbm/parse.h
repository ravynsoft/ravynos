/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Parse xbm token to create pixmap
 *
 * Copyright Johan Malm 2020
 */

#ifndef __LABWC_PARSE_H
#define __LABWC_PARSE_H

#include "xbm/tokenize.h"
#include <stdint.h>

struct pixmap {
	uint32_t *data;
	int width;
	int height;
};

/**
 * parse_set_color - set color to be used when parsing icons
 * @rgba: four floats representing red, green, blue, alpha
 */
void parse_set_color(float *rgba);

/**
 * parse_xbm_tokens - parse xbm tokens and create pixmap
 * @tokens: token vector
 */
struct pixmap parse_xbm_tokens(struct token *tokens);

/**
 * parse_xbm_builtin - parse builtin xbm button and create pixmap
 * @button: button byte array (xbm format)
 */
struct pixmap parse_xbm_builtin(const char *button, int size);

#endif /* __LABWC_PARSE_H */
