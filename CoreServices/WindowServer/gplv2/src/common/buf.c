// SPDX-License-Identifier: GPL-2.0-only
#include <ctype.h>
#include "common/buf.h"


static void
strip_curly_braces(char *s)
{
	size_t len = strlen(s);
	if (s[0] != '{' || s[len - 1] != '}') {
		return;
	}
	len -= 2;
	memcpy(s, s + 1, len);
	s[len] = 0;
}

void
buf_expand_shell_variables(struct buf *s)
{
	struct buf new;
	struct buf environment_variable;
	buf_init(&new);
	buf_init(&environment_variable);

	for (int i = 0 ; i < s->len ; i++) {
		if (s->buf[i] == '$') {
			/* expand environment variable */
			environment_variable.len = 0;
			buf_add(&environment_variable, s->buf + i + 1);
			char *p = environment_variable.buf;
			while (isalnum(*p) || *p == '{' || *p == '}') {
				++p;
			}
			*p = '\0';
			i += strlen(environment_variable.buf);
			strip_curly_braces(environment_variable.buf);
			p = getenv(environment_variable.buf);
			if (p) {
				buf_add(&new, p);
			}
		} else if (s->buf[i] == '~') {
			/* expand tilde */
			buf_add(&new, getenv("HOME"));
		} else {
			/* just add one character at a time */
			if (new.alloc <= new.len + 1) {
				new.alloc = new.alloc * 3/2 + 16;
				new.buf = realloc(new.buf, new.alloc);
			}
			new.buf[new.len++] = s->buf[i];
			new.buf[new.len] = '\0';
		}
	}
	free(environment_variable.buf);
	free(s->buf);
	s->buf = new.buf;
}

void
buf_init(struct buf *s)
{
	s->alloc = 256;
	s->buf = malloc(s->alloc);
	s->buf[0] = '\0';
	s->len = 0;
}

void
buf_add(struct buf *s, const char *data)
{
	if (!data || data[0] == '\0') {
		return;
	}
	int len = strlen(data);
	if (s->alloc <= s->len + len + 1) {
		s->alloc = s->alloc + len;
		s->buf = realloc(s->buf, s->alloc);
	}
	memcpy(s->buf + s->len, data, len);
	s->len += len;
	s->buf[s->len] = 0;
}
