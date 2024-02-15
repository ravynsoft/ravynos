/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2007-2015, 2023 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <config.h>

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include <sudo_compat.h>
#include <sudo_debug.h>
#include <sudo_lbuf.h>
#include <sudo_util.h>

void
sudo_lbuf_init_v1(struct sudo_lbuf *lbuf, sudo_lbuf_output_t output,
    unsigned int indent, const char *continuation, int cols)
{
    debug_decl(sudo_lbuf_init, SUDO_DEBUG_UTIL);

    if (cols < 0)
	cols = 0;

    lbuf->output = output;
    lbuf->continuation = continuation;
    lbuf->indent = indent;
    lbuf->cols = (unsigned short)cols;
    lbuf->error = 0;
    lbuf->len = 0;
    lbuf->size = 0;
    lbuf->buf = NULL;

    debug_return;
}

void
sudo_lbuf_destroy_v1(struct sudo_lbuf *lbuf)
{
    debug_decl(sudo_lbuf_destroy, SUDO_DEBUG_UTIL);

    free(lbuf->buf);
    lbuf->error = 0;
    lbuf->len = 0;
    lbuf->size = 0;
    lbuf->buf = NULL;

    debug_return;
}

static bool
sudo_lbuf_expand(struct sudo_lbuf *lbuf, unsigned int extra)
{
    debug_decl(sudo_lbuf_expand, SUDO_DEBUG_UTIL);

    if (lbuf->len + extra + 1 <= lbuf->len) {
	errno = ENOMEM;
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "integer overflow updating lbuf->len");
	lbuf->error = 1;
	debug_return_bool(false);
    }

    if (lbuf->len + extra + 1 > lbuf->size) {
	const size_t size = lbuf->len + extra + 1;
	size_t new_size = sudo_pow2_roundup(size);
	char *new_buf;

	if (new_size > UINT_MAX || new_size < lbuf->size) {
	    errno = ENOMEM;
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"integer overflow updating lbuf->size");
	    lbuf->error = 1;
	    debug_return_bool(false);
	}
	if (new_size < 1024)
	    new_size = 1024;
	if ((new_buf = realloc(lbuf->buf, new_size)) == NULL) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"unable to allocate memory");
	    lbuf->error = 1;
	    debug_return_bool(false);
	}
	lbuf->buf = new_buf;
	lbuf->size = (unsigned int)new_size;
    }
    debug_return_bool(true);
}

/*
 * Escape a character in octal form (#0n) and store it as a string
 * in buf, which must have at least 6 bytes available.
 * Returns the length of buf, not counting the terminating NUL byte.
 */
static unsigned int
escape(char ch, char *buf)
{
    unsigned char uch = (unsigned char)ch;
    const unsigned int len = uch < 0100 ? (uch < 010 ? 3 : 4) : 5;

    /* Work backwards from the least significant digit to most significant. */
    switch (len) {
    case 5:
	buf[4] = (uch & 7) + '0';
	uch >>= 3;
	FALLTHROUGH;
    case 4:
	buf[3] = (uch & 7) + '0';
	uch >>= 3;
	FALLTHROUGH;
    case 3:
	buf[2] = (uch & 7) + '0';
	buf[1] = '0';
	buf[0] = '#';
	break;
    }
    buf[len] = '\0';

    return len;
}

/*
 * Parse the format and append strings, only %s and %% escapes are supported.
 * Any non-printable characters are escaped in octal as #0nn.
 */
bool
sudo_lbuf_append_esc_v1(struct sudo_lbuf *lbuf, int flags, const char * restrict fmt, ...)
{
    unsigned int saved_len = lbuf->len;
    bool ret = false;
    const char *s;
    va_list ap;
    debug_decl(sudo_lbuf_append_esc, SUDO_DEBUG_UTIL);

    if (sudo_lbuf_error(lbuf))
	debug_return_bool(false);

#define should_escape(ch) \
    ((ISSET(flags, LBUF_ESC_CNTRL) && iscntrl((unsigned char)ch)) || \
    (ISSET(flags, LBUF_ESC_BLANK) && isblank((unsigned char)ch)))
#define should_quote(ch) \
    (ISSET(flags, LBUF_ESC_QUOTE) && (ch == '\'' || ch == '\\'))

    va_start(ap, fmt);
    while (*fmt != '\0') {
	if (fmt[0] == '%' && fmt[1] == 's') {
	    if ((s = va_arg(ap, char *)) == NULL)
		s = "(NULL)";
	    while (*s != '\0') {
		if (should_escape(*s)) {
		    if (!sudo_lbuf_expand(lbuf, sizeof("#0177") - 1))
			goto done;
		    lbuf->len += escape(*s++, lbuf->buf + lbuf->len);
		    continue;
		}
		if (should_quote(*s)) {
		    if (!sudo_lbuf_expand(lbuf, 2))
			goto done;
		    lbuf->buf[lbuf->len++] = '\\';
		    lbuf->buf[lbuf->len++] = *s++;
		    continue;
		}
		if (!sudo_lbuf_expand(lbuf, 1))
		    goto done;
		lbuf->buf[lbuf->len++] = *s++;
	    }
	    fmt += 2;
	    continue;
	}
	if (should_escape(*fmt)) {
	    if (!sudo_lbuf_expand(lbuf, sizeof("#0177") - 1))
		goto done;
	    if (*fmt == '\'') {
		lbuf->buf[lbuf->len++] = '\\';
		lbuf->buf[lbuf->len++] = *fmt++;
	    } else {
		lbuf->len += escape(*fmt++, lbuf->buf + lbuf->len);
	    }
	    continue;
	}
	if (!sudo_lbuf_expand(lbuf, 1))
	    goto done;
	lbuf->buf[lbuf->len++] = *fmt++;
    }
    ret = true;

done:
    if (!ret)
	lbuf->len = saved_len;
    if (lbuf->size != 0)
	lbuf->buf[lbuf->len] = '\0';
    va_end(ap);

    debug_return_bool(ret);
}

