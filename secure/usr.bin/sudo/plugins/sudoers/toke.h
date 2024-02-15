/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2011-2013, 2015-2016, 2020-2023
 *	Todd C. Miller <Todd.Miller@sudo.ws>
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

#ifndef SUDOERS_TOKE_H
#define SUDOERS_TOKE_H

struct sudolinebuf {
    char *buf;			/* line buffer */
    size_t size;		/* size of buffer */
    size_t len;			/* used length */
    size_t off;			/* consumed length */
    size_t toke_start;		/* starting column of current token */
    size_t toke_end;		/* ending column of current token */
};

extern const char *sudoers_errstr;
extern struct sudolinebuf sudolinebuf;
extern int sudolineno;
extern char *sudoers_search_path;

struct sudoers_parser_config;
bool append(const char *, int);
bool fill_args(const char *, int, bool);
bool fill_cmnd(const char *, int);
bool fill(const char *, int);
void init_lexer(void);
bool ipv6_valid(const char *s);
int sudoers_trace_print(const char *);
void sudoerserrorf(const char *, ...) sudo_printf0like(1, 2);
void sudoerserror(const char *);
bool push_include(const char *, const char *, struct sudoers_parser_config *);
bool push_includedir(const char *, const char *, struct sudoers_parser_config *);

#ifndef FLEX_SCANNER
extern int (*trace_print)(const char *msg);
#endif

#define LEXTRACE(msg)   do {						\
    if (trace_print != NULL)						\
	(*trace_print)(msg);						\
} while (0);

#endif /* SUDOERS_TOKE_H */
