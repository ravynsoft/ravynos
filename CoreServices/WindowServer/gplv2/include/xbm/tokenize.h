/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * XBM file tokenizer
 *
 * Copyright Johan Malm 2020
 */

#ifndef __LABWC_TOKENIZE_H
#define __LABWC_TOKENIZE_H

enum token_type {
	TOKEN_NONE = 0,
	TOKEN_IDENT,
	TOKEN_INT,
	TOKEN_SPECIAL,
	TOKEN_OTHER,
};

#define MAX_TOKEN_SIZE (256)
struct token {
	char name[MAX_TOKEN_SIZE];
	int value;
	size_t pos;
	enum token_type type;
};

/**
 * tokenize - tokenize xbm file
 * @buffer: buffer containing xbm file
 * return token vector
 */
struct token *tokenize_xbm(char *buffer);

#endif /* __LABWC_TOKENIZE_H */