/*
 * Parse the format and append strings, only %s and %% escapes are supported.
 * Any characters in set are quoted with a backslash.
 */
bool
sudo_lbuf_append_quoted_v1(struct sudo_lbuf *lbuf, const char *set, const char * restrict fmt, ...)
{
    unsigned int saved_len = lbuf->len;
    bool ret = false;
    const char *cp, *s;
    va_list ap;
    unsigned int len;
    debug_decl(sudo_lbuf_append_quoted, SUDO_DEBUG_UTIL);

    if (sudo_lbuf_error(lbuf))
	debug_return_bool(false);

    va_start(ap, fmt);
    while (*fmt != '\0') {
	if (fmt[0] == '%' && fmt[1] == 's') {
	    if ((s = va_arg(ap, char *)) == NULL)
		s = "(NULL)";
	    while ((cp = strpbrk(s, set)) != NULL) {
		len = (unsigned int)(cp - s);
		if (!sudo_lbuf_expand(lbuf, len + 2))
		    goto done;
		memcpy(lbuf->buf + lbuf->len, s, len);
		lbuf->len += len;
		lbuf->buf[lbuf->len++] = '\\';
		lbuf->buf[lbuf->len++] = *cp;
		s = cp + 1;
	    }
	    if (*s != '\0') {
		len = (unsigned int)strlen(s);
		if (!sudo_lbuf_expand(lbuf, len))
		    goto done;
		memcpy(lbuf->buf + lbuf->len, s, len);
		lbuf->len += len;
	    }
	    fmt += 2;
	    continue;
	}
	if (!sudo_lbuf_expand(lbuf, 2))
	    goto done;
	if (strchr(set, *fmt) != NULL)
	    lbuf->buf[lbuf->len++] = '\\';
	lbuf->buf[lbuf->len++] = *fmt++;
    }
    ret = true;

done:
    if (!ret)
	lbuf->len = saved_len;
    if (lbuf->size != 0)
	lbuf->buf[lbuf->len] = '\0';
    va_end(ap);

    debug_return_bool(ret);
}

/*
 * Parse the format and append strings, only %s, %n$s and %% escapes are supported.
 */
bool
sudo_lbuf_append_v1(struct sudo_lbuf *lbuf, const char * restrict fmt, ...)
{
    unsigned int saved_len = lbuf->len;
    bool ret = false;
    va_list ap;
    const char *s;
    unsigned int len;
    debug_decl(sudo_lbuf_append, SUDO_DEBUG_UTIL);

    if (sudo_lbuf_error(lbuf))
	debug_return_bool(false);

    va_start(ap, fmt);
    while (*fmt != '\0') {
	if (fmt[0] == '%' && isdigit((unsigned char)fmt[1])) {
	    const char *num_start = fmt + 1;
	    const char *num_end = num_start;
	    int arg_num;
	    /* Find the end of the numeric part */
	    while (isdigit((unsigned char)*num_end))
		num_end++;
	    if (num_end[0] == '$' && num_end[1] == 's' && num_end > num_start) {
		/* Convert the numeric part to an integer */
		char numbuf[STRLEN_MAX_SIGNED(int) + 1];
		len = (unsigned int)(num_end - num_start);
		if (len >= sizeof(numbuf)) {
		    errno = EINVAL;
		    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
			"integer overflow parsing $n");
		    lbuf->error = 1;
		    goto done;
		}
		memcpy(numbuf, num_start, len);
		numbuf[len] = '\0';
		arg_num = atoi(numbuf);
		if (arg_num > 0) {
		    va_list arg_copy;
		    va_copy(arg_copy, ap);
		    for (int i = 1; i < arg_num; i++) {
			(void)va_arg(arg_copy, char *);
		    }
		    if ((s = va_arg(arg_copy, char *)) == NULL)
			s = "(NULL)";
		    len = (unsigned int)strlen(s);
		    if (!sudo_lbuf_expand(lbuf, len)) {
			va_end(arg_copy);
			goto done;
		    }
		    memcpy(lbuf->buf + lbuf->len, s, len);
		    lbuf->len += len;
		    fmt = num_end + 2;
		    va_end(arg_copy);
		    continue;
		}
	    }
	}
	if (fmt[0] == '%' && fmt[1] == 's') {
	    if ((s = va_arg(ap, char *)) == NULL)
		s = "(NULL)";
	    len = (unsigned int)strlen(s);
	    if (!sudo_lbuf_expand(lbuf, len))
		goto done;
	    memcpy(lbuf->buf + lbuf->len, s, len);
	    lbuf->len += len;
	    fmt += 2;
	    continue;
	}
	if (!sudo_lbuf_expand(lbuf, 1))
	    goto done;
	lbuf->buf[lbuf->len++] = *fmt++;
    }
    ret = true;

