/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 1996, 1998-2005, 2007-2016
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
 *
 * Sponsored in part by the Defense Advanced Research Projects
 * Agency (DARPA) and Air Force Research Laboratory, Air Force
 * Materiel Command, USAF, under agreement number F39502-99-1-0512.
 */

/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

#include <sudoers.h>
#include <toke.h>
#include <gram.h>

static size_t arg_len = 0;
static size_t arg_size = 0;

/*
 * Copy the string and collapse any escaped characters.
 * Requires that dst have at least len + 1 bytes free.
 */
static void
copy_string(char *dst, const char *src, size_t len)
{
    int h;

    while (len--) {
	if (*src == '\\' && len) {
	    if (src[1] == 'x' && len >= 3 && (h = sudo_hexchar(src + 2)) != -1) {
		*dst++ = (char)h;
		src += 4;
		len -= 3;
	    } else {
		src++;
		len--;
		*dst++ = *src++;
	    }
	} else {
	    *dst++ = *src++;
	}
    }
    *dst = '\0';
}

bool
fill(const char *src, int ilen)
{
    const size_t len = (size_t)ilen;
    char *dst;
    debug_decl(fill, SUDOERS_DEBUG_PARSER);

    dst = malloc(len + 1);
    if (dst == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	sudoerserror(NULL);
	debug_return_bool(false);
    }
    parser_leak_add(LEAK_PTR, dst);
    copy_string(dst, src, len);
    sudoerslval.string = dst;

    debug_return_bool(true);
}

bool
append(const char *src, int ilen)
{
    const size_t len = (size_t)ilen;
    size_t olen = 0;
    char *dst;
    debug_decl(append, SUDOERS_DEBUG_PARSER);

    if (sudoerslval.string != NULL) {
	olen = strlen(sudoerslval.string);
	parser_leak_remove(LEAK_PTR, sudoerslval.string);
    }

    dst = realloc(sudoerslval.string, olen + len + 1);
    if (dst == NULL) {
	/* realloc failure, avoid leaking original */
	free(sudoerslval.string);
	sudoerslval.string = NULL;
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	sudoerserror(NULL);
	debug_return_bool(false);
    }
    parser_leak_add(LEAK_PTR, dst);
    copy_string(dst + olen, src, len);
    sudoerslval.string = dst;

    debug_return_bool(true);
}

#define SPECIAL(c) \
    ((c) == ',' || (c) == ':' || (c) == '=' || (c) == ' ' || (c) == '\t' || (c) == '#')

bool
fill_cmnd(const char *src, int ilen)
{
    const size_t len = (size_t)ilen;
    char *dst;
    size_t i;
    debug_decl(fill_cmnd, SUDOERS_DEBUG_PARSER);

    arg_len = arg_size = 0;

    dst = sudoerslval.command.cmnd = malloc(len + 1);
    if (dst == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	sudoerserror(NULL);
	debug_return_bool(false);
    }
    sudoerslval.command.args = NULL;

    if (src[0] == '^') {
	/* Copy the regular expression, no escaped sudo-specific characters. */
	memcpy(dst, src, len);
	dst[len] = '\0';
    } else {
	/* Copy the string and collapse any escaped sudo-specific characters. */
	for (i = 0; i < len; i++) {
	    if (src[i] == '\\' && i != len - 1 && SPECIAL(src[i + 1]))
		*dst++ = src[++i];
	    else
		*dst++ = src[i];
	}
	*dst = '\0';

	/* Check for sudoedit specified as a fully-qualified path. */
	if ((dst = strrchr(sudoerslval.command.cmnd, '/')) != NULL) { // -V575
	    if (strcmp(dst, "/sudoedit") == 0) {
		if (sudoers_strict()) {
		    sudoerserror(
			N_("sudoedit should not be specified with a path"));
		}
		free(sudoerslval.command.cmnd);
		if ((sudoerslval.command.cmnd = strdup("sudoedit")) == NULL) {
		    sudo_warnx(U_("%s: %s"), __func__,
			U_("unable to allocate memory"));
		    debug_return_bool(false);
		}
	    }
	}
    }

    parser_leak_add(LEAK_PTR, sudoerslval.command.cmnd);
    debug_return_bool(true);
}

bool
fill_args(const char *s, int ilen, bool addspace)
{
    size_t len = (size_t)ilen;
    size_t new_len;
    char *p;
    debug_decl(fill_args, SUDOERS_DEBUG_PARSER);

    if (arg_size == 0) {
#ifdef NO_LEAKS
	if (sudoerslval.command.args != NULL) {
	    sudo_warnx("%s: command.args %p, should be NULL", __func__,
		sudoerslval.command.args);
	    sudoerslval.command.args = NULL;
	}
#endif
	addspace = 0;
	new_len = len;
    } else {
	new_len = arg_len + len + addspace;
    }

    if (new_len >= arg_size) {
	/* Allocate in increments of 128 bytes to avoid excessive realloc(). */
	arg_size = (new_len + 1 + 127) & ~127U;

	parser_leak_remove(LEAK_PTR, sudoerslval.command.args);
	p = realloc(sudoerslval.command.args, arg_size);
	if (p == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    goto bad;
	}
	parser_leak_add(LEAK_PTR, p);
	sudoerslval.command.args = p;
    }

    /* Efficiently append the arg (with a leading space if needed). */
    p = sudoerslval.command.args + arg_len;
    if (addspace)
	*p++ = ' ';
    len = arg_size - (size_t)(p - sudoerslval.command.args);
    if (strlcpy(p, s, len) >= len) {
	sudo_warnx(U_("internal error, %s overflow"), __func__);
	parser_leak_remove(LEAK_PTR, sudoerslval.command.args);
	goto bad;
    }
    arg_len = new_len;
    debug_return_bool(true);
bad:
    sudoerserror(NULL);
    free(sudoerslval.command.args);
    sudoerslval.command.args = NULL;
    arg_len = arg_size = 0;
    debug_return_bool(false);
}

/*
 * Check to make sure an IPv6 address does not contain multiple instances
 * of the string "::".  Assumes strlen(s) >= 1.
 * Returns true if address is valid else false.
 */
bool
ipv6_valid(const char *s)
{
    int nmatch = 0;
    debug_decl(ipv6_valid, SUDOERS_DEBUG_PARSER);

    for (; *s != '\0'; s++) {
	if (s[0] == ':' && s[1] == ':') {
	    if (++nmatch > 1)
		break;
	}
	if (s[0] == '/')
	    nmatch = 0;			/* reset if we hit netmask */
    }

    debug_return_bool(nmatch <= 1);
}
