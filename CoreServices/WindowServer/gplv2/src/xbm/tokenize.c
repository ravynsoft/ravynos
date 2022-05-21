// SPDX-License-Identifier: GPL-2.0-only
/*
 * XBM file tokenizer
 *
 * Copyright Johan Malm 2020
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xbm/tokenize.h"

static char *current_buffer_position;
static struct token *tokens;
static int nr_tokens, alloc_tokens;

static void
add_token(enum token_type token_type)
{
	if (nr_tokens == alloc_tokens) {
		alloc_tokens = (alloc_tokens + 16) * 2;
		tokens = realloc(tokens, alloc_tokens * sizeof(struct token));
	}
	struct token *token = tokens + nr_tokens;
	memset(token, 0, sizeof(*token));
	nr_tokens++;
	token->type = token_type;
}

static void
get_identifier_token()
{
	struct token *token = tokens + nr_tokens - 1;
	token->name[token->pos] = current_buffer_position[0];
	token->pos++;
	if (token->pos == MAX_TOKEN_SIZE - 1) {
		return;
	}
	current_buffer_position++;
	switch (current_buffer_position[0]) {
	case '\0':
		return;
	case 'a' ... 'z':
	case 'A' ... 'Z':
	case '0' ... '9':
	case '_':
	case '#':
		get_identifier_token();
		break;
	default:
		break;
	}
}

static void
get_number_token(void)
{
	struct token *token = tokens + nr_tokens - 1;
	token->name[token->pos] = current_buffer_position[0];
	token->pos++;
	if (token->pos == MAX_TOKEN_SIZE - 1) {
		return;
	}
	current_buffer_position++;
	switch (current_buffer_position[0]) {
	case '\0':
		return;
	case '0' ... '9':
	case 'a' ... 'f':
	case 'A' ... 'F':
	case 'x':
		get_number_token();
		break;
	default:
		break;
	}
}

static void
get_special_char_token()
{
	struct token *token = tokens + nr_tokens - 1;
	token->name[0] = current_buffer_position[0];
	current_buffer_position++;
}

struct token *
tokenize_xbm(char *buffer)
{
	tokens = NULL;
	nr_tokens = 0;
	alloc_tokens = 0;

	current_buffer_position = buffer;

	for (;;) {
		switch (current_buffer_position[0]) {
		case '\0':
			goto out;
		case 'a' ... 'z':
		case 'A' ... 'Z':
		case '_':
		case '#':
			add_token(TOKEN_IDENT);
			get_identifier_token();
			continue;
		case '0' ... '9':
			add_token(TOKEN_INT);
			get_number_token();
			struct token *token = tokens + nr_tokens - 1;
			token->value = (int)strtol(token->name, NULL, 0);
			continue;
		case '{':
			add_token(TOKEN_SPECIAL);
			get_special_char_token();
			continue;
		default:
			break;
		}
		++current_buffer_position;
	}
out:
	add_token(TOKEN_NONE); /* vector end marker */
	return tokens;
}