done:
    if (!ret)
	lbuf->len = saved_len;
    if (lbuf->size != 0)
	lbuf->buf[lbuf->len] = '\0';
    va_end(ap);

    debug_return_bool(ret);
}

/* XXX - check output function return value */
static void
sudo_lbuf_println(struct sudo_lbuf *lbuf, char *line, size_t len)
{
    char *cp, save;
    size_t i, have, contlen = 0;
    unsigned int indent = lbuf->indent;
    bool is_comment = false;
    debug_decl(sudo_lbuf_println, SUDO_DEBUG_UTIL);

    /* Comment lines don't use continuation and only indent is for "# " */
    if (line[0] == '#' && isblank((unsigned char)line[1])) {
	is_comment = true;
	indent = 2;
    }
    if (lbuf->continuation != NULL && !is_comment)
	contlen = strlen(lbuf->continuation);

    /*
     * Print the buffer, splitting the line as needed on a word
     * boundary.
     */
    cp = line;
    have = lbuf->cols;
    while (cp != NULL && *cp != '\0') {
	char *ep = NULL;
	size_t need = len - (size_t)(cp - line);

	if (need > have) {
	    have -= contlen;		/* subtract for continuation char */
	    if ((ep = memrchr(cp, ' ', have)) == NULL)
		ep = memchr(cp + have, ' ', need - have);
	    if (ep != NULL)
		need = (size_t)(ep - cp);
	}
	if (cp != line) {
	    if (is_comment) {
		lbuf->output("# ");
	    } else {
		/* indent continued lines */
		/* XXX - build up string instead? */
		for (i = 0; i < indent; i++)
		    lbuf->output(" ");
	    }
	}
	/* NUL-terminate cp for the output function and restore afterwards */
	save = cp[need];
	cp[need] = '\0';
	lbuf->output(cp);
	cp[need] = save;
	cp = ep;

	/*
	 * If there is more to print, reset have, incremement cp past
	 * the whitespace, and print a line continuaton char if needed.
	 */
	if (cp != NULL) {
	    have = lbuf->cols - indent;
	    ep = line + len;
	    while (cp < ep && isblank((unsigned char)*cp)) {
		cp++;
	    }
	    if (contlen)
		lbuf->output(lbuf->continuation);
	}
	lbuf->output("\n");
    }

    debug_return;
}

/*
 * Print the buffer with word wrap based on the tty width.
 * The lbuf is reset on return.
 * XXX - check output function return value
 */
void
sudo_lbuf_print_v1(struct sudo_lbuf *lbuf)
{
    char *cp, *ep;
    size_t len;
    debug_decl(sudo_lbuf_print, SUDO_DEBUG_UTIL);

    if (lbuf->buf == NULL || lbuf->len == 0)
	goto done;

    /* For very small widths just give up... */
    len = lbuf->continuation ? strlen(lbuf->continuation) : 0;
    if (lbuf->cols <= lbuf->indent + len + 20) {
	lbuf->buf[lbuf->len] = '\0';
	lbuf->output(lbuf->buf);
	if (lbuf->buf[lbuf->len - 1] != '\n')
	    lbuf->output("\n");
	goto done;
    }

    /* Print each line in the buffer */
    for (cp = lbuf->buf; cp != NULL && *cp != '\0'; ) {
	if (*cp == '\n') {
	    lbuf->output("\n");
	    cp++;
	} else {
	    len = lbuf->len - (size_t)(cp - lbuf->buf);
	    if ((ep = memchr(cp, '\n', len)) != NULL)
		len = (size_t)(ep - cp);
	    if (len)
		sudo_lbuf_println(lbuf, cp, len);
	    cp = ep ? ep + 1 : NULL;
	}
    }

done:
    lbuf->len = 0;		/* reset the buffer for re-use. */
    lbuf->error = 0;

    debug_return;
}

bool
sudo_lbuf_error_v1(struct sudo_lbuf *lbuf)
{
    if (lbuf != NULL && lbuf->error != 0)
	return true;
    return false;
}

void
sudo_lbuf_clearerr_v1(struct sudo_lbuf *lbuf)
{
    if (lbuf != NULL)
	lbuf->error = 0;
}
